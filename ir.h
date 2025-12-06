/* DEPRECATED */

#ifndef BFC_IR_H
#define BFC_IR_H

#include "op.h"

typedef enum {
  IR_REG_SP,
  IR_REG_IP,
} IrReg;

typedef enum {
  /* Add `imm` to `reg` */
  IR_OP_ADD,
  /* Subtract `imm` from `reg` */
  IR_OP_SUB,
  /* Jump to `imm` if current pointed stack byte is `0` */
  IR_JZ,
  /* Jump to `imm` if current pointed stack byte is NOT `0` */
  IR_JNZ,
  /* Syscall to read a byte into the current pointed stack byte */
  IR_READ,
  /* Syscall to write a the current pointed stack byte */
  IR_WRITE,
} IrOpType;

typedef struct IrOp {
  struct IrOp* next;

  IrOpType type;
  IrReg reg;
  int imm;
} IrOp;

typedef struct ControlFlowGraph {
  IrOp* ir_ops;
  IrOp* branch_ir_op;
} ControlFlowGraph;

/*
 * `ops` is translated into `IrOp` links, and freed.
 */
IrOp* ir_ops_into_ops(Op* ops);

#endif /* ifndef BFC_IR_H */

