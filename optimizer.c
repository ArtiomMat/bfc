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

OptimizationInfo optimize_ops(Source* src, Op** ops) {
  OptimizationInfo optimiziation_info = {
    .input_op_i = -1,
  };

  *ops = prune_null_ops(src, *ops);

  return optimiziation_info;
}

