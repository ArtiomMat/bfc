#ifndef BFC_LEXER_H
#define BFC_LEXER_H

#include "source.h"
#include "op.h"

/*
 * On success, returns `1` and sets `*first_op_ptr` to the first `Op` lexed.
 *
 * On failure, returns `0`.
 */
int lex(Source* src, Op** first_op_ptr);

#endif /* define BFC_LEXER_H */
