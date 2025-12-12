#include "io_buf.h"

#include <assert.h>

#define ADD_IMM8_AT_RSP_SIZE (4)
#define ADD_IMM8_TO_SP_SIZE (4)
#define JZ_NEAR_IMM32_SIZE (6)
#define JZ_SHORT_IMM8_SIZE (2)
#define JNZ_NEAR_IMM32_SIZE JZ_NEAR_IMM32_SIZE
#define JNZ_SHORT_IMM8_SIZE JZ_SHORT_IMM8_SIZE

/* TODO: Can only compile with 1 byte. */

void write_add_imm8_at_rsp(IoBuf* buf, int imm) {
  const char template[] = { 0x80, 0x04, 0x24 };

  write_to_buf(buf, template, sizeof (template));
  write_byte_to_buf(buf, imm);
}

void write_add_imm8_to_sp(IoBuf* buf, int imm) {
  const char template[] = { 0x66, 0x83, 0xc4 };

  write_to_buf(buf, template, sizeof (template));
  write_byte_to_buf(buf, imm);
}

void write_jz_near_imm32(IoBuf* buf, int imm) {
  const char template[] = { 0x0f, 0x84 };

  assert(sizeof (imm) == 4);
  // TODO: Make it cross platform with endianess changes and use int32
  write_to_buf(buf, &imm, 4);
}

void write_jz_short_imm8(IoBuf* buf, int imm) {
  write_byte_to_buf(buf, 0x74);
  write_byte_to_buf(buf, imm);
}

void write_jnz_near_imm32(IoBuf* buf, int imm) {
  const char template[] = { 0x0f, 0x85 };

  assert(sizeof (imm) == 4);
  // TODO: Make it cross platform with endianess changes and use int32
  write_to_buf(buf, &imm, 4);
}

void write_jnz_short_imm8(IoBuf* buf, int imm) {
  write_byte_to_buf(buf, 0x75);
  write_byte_to_buf(buf, imm);
}

void write_test_at_sp(IoBuf* buf) {
  const char template[] = {
    0xa, 0x04, 0x24, /* mov al, [rsp] */
    0x84, 0xc0 /* test al, al */
  };

  write_to_buf(buf, template, sizeof (template));
}

void write_read_syscall(IoBuf* buf) {
  const char template[] = {
    0x48, 0x31, 0xc0, /* xor rax, rax */
    0x48, 0x31, 0xff, /* xor rdi, rdi */
    0x48, 0x89, 0xe6, /* mov rsi, rsp */
    0xba, 0x01, 0x00, 0x00, 0x00, /* mov rdx, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

void write_write_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x01, 0x00, 0x00, 0x00, /* mov rax, 1 */
    0xbf, 0x01, 0x00, 0x00, 0x00, /* mov rdi, 1 */
    0x48, 0x89, 0xe6, /* mov rsi, rsp */
    0xba, 0x01, 0x00, 0x00, 0x00, /* mov rdx, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

void write_exit_success_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x3c, 0x00, 0x00, 0x00, /* mov rax, 0x3c */
    0x48, 0x31, 0xff, /* xor rdi, rdi */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}

void write_exit_fail_syscall(IoBuf* buf) {
  const char template[] = {
    0xb8, 0x3c, 0x00, 0x00, 0x00, /* mov rax, 0x3c */
    0xbf, 0x01, 0x00, 0x00, 0x00, /* mov rdi, 1 */
    0x0f, 0x05 /* syscall */
  };

  write_to_buf(buf, template, sizeof (template));
}
