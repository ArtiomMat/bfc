#ifndef IO_BUF_H
#define IO_BUF_H

#define NULL_IO_BUF ((IoBuf){0})

/*
 * Just a vector of chars.
 * I renamed+refactored it like 3 times already and too lazy to rename it
 * again to Vec or something, so I will just leave it as is.
 */
typedef struct {
  char* ptr;
  int size;
  int raw_size;
} IoBuf;

int create_io_buf(IoBuf* buf);

void free_io_buf(IoBuf* buf);

int write_to_buf(IoBuf* buf, const void* data, const int size);

int write_byte_to_buf(IoBuf* buf, const char byte);

#endif /* ifndef IO_BUF_H */

