#include "lexer.h"
#include "log.h"
#include "op.h"

#include <assert.h>
#include <stdlib.h>

/*
 * `src->text[src->i]` is asserted to point to the bracket.
 *
 * On success, returns `1`, and sets `op->n` to the distance in non `OP_SKIP`
 * characters from this bracket, if it's `]` it should be negative.
 * 
 * On failure, `0` if delimiter is not found.
 *
 * NOTE: Don't forget, the returned `n` is not normalized to `Op` units.
 * You must do so manually.
 *
 * `OP_SKIP` is ignored because we can do so in this stage and it will
 * prevent more complicated computation later due to getting rid of them.
 */
static int find_delimiter_for_if(const Source* src, Op* op) {
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
        op->n = n;
        return 1;
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
 * Modifies `op->src_i` to *EVENTUALLY* point to the correct `src->i`.
 *
 * If `op->type` is not `OP_INVALID` and `c` is not of the same type,
 * we avoid lexing it because a new `Op` needs to be created.
 *
 * On success, returns if above calling code should break and start lexing a new
 * `Op`.
 *
 * On failure, returns `-1`, state of `src->i` is unchanged.
 */
static int update_op_from_c(Source* src, Op* op) {
  const char c = src->text[src->i];
  OpType type = op_type_from_c(c);
  int should_break = 0;
  int success = 1;

  /* First time updating. */
  if (OP_INVALID == op->type) {
    assert(!op->n); /* Must be reset if OP_INVALID. */
    op->src_start = src->i;

    switch (type) {
    case OP_SKIP:
      goto done_; /* We ignore these */

    case OP_IF_NOT_0:
    case OP_IF_0:
      assert('[' == c || ']' == c);
      success = find_delimiter_for_if(src, op);
      if (!success) {
        return -1;
      }

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

    goto done_;
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

done_:
  ++src->i;
  return should_break;
}

/*
 * Analyzes `src->text` and constructs a full `Op` from as many related characters
 * as possible. For example `++----` would group into an `OP_MUTATE` with `n=-2`.
 *
 * Modifies `src->i` as seen fit, preparing for the next `tokenize_one_op()`.
 *
 * On success, returns `1`, and sets `*op_ptr` to `NULL` if `src->i` is at the end, or sets it to
 * some valid `Op` for the sequence of operations we lexed.
 *
 * On failure, returns `0`.
 */
static int lex_one_op(Source* src, Op** op_ptr) {
  Op* op = NULL;
  int success = 1;

  if (src->i >= src->len) {
    assert(src->i == src->len);
    *op_ptr = NULL;
    return 1;
  }

  /* Reset op */
  op = malloc(sizeof (Op));
  reset_op(op);

  for (/* Already initialized */; src->i < src->len; /* Inside */) {
    int should_break = update_op_from_c(src, op);
    
    if (-1 == should_break) {
      success = 0;
      goto nothing_;
    }

    if (should_break) {
      op->src_end = src->i;
      break;
    }
  }

  if (OP_INVALID == op->type) {
    assert(src->i >= src->len); /* Should be the only scenario where we still have OP_INVALID. */
    goto nothing_;
  }

  log_debug(
    src, "lexer: Op{type=%s, n=%i, start=%i, end=%i}",
    str_from_op_type(op->type), op->n, op->src_start, op->src_end
  );

  goto done_;

nothing_:
  if (op) {
    free(op);
    op = NULL;
  }

done_:
  *op_ptr = op;
  return success;
}

int lex(Source* src, Op** first_op_ptr) {
  Op* first_op = NULL;
  Op* last_op = NULL; /* To know from where to push the next */
  Op* current_op = NULL; /* For iteration */
  int success = 0;

  while (1) {
    success = lex_one_op(src, &current_op);
    if (!current_op) {
      break;
    }

    if (!first_op) {
      /* current_op is the actual first */
      first_op = current_op;
      last_op = current_op;
    } else {
      /* current_op is appended */
      last_op->next = current_op;
      last_op = current_op;
    }

    if (!success) {
      goto failure_;
    }
  }

  goto done_;

failure_:
  if (first_op) {
    free_ops(first_op);
    first_op = NULL;
  }

done_:
  *first_op_ptr = first_op;
  return success;
}

