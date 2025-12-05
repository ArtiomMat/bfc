#ifndef BFC_SOURCE_H
#define BFC_SOURCE_H

/*
 * Represents an extended version of the source code,
 * with members to easen lexing.
 */
typedef struct{
  const char* text;

  const char* path;

  /*
   * The size as `len`.
   *
   * The purpose of this global is to mark `[` and `]` if they don't signify
   * ops but rather delimiters for previous `]` and `[` respectively.
   *
   * For example, if `text[10]` has `[` and then in `text[15]` there is `]`
   * that then `delimiter_brackets[15]` is marked with `1` to show that it's
   * only a delimiter.
   char* delimiter_brackets;
   */

  /*
   * Cache it because we already calculate it anyway.
   *
   * Length not including null terminator.
   */
  int len;

  /*
   * Index within text.
   */
  int i;
} Source;

Source create_source(const char* path, const char* text);

/*
 * Return `NULL` if failed.
 */
char* read_from_path(const char* path);

#endif /* BFC_SOURCE_H */

