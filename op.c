#include "op.h"

#include <assert.h>
#include <stdlib.h>

void reset_op(Op* op) {
  op->next = NULL;
  op->type = OP_INVALID;
  op->src_start = -1;
  op->src_end = -1;
  op->n = 0;
  op->vaddress = 0;
}

OpType op_type_from_c(const char c) {
  switch (c) {
  case '+':
  case '-':
    return OP_MUTATE;
  case '>':
  case '<':
    return OP_MOVE;
  case ',':
    return OP_INPUT;
  case '.':
    return OP_PRINT;
  case '[':
    return OP_IF_0;
  case ']':
    return OP_IF_NOT_0;
  default:
    return OP_SKIP;
  }
}

const char* str_from_op_type(OpType type) {
  switch (type) {
  case OP_PRINT:
    return "PRINT";
  case OP_INPUT:
    return "INPUT";
  case OP_MUTATE:
    return "MUTATE";
  case OP_MOVE:
    return "MOVE";
  case OP_IF_0:
    return "IF0";
  case OP_IF_NOT_0:
    return "IF!0";
  case OP_SKIP:
    return "NOP";
  default:
    return "INVALID";
  };
}

void free_ops(Op* first_op) {
  Op* current_op;

  assert(first_op);

  for (current_op = first_op; current_op; /* Inside */) {
    Op* next = current_op->next;
    free(current_op);
    current_op = next;
  }
}

