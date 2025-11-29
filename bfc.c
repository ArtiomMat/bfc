/* A brainfuck compiler written in ANSI-C */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

typedef enum {
  /* Let the architecture decide. */
  OVERFLOW_BEHAVIOR_UNDEFINED,
  /* Check and contain overflows. */
  OVERFLOW_BEHAVIOR_CAP,
  /* Check and abort on overflow. */
  OVERFLOW_BEHAVIOR_ABORT,
} OverflowBehavior;

typedef enum {
  LOG_LEVEL_FATAL = 10,
  LOG_LEVEL_ERROR = 20,
  LOG_LEVEL_WARN = 30,
  LOG_LEVEL_INFO = 40,
  LOG_LEVEL_DEBUG = 50,
} LogLevel;

typedef enum {
  /* NULL equivalent for OpType */
  OP_INVALID,

  /* NOP, unlike OP_INVALID it has meaning, skip. */
  OP_SKIP,

  /* n = How much to add to byte */
  OP_MUTATE,
  /* n = How many bytes to jump */
  OP_MOVE,

  /* n = How many bytes to input */
  OP_INPUT,
  /* n = How many bytes to print */
  OP_PRINT,
  
  /* n = How many operations to jump */
  OP_IF_0,
  /* n = How many operations to jump */
  OP_IF_NOT_0,
} OpType;

typedef struct Op {
  /* NULL means no next */
  struct Op* next;
  OpType type;
  /* Depends on type, read OpType */
  int n;
} Op;

/*
 * Represents an extended version of the source code,
 * with members to easen lexing.
 */
typedef struct{
  const char* text;

  /*
   * The size as `len`.
   *
   * The purpose of this global is to mark `[` and `]` if they don't signify
   * ops but rather delimiters for previous `]` and `[` respectively.
   *
   * For example, if `text[10]` has `[` and then in `text[15]` there is `]`
   * that then `delimiter_brackets[15]` is marked with `1` to show that it's
   * only a delimiter.
   char* delimiter_brackets;
   */

  /*
   * Cache it because we already calculate it anyway.
   *
   * Length not including null terminator.
   */
  int len;

  /*
   * Index within text.
   */
  int i;
} Source;

/* Set to 1 if there was an error somewhere in the pipeline. */
static int G_ERROR = 0;

static struct {
  int overflow_behavior;
  /* Size in sizeof() units. */
  int byte_size;
} G_PARAMETERS = {
  .overflow_behavior = OVERFLOW_BEHAVIOR_UNDEFINED,
  .byte_size = 1,
};

static void log(FILE* f, const LogLevel level, const Source* src, const char* fmt, va_list args){
  const char* level_str = NULL;
  int line = 1;
  int column = 1;
  int i = 0;

  switch (level) {
  case LOG_LEVEL_FATAL:
    level_str = "FATAL";
    break;
  case LOG_LEVEL_ERROR:
    level_str = "ERROR";
    break;
  case LOG_LEVEL_WARN:
    level_str = "WARNING";
    break;
  case LOG_LEVEL_INFO:
    level_str = "INFO";
    break;
  case LOG_LEVEL_DEBUG:
    level_str = "DEBUG";
    break;
  default:
    level_str = "LOG";
    break;
  }

  fprintf(f, "%s: ", level_str);

  if (src) {
    for (i = 0; i < src->i; ++i, ++column) {
      if ('\n' == src->text[i]) {
        ++line;
        column = 0;
      }
    }
  
    fprintf(f, "%s:%i:%i: ", src->path, line, column);
  }

  vfprintf(f, fmt, args);
  putc('\n', stdout);
}

/*
 * Sets `G_ERROR` to `1`.
 */
static void log_error(const Source* src, const char* fmt, ...) {
  va_list args;

  G_ERROR = 1;

  va_start(args, fmt);
  log(stderr, LOG_LEVEL_ERROR, src, fmt, args);
  va_end(args);
}

static OpType op_type_from_c(const char c) {
  switch (c) {
  case '+':
  case '-':
    return OP_MUTATE;
  case '>':
  case '<':
    return OP_MOVE;
  case ',':
    return OP_INPUT;
  case '.':
    return OP_PRINT;
  case '[':
    return OP_IF_0;
  case ']':
    return OP_IF_NOT_0;
  default:
    return OP_SKIP;
  }
}

/*
 * `src->text[src->i]` is asserted to point to the bracket.
 *
 * Sets error if there is no matching delimiter.
 *
 * Returns `op->n`, which in other words is the distance in non `OP_SKIP`
 * characters from this bracket, if it's `]` it should be negative.
 *
 * NOTE: Don't forget, the returned `n` is not normalized to `Op` units.
 * You must do so manually.
 *
 * `OP_SKIP` is ignored because we can do so in this stage and it will
 * prevent more complicated computation later due to getting rid of them.
 */
static int find_delimiter_for_if(const Source* src) {
  const char c = src->text[src->i];
  const OpType type = op_type_from_c(c);
  /* What is the delimiter we are looking for? */
  const int delim = type == OP_IF_0 ? ']' : '[';
  /* If it's [ we iterate forward otherwise back */
  const int j_inc = type == OP_IF_0 ? 1 : -1;
  int j = 0;
  int n = 0;
  /* If we find nested brackets we increment it to ignore them. */
  int bracket_depth = 1;

  assert('[' == c || ']' == c); /* Otherwise no point in calling this. */

  for (j = src->i + j_inc; j >= 0 && j < src->len; j += j_inc) {
    n += j_inc;

    if (c == src->text[j]) {
      ++bracket_depth;
    } else if (delim == src->text[j]) {
      --bracket_depth;

      if (!bracket_depth) {
        /* GG we found it */
        return n;
      }
    } else if (OP_SKIP == op_type_from_c(src->text[j])) {
      /* OP_SKIP are not registered */
      n -= j_inc;
    }
  }

  log_error(src, "No delimiter(%c) for %c", delim, c);
  return 0;
}

/*
 * Analyzes the current character in `src->text` and updates `op` for that character.
 *
 * Modifies `src->i` as seen fit, preparing for the next `update_op_from_c()`.
 *
 * If `op->type` is not `OP_INVALID` and `c` is not of the same type,
 * we avoid lexing it because a new `Op` needs to be created.
 *
 * Returns if above calling code should break and start lexing a new
 * `Op`.
 *
 * Returns unchanged `OP_INVALID` if the character is `OP_SKIP`, this is done to
 * prevent an all-too-expensive removal phase of `OP_SKIP` from the list.
 */
static int update_op_from_c(Source* src, Op* op) {
  const char c = src->text[src->i];
  OpType type = op_type_from_c(c);
  int should_break = 0;

  /* First time updating. */
  if (OP_INVALID == op->type) {
    assert(!op->n); /* Must be reset if OP_INVALID. */

    switch (type) {
    case OP_SKIP:
      goto _done; /* We ignore these */

    case OP_IF_NOT_0:
    case OP_IF_0:
      assert('[' == c || ']' == c);
      op->n = find_delimiter_for_if(src);
      should_break = 1; /* We don't want to accumalate them */
      break;

    case OP_MUTATE:
      assert('+' == c || '-' == c);
      op->n = c == '+' ? 1 : -1;
      break;

    case OP_MOVE:
      assert('>' == c || '<' == c);
      op->n = c == '>' ? 1 : -1;
      break;

    case OP_PRINT:
    case OP_INPUT:
      op->n = 1;
      break;

    default:
      assert(0); /* Unknown OP. */
      break;
    }

    op->type = type;

    goto _done;
  }

  if (type != op->type) {
    /* Means we need to start lexing a new Op */
    return 1;
  }

  switch (type) {
  case OP_MUTATE:
    assert('+' == c || '-' == c);
    op->n += c == '+' ? 1 : -1;
    break;

  case OP_MOVE:
    assert('>' == c || '<' == c);
    op->n += c == '>' ? 1 : -1;
    break;

  case OP_INPUT:
  case OP_PRINT:
    ++op->n;
    break;

  case OP_IF_0:
  case OP_IF_NOT_0:
    assert(0); /* Should not have gotten here with the OP_IF_* */
    break;
  default:
    assert(0); /* Unknown OP. */
    break;
  }

_done:
  ++src->i;
  return should_break;
}

/*
 * Analyzes `src->text` and constructs a full `Op` from as many related characters
 * as possible. For example `++----` would group into an `OP_MUTATE` with `n=-2`.
 *
 * Modifies `src->i` as seen fit, preparing for the next `tokenize_one_op()`.
 *
 * Returns `NULL` if `src->i` is at the end.
 */
static Op* tokenize_one_op(Source* src) {
  Op* op = NULL;

  if (src->i >= src->len) {
    assert(src->i == src->len);
    return NULL;
  }

  /* Reset op */
  op = malloc(sizeof (Op));
  op->next = NULL;
  op->type = OP_INVALID;
  op->n = 0;

  for (/* Already initialized */; src->i < src->len; /* Inside */) {
    int should_break = update_op_from_c(src, op);
    
    if (G_ERROR) {
      goto _nothing;
    }

    if (should_break) {
      break;
    }
  }

  if (OP_INVALID == op->type) {
    assert(src->i >= src->len); /* Should be the only scenario where we still have OP_INVALID. */
    goto _nothing;
  }

  return op;

_nothing:
  if (op) {
    free(op);
  }
  return NULL;
}

/*
 * Returns NULL if there is nothing to tokenize.
 */
static Op* tokenize(Source* src) {
  Op* first_op = NULL;
  Op* last_op = NULL; /* To know from where to push the next */
  Op* current_op = NULL; /* For iteration */

  while (1) {
    current_op = tokenize_one_op(src);
    if (!current_op) {
      break;
    }

    if (!first_op) {
      first_op = current_op;
      last_op = current_op;
    } else {
      last_op->next = current_op;
      last_op = current_op;
    }

    if (G_ERROR) {
      goto _failure;
    }
  }

  return first_op;

_failure:
  if (first_op) {
    for (current_op = first_op; current_op; /* Inside */) {
      Op* next = current_op->next;
      free(current_op);
      current_op = next;
    }
  }
  return NULL;
}

static Source source_from_text(const char* text) {
  Source source;

  assert(text);

  source.text = text;
  source.len = strlen(text);
  source.i = 0;
  /* source.delimiter_brackets = calloc(source.len, sizeof(*source.delimiter_brackets)); */

  return source;
}

int main(const int argc, const char** argv) {
  /* TODO: Everything up until the first input instruction can be cached. */
  Op* x;

  Source example = source_from_text(argv[1]);
  for (x = tokenize(&example); x; x = x->next) {
    printf("%i %i\n", x->type, x->n);
  }

  return 0;
}

