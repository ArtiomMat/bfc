/* A brainfuck compiler written in ANSI-C */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

typedef enum {
  LOG_LEVEL_FATAL = 10,
  LOG_LEVEL_ERROR = 20,
  LOG_LEVEL_WARN = 30,
  LOG_LEVEL_INFO = 40,
  LOG_LEVEL_DEBUG = 40,
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
   */
  char* delimiter_brackets;

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

static void log(LogLevel level, const char* fmt, va_list args){
  printf("%i: ", level);
  vprintf(fmt, args);
}

static void log_error(const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  log(LOG_LEVEL_ERROR, fmt, args);
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
 * Analyzes the current character in `src->text` and updates `op` for that character.
 *
 * Modifies `src->i` as seen fit, preparing for the next `update_op_from_c()`.
 *
 * If `op->type` is not `OP_INVALID` and `c` is not of the same type,
 * we avoid lexing it because a new `Op` needs to be created.
 *
 * Returns if above calling code should break and start lexing a new
 * `Op`.
 */
static OpType update_op_from_c(Source* src, Op* op) {
  const char c = src->text[src->i];
  OpType type = op_type_from_c(c);

  /* First time updating. */
  if (OP_INVALID == op->type) {
    assert(0 == op->n); /* Must be reset if OP_INVALID. */

    switch (type) {
    case OP_IF_NOT_0:
    case OP_IF_0:
      /* TODO: Implement marking delimiters and such */
      break;

    case OP_SKIP:
    case OP_PRINT:
    case OP_INPUT:
    case OP_MUTATE:
    case OP_MOVE:
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
    return 1; /* Avoid lexing and notify that should break */
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
  case OP_SKIP:
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
  return 0;
}

/*
 * Analyzes `src->text` and constructs a full `Op` from as many related characters
 * as possible. For example `++----` would group into an `OP_MUTATE` with `n=-2`.
 *
 * Modifies `src->i` as seen fit, preparing for the next `tokenize_one_op()`.
 *
 * Returns NULL if there was an error. `*i_ptr` will point to the problematic char.
 */
static Op* tokenize_one_op(Source* src) {
  Op* op = malloc(sizeof (Op));

  op->next = NULL;
  op->type = OP_INVALID;
  op->n = 0;

  for (/* Already initialized */; src->i < src->len; /* Inside */) {
    int should_break = update_op_from_c(src, op);
    
    if (G_ERROR) {
      goto _failure;
    }

    if (should_break) {
      break;
    }
  }

  return op;

_failure:
  if (op) {
    free(op);
  }
  return NULL;
}

/*
 * Returns NULL if there was an error.
 */
static Op* tokenize(Source* src) {
  Op* first_op = NULL;
  Op* last_op = NULL; /* To know from where to push the next */
  Op* current_op = NULL; /* For iteration */

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
  source.delimiter_brackets = calloc(source.len, sizeof(*source.delimiter_brackets));

  return source;
}

int main(const int argc, const char** argv) {
  /* TODO: Everything up until the first input instruction can be cached. */

  return 0;
}

