#ifndef BFC_ASSEMBLER_H
#define BFC_ASSEMBLER_H

#include "op.h"
#include "io_buf.h"
#include "optimizer.h"

typedef struct {
  /*
   * Code segment, entry point can be considered at `[0]`.
   */
  IoBuf code;

  /*
   * Initial stack state, cached.
   *
   * `NULL` would mean there is none.
   */
  IoBuf initial_stack;
} AssemblerResult;

typedef struct Assembler {
  OptimizationInfo optimization_info;
  const Op* ops;
  
  AssemblerResult (*assemble)(const struct Assembler* self);
} Assembler;

extern const Assembler G_X86_ASSEMBLER;

#endif /* ifndef BFC_ASSEMBLER_H */

