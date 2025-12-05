#include "source.h"
#include "log.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Source create_source(const char* path, const char* text) {
  Source source;

  assert(text);

  source.text = text;
  source.path = path;
  source.len = strlen(text);
  source.i = 0;
  /* source.delimiter_brackets = calloc(source.len, sizeof(*source.delimiter_brackets)); */

  return source;
}

char* read_from_path(const char* path) {
  FILE* f = NULL;
  char* text = NULL;
  long size = 0;
  size_t read_size = 0;

  assert(path); /* Path is not supposed to be NULL here */

  if (!(f = fopen(path, "r"))) {
    log_error(0, "File could not be opened: %s", path);
    goto failure_;
  }

  if (fseek(f, 0, SEEK_END)) {
    goto seek_failure_;
  }
  size = ftell(f);
  if (fseek(f, 0, SEEK_SET)) {
    goto seek_failure_;
  }

  text = malloc(size + 1);
  if (!text) {
    log_error(0, "Could not allocate buffer for code!");
  }

  read_size = fread(text, 1, size, f);
  if (read_size != size) {
    log_error(0, "Read only %li but expected %i bytes from: %s", read_size, (int)size, path);
    goto failure_;
  }

  text[size] = 0;

  fclose(f);
  return text;

seek_failure_:
    log_error(0, "Could not seek() in file: %s", path);
    goto failure_;

failure_:
  if (f) {
    fclose(f);
  }
  if (text) {
    free(text);
  }
  return NULL;
} 

