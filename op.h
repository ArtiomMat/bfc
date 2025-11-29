#ifndef BFC_OP_H
#define BFC_OP_H

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

OpType op_type_from_c(const char c);

void free_ops(Op* ops);

#endif /* define BFC_OP_H */
