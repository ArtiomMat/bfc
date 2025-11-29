/* A brainfuck compiler written in ANSI-C */

#include "bfc.h"
#include "log.h"
#include "op.h"
#include "lexer.h"
#include "source.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

int G_ERROR = 0;

int main(const int argc, const char** argv) {
  /* TODO: Everything up until the first input instruction can be cached. */
  Op* ops;
  const char* path = "stdin";
  char* text = NULL;

  if (argc < 2) {
    log_error(0, "Missing file!");
    goto _done;
  } else {
    path = argv[1];
    text = read_from_path(path);
    if (G_ERROR) {
      goto _done;
    }
  }

  Source example = create_source(path, text);
  
  ops = lex(&example);
  if (G_ERROR) {
    goto _done;
  }
  
  for (; ops; ops = ops->next) {
    printf("%i %i\n", ops->type, ops->n);
  }

_done:
  if (ops) {
    free_ops(ops);
  }
  if (text) {
    free(text);
  }

  return G_ERROR;
}

