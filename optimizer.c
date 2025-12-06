#include "optimizer.h"
#include "log.h"
#include "op.h"
#include "parameters.h"
#include "source.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

static int merge_ops(Op** ops) {
  /* TODO */
  return 0;
}

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
    set_source_i((SRC), (OP)); \
    log_warn(SRC, "optimizer: %s sequence evaluates to NOP here.", str_from_op_type((OP)->type)); \
  } while (0)

/*
 * Finds operations that are useless to keep.
 * 
 * Returns how many times we pruned.
 */
static int prune_null_ops(Source* src, Op** ops) {
  Op* op = NULL;
  Op* next = NULL;
  Op* previous = NULL;
  Op* first_op = NULL;
  int prunes_count = 0;

  /* There is special logic if we need to prune the first op. */
  first_op = *ops;
  while (first_op && should_prune(first_op)) {
    LOG_PRUNE(src, first_op);

    ++prunes_count;
    next = first_op->next;
    free(op);
    first_op = next;
  }

  if (!first_op) {
    goto done_;
  }
  
  assert(!should_prune(first_op)); /* Sanity check */

  previous = first_op;
  /* We have asserted that we can skip the first one */
  for (op = first_op->next; op; op = next) {
    next = op->next;

    if (should_prune(op)) {
      LOG_PRUNE(src, op);
      
      assert(previous); /* Must always exist. */
      ++prunes_count;
      previous->next = op->next;
      free(op);
    } else {
      previous = op;
    }
  }

done_:
  *ops = first_op;
  return prunes_count;
}

static void warn_overflows(Source* src, Op* ops) {
  Op* op;

  for (op = ops; op; op = op->next) {
    switch (op->type) {
    case OP_MOVE:
    case OP_MUTATE:
      set_source_i(src, op);

      if (op->n > MAX_BF_BYTE || op->n < -MAX_BF_BYTE) {
        log_warn(src, "optimizer: %s sequence causes overflow(%d).", str_from_op_type(op->type), op->n);

        if (G_PARAMETERS.overflow_behavior == OVERFLOW_BEHAVIOR_ABORT) {
          log_warn(src, "optimizer: Regarding above warning, this guarantees eventual abort due to the configured overflow behavior");
        }
      }
      break;

    default:
      break;
    }
  }
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

  do {
    merge_ops(ops);
  } while (prune_null_ops(src, ops));

  optimiziation_info.first_input_op = find_first_input_op(*ops);
  if (optimiziation_info.first_input_op) {
    set_source_i(src, optimiziation_info.first_input_op);
    log_debug(src, "optimizer: All code up to here can be evaluated at compile-time.");
  } else {
    log_debug(src, "optimizer: The entire program can be evaluated at compile-time.");
  }

  return optimiziation_info;
}

