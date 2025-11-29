/* A brainfuck compiler written in ANSI-C */

#include "bfc.h"
#include "log.h"
#include "op.h"
#include "lexer.h"
#include "source.h"

#include <stdlib.h>

int G_ERROR = 0;

/* TODO: In x86 ADD sets ZF=1 if src+dst=0, so if the last operation is guaranteed to be ADD for
 * the bytes(and not ADD for the stack pointer) we can skip CMP and do only JZ/JNZ for [/].
 */
/* TODO: OP_MUTATE and OP_MOVE with n=1 should be INC, and n=-1 DEC */

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

_done:
  if (ops) {
    free_ops(ops);
  }
  if (text) {
    free(text);
  }

  return G_ERROR;
}

