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
 * The same size as `text`.
 *
 * The purpose of this global is to mark `[` and `]` if they don't signify
 * ops but rather delimiters for previous `]` and `[` respectively.
 *
 * For example, if `text[10]` has `[` and then in `text[15]` there is `]`
 * that then `G_DELIMITER_BRACKETS[15]` is marked with `1` to show that it's
 * only a delimiter.
 */
char* G_DELIMITER_BRACKETS = NULL;

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
 * `op` is modified according to `c` and its `OpType`.
 *
 * If `op->type` is not `OP_INVALID` and `c` is not of the same type,
 * this is an error.
 *
 * Returns OP_INVALID on error, otherwise the `OpType` of `c`. 
 * This is done so the caller compares the return value with `op->type`
 * and knows if this is the last `c`.
 */
static OpType update_op_from_c(const char c, Op* op) {
  OpType type = op_type_from_c(c);

  if (OP_INVALID == op->type) {

  }
}

/*
 * `i_ptr` is a pointer to the current index within `text` it is modified
 * as seen fit, and prepared for the next `tokenize_one_op()`.
 *
 * Returns NULL if there was an error. `*i_ptr` will point to the problematic char.
 */
static Op* tokenize_one_op(const char* text, int* i_ptr) {
  Op* op = malloc(sizeof (Op));
  int i = *i_ptr;

  op->next = NULL;
  op->type = OP_INVALID;
  op->n = 0;

  for (i = *i_ptr; text[i]; ++i) {
    const OpType type = update_op_from_c(text[i], op);
    
    if (OP_INVALID == type) {
      goto _failure;
    } else if (op->type != type) {
      /* Time to stop for this op */
      break;
    }
  }

  *i_ptr = i;
  return op;

_failure:
  *i_ptr = i;

  if (op) {
    free(op);
  }
  return NULL;
}

/*
 * Returns NULL if there was an error.
 */
static Op* tokenize(const char* text) {
  Op* first_op = NULL;
  Op* last_op = NULL; /* To know from where to push the next */
  Op* current_op = NULL; /* For iteration */
  int i = 0;

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

int main(const int argc, const char** argv) {
  /* TODO: Everything up until the first input instruction can be cached. */

  return 0;
}

