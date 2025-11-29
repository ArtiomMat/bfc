#ifndef BFC_LEXER_H
#define BFC_LEXER_H

#include "source.h"
#include "op.h"

/*
 * Returns NULL if there is nothing to tokenize.
 */
Op* lex(Source* src);

#endif /* define BFC_LEXER_H */
