#include "io_buf.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 16

int create_io_buf(IoBuf* buf) {
  buf->ptr = malloc(INITIAL_SIZE);
  if (!buf->ptr) {
    return 0;
  }

  buf->raw_size = INITIAL_SIZE;
  buf->size = 0;

  return 1;
}

void free_io_buf(IoBuf* buf) {
  free(buf->ptr);
  *buf = NULL_IO_BUF;
}

static int double_raw_size_until_size_fits(IoBuf* buf) {
  while (buf->raw_size < buf->size) {
    buf->raw_size *= 2;
    buf->ptr = realloc(buf->ptr, buf->raw_size);
    if (!buf->ptr) {
      return 0; 
    }
  }

  return 1;
}

int write_to_buf(IoBuf* buf, const void* data, const int size) {
  int i = buf->size;

  assert(buf->ptr && buf->raw_size);

  buf->size += size;
  if (!double_raw_size_until_size_fits(buf)) {
    return 0;
  }

  memcpy(buf->ptr + i, data, size);

  return 1;
}

int write_byte_to_buf(IoBuf* buf, const char byte) {
  assert(buf->ptr && buf->raw_size);
  
  ++buf->size;
  if (!double_raw_size_until_size_fits(buf)) {
    return 0;
  }

  buf->ptr[buf->size - 1] = byte;
  
  return 1;
}

