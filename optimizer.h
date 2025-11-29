#ifndef BFC_OPTIMIZER_H
#define BFC_OPTIMIZER_H

#include "op.h"

/* TODO: Prune null OP_MUTATE and OP_MOVE, if their n is 0. */
/* TODO: Optimize logical flows that are never reached. */

/*
 * There is some, but little, that the optimizer does on the `Op` level,
 * since it's already such a minimal instruction set.
 *
 * But the optimizer collects information that can be very relevant
 * for the assembly stage, and compiles it here.
 */
typedef struct {
    /*
     * Index in the `Op` list.
     *
     * If -1 there is none, then entire program can be evaluated at compile time
     * into a single print call.
     *
     * If positive, then everything up to that index can be cached.
     */
    int input_op_i;
} OptimizationInfo;

/*
 * Prunes ops that equate to `NOP`, like `Op`s that came from `<<>>` or `++--`.
 * Removes dead code, like brackets that are known to never execute.
 *
 * Returns additional information for assembly stage, see documentation for `OptimizationInfo`
 */
OptimizationInfo optimize_ops(Op* ops);

#endif /* define BFC_OPTIMIZER_H */
