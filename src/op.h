#ifndef BFC_OP_H
#define BFC_OP_H

#include "io_buf.h"

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
  
  /*
   * During lexing `n` = How many operations in code to jump.
   * After normalization `n` = How many `Op` to jump.
   */
  OP_IF_0,
  /*
   * During lexing `n` = How many operations in code to jump.
   * After normalization `n` = How many `Op` to jump.
   */
  OP_IF_NOT_0,
} OpType;

typedef struct Op {
  /* NULL means no next */
  struct Op* next;
  
  OpType type;

  /*
   * First index in source code where it starts.
   */
  int src_start;
  /*
   * Index of the terminating character for this `Op`.
   */
  int src_end;

  /*
   * Depends on type and stage, read OpType.
   */
  int n;

  /*
   * Relevant only for assembly.
   * Virtual-address in executable where the operation starts.
   *
   * NOTE: Operation may be any amount of bytes, and can be variable even
   * for the same `type`, if presented with optimization opportunities.
   */
  /* int vaddress; */

  /*
   * For the assembler, at first is uninitialized.
   *
   * The assembler writes the machine code here
   */
  IoBuf code;
} Op;

/*
 * Sets `op` to 0 equivalents, predominantly `type` is set to `OP_INVALID`
 */
void reset_op(Op* op);

OpType op_type_from_c(const char c);

const char* str_from_op_type(OpType type);

void free_ops(Op* ops);

#endif /* ifndef BFC_OP_H */
