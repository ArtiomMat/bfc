#include "assembler.h"
#include "io_buf.h"
#include "op.h"

#include <stddef.h>
#include <assert.h>
#include <stdio.h>

/*
 * Due to jumps being relative to the NEXT instruction after
 * the jumps, we need to store these magical constants, which
 * aren't that magical.
 *
 * These constants is just the size in bytes that an [ or ]
 * take up in the final machine code, that includes
 * the `mov al, [rsp]`, `test al, al` and `je/jne ...`.
 */
/* The short version has an imm8 for the jump */
#define IF_OP_CODE_SIZE_SHORT (7)
/* The near version has an imm32 for the jump */
#define IF_OP_CODE_SIZE_NEAR (11)

/* TODO: Can only compile with 1 byte. */

static void write_add_imm8_at_rsp(IoBuf* buf, int imm) {
  const char template[] = { 0x80, 0x04, 0x24 };

  write_to_buf(buf, template, sizeof (template));
  write_byte_to_buf(buf, imm);
}

static void write_add_imm32_to_rsp(IoBuf* buf, int imm) {
  const char template[] = { 0x48, 0x81, 0xc4 };

  /* TODO: Make it cross platform with endianess changes and use int32 */
  assert(sizeof (imm) == 4);
  write_to_buf(buf, template, sizeof (template));
  write_to_buf(buf, &imm, 4);
}

static void write_jz_near_imm32(IoBuf* buf, int imm) {
  const char template[] = { 0x0f, 0x84 };

  assert(sizeof (imm) == 4);
  write_to_buf(buf, template, sizeof (template));
  /* TODO: Make it cross platform with endianess changes and use int32 */
  write_to_buf(buf, &imm, 4);
}

static void write_jz_short_imm8(IoBuf* buf, int imm) {
  write_byte_to_buf(buf, 0x74);
  write_byte_to_buf(buf, imm);
}

static void write_jnz_near_imm32(IoBuf* buf, int imm) {
  const char template[] = { 0x0f, 0x85 };

  assert(sizeof (imm) == 4);
  write_to_buf(buf, template, sizeof (template));
  /* TODO: Make it cross platform with endianess changes and use int32 */
  write_to_buf(buf, &imm, 4);
}

static void write_jnz_short_imm8(IoBuf* buf, int imm) {
  write_byte_to_buf(buf, 0x75);
  write_byte_to_buf(buf, imm);
}

static void write_test_at_sp(IoBuf* buf) {
  const char template[] = {
    0x8a, 0x04, 0x24, /* mov al, [rsp] */
    0x84, 0xc0 /* test al, al */
  };

  write_to_buf(buf, template, sizeof (template));
}

static void write_read_syscall(IoBuf* buf) {
  const char template[] = {
    0x48, 0x31, 0xc0, /* xor rax, rax */
    0x48, 0x31, 0xff, /* xor rdi, rdi */
    0x48, 0x89, 0xe6, /* mov rsi, rsp */
    0xba, 0x01, 0x00, 0x00, 0x00, /* mov rdx, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

static void write_write_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x01, 0x00, 0x00, 0x00, /* mov rax, 1 */
    0xbf, 0x01, 0x00, 0x00, 0x00, /* mov rdi, 1 */
    0x48, 0x89, 0xe6, /* mov rsi, rsp */
    0xba, 0x01, 0x00, 0x00, 0x00, /* mov rdx, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

static void write_exit_success_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x3c, 0x00, 0x00, 0x00, /* mov rax, 0x3c */
    0x48, 0x31, 0xff, /* xor rdi, rdi */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

static void write_exit_fail_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x3c, 0x00, 0x00, 0x00, /* mov rax, 0x3c */
    0xbf, 0x01, 0x00, 0x00, 0x00, /* mov rdi, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

static void write_op_code(Op* op) {
  int i = 0;

  assert(!op->code.ptr); /* op->code must be NULL_IO_BUF */

  create_io_buf(&op->code);

  switch (op->type) {
  case OP_MOVE:
    /* -op->n because the stack goes up */
    write_add_imm32_to_rsp(&op->code, -op->n);
    break;
  
  case OP_MUTATE:
    write_add_imm8_at_rsp(&op->code, op->n);
    break;

  /* TODO: OP_PRINT/INPUT don't support n>1 */
  case OP_PRINT:
    for (i = 0; i < op->n; ++i) {
      write_write_syscall(&op->code);
    }
    break;

  case OP_INPUT:
    for (i = 0; i < op->n; ++i) {
      write_read_syscall(&op->code);
    }
    break;

  case OP_IF_NOT_0:
  case OP_IF_0:
  default:
    assert(0); /* Not made for this */
    break;
  }
}

/*
 * Asserts that between `if_0_op` and `if_not_0_op` all op's
 * `code` fields have a defined size.
 *
 * Note: This assert means that any inner ifs must have already
 * been written. You can achieve this with recursion.
 *
 * Sets up the `code` fields for both ops.
 */
static void write_ifs_op_codes(Op* if_0_op, Op* if_not_0_op) {
  Op* op = NULL;
  /* The distance between the two in instruction bytes */
  int sizes_sum = 0;

  assert(if_0_op);
  assert(if_not_0_op);

  assert(if_0_op->type == OP_IF_0);
  assert(if_not_0_op->type == OP_IF_NOT_0);
  assert(if_0_op->next); /* The if_0_op must have a next. */

  /* Set up sizes_sum */
  for (op = if_0_op->next; op != if_not_0_op; op = op->next) {
    assert(op); /* op must lead to if_not_0_op at some point. */
    assert(op->code.ptr && op->code.raw_size); /* All ops in-between must be initialized. */

    sizes_sum += op->code.size;
  }

  assert(!if_0_op->code.ptr); /* code must be NULL_IO_BUF */
  create_io_buf(&if_0_op->code);
  write_test_at_sp(&if_0_op->code);
  
  assert(!if_not_0_op->code.ptr); /* code must be NULL_IO_BUF */
  create_io_buf(&if_not_0_op->code);
  write_test_at_sp(&if_not_0_op->code);

  /* FIXME: Jumps are not correct */

  /*
   * We add IF_OP_CODE_SIZE_SHORT because it is also included as part of the short jump.
   * HISTORY: Not including it caused such a horrible edge case that took so much time to debug.
   */
  if (sizes_sum + IF_OP_CODE_SIZE_SHORT < 128) {
    sizes_sum += IF_OP_CODE_SIZE_SHORT;
    write_jz_short_imm8(&if_0_op->code, sizes_sum);
    write_jnz_short_imm8(&if_not_0_op->code, -sizes_sum);
  } else {
    sizes_sum += IF_OP_CODE_SIZE_NEAR;
    write_jz_near_imm32(&if_0_op->code, sizes_sum);
    write_jnz_near_imm32(&if_not_0_op->code, -sizes_sum);
  }
}

/*
 * All it needs is that you give it the first instance
 * the `OP_IF_0`, then it recursively writes the `code`
 * fields for it and its inner ops in inner-to-outer order.
 * 
 * Asserts that return value will not be NULL.
 *
 * Returns the value of the matching `OP_IF_NOT_0` of
 * this `if_0_op`.
 */
static Op* recursive_write_if_op_codes(Op* if_0_op) {
  Op* op = NULL;
  Op* if_not_0_op = NULL;
  
  assert(if_0_op);

  assert(if_0_op->type == OP_IF_0);
  assert(if_0_op->next); /* The if_0_op must have a next. */
  
  for (op = if_0_op->next; op; op = op->next) {
    if (op->type == OP_IF_0) {
      /* We found an inner if_0_op */
      
      op = recursive_write_if_op_codes(op);
      
      continue; /* op->next will be after the OP_IF_NOT_0 */
    } else if (op->type == OP_IF_NOT_0) {
      /* We found the matching if */
      
      write_ifs_op_codes(if_0_op, op);
      if_not_0_op = op;

      break;
    }
  }

  assert(if_not_0_op); /* Above code must set this to a valid value */
  return if_not_0_op;
}

void assemble_x86_64(Assembler* self, AssemblerResult* result) {
  Op* op = NULL;

  for (op = self->ops; op; op = op->next) {
    if (op->type == OP_IF_0 || op->type == OP_IF_NOT_0) {
      /* Reserved for another pass where we know how much to jump */
      continue;
    }

    write_op_code(op);
  }

  /* The aforementioned "another pass" */
  for (op = self->ops; op; op = op->next) {
    if (op->type == OP_IF_0) {
      op = recursive_write_if_op_codes(op);
      assert(op);
    }

  }

  /* TODO: Delete */
  IoBuf code;
  create_io_buf(&code);
  write_add_imm32_to_rsp(&code, -30000);

  FILE* f = fopen("bfcbin", "wb");
  for (op = self->ops; op; op = op->next) {
    write_to_buf(&code, op->code.ptr, op->code.size);
  }
  write_exit_success_syscall(&code);

  fwrite(code.ptr, 1, code.size, f);
  fclose(f);
}

