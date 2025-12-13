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
  Op* ops;
  
  void (*assemble)(struct Assembler* self, AssemblerResult* result);
} Assembler;

extern const Assembler G_X86_64_ASSEMBLER_TEMPLATE;

#endif /* ifndef BFC_ASSEMBLER_H */

