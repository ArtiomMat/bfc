#include "optimizer.h"
#include "log.h"
#include "op.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static int should_prune(const Op* op) {
  assert(op);

  switch (op->type) {
  case OP_MOVE:
  case OP_MUTATE:
    if (!op->n) {
      return 1;
    }
    break;

  /* TODO: More */

  default:
    break;
  }

  return 0;
}

/* We use it twice so just to avoid duplication */
#define LOG_PRUNE(SRC, OP) \
  do { \
    (SRC)->i = (OP)->src_i; \
    log_warn(SRC, "optimizer: %s sequence evaluates to NOP here.", str_from_op_type((OP)->type)); \
  } while (0)

/*
 * Finds operations that are useless to keep.
 * 
 * Returns a pruned subset of `ops`, others are freed of course.
 */
static Op* prune_null_ops(Source* src, Op* ops) {
  Op* op = NULL;
  Op* next = NULL;
  Op* previous = NULL;
  Op* first_op = ops;

  /* There is special logic if we need to prune the first op. */
  first_op = ops;
  while (first_op && should_prune(first_op)) {
    LOG_PRUNE(src, first_op);

    next = first_op->next;
    free(op);
    first_op = next;
  }

  if (!first_op) {
    goto done_;
  }
  
  assert(!should_prune(first_op)); /* Sanity check */

  /* We have asserted that we can skip the first one */
  for (op = first_op->next; op; op = next) {
    next = op->next;

    if (should_prune(op)) {
      LOG_PRUNE(src, op);
      
      assert(previous); /* Must always exist. */
      previous->next = op->next;
      free(op);
    } else {
      previous = op;
    }
  }

done_:
  return first_op;
}

static Op* find_first_input_op(Op* ops) {
  Op* op;

  for (op = ops; op; op = op->next) {
    if (OP_INPUT == op->type) {
      return op;
    }
  }

  return NULL;
}

OptimizationInfo optimize_ops(Source* src, Op** ops) {
  OptimizationInfo optimiziation_info = {
    .first_input_op = NULL,
  };

  *ops = prune_null_ops(src, *ops);

  optimiziation_info.first_input_op = find_first_input_op(*ops);
  if (optimiziation_info.first_input_op) {
    src->i = optimiziation_info.first_input_op->src_i;
    log_debug(src, "optimizer: All code up to here can be evaluated at compile-time.");
  } else {
    log_debug(src, "optimizer: The entire program is can be evaluated at compile-time.");
  }

  return optimiziation_info;
}

