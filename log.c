#include "log.h"

#include <stdio.h>
#include <stdarg.h>

void bfc_log(FILE* f, const LogLevel level, const Source* src, const char* fmt, va_list args){
  const char* level_str = NULL;
  int line = 1;
  int column = 1;
  int i = 0;

  if (level > LOG_LEVEL_MAX) {
    return;
  }

  switch (level) {
  case LOG_LEVEL_FATAL:
    level_str = "FATAL";
    break;
  case LOG_LEVEL_ERROR:
    level_str = "ERROR";
    break;
  case LOG_LEVEL_WARN:
    level_str = "WARNING";
    break;
  case LOG_LEVEL_INFO:
    level_str = "INFO";
    break;
  case LOG_LEVEL_DEBUG:
    level_str = "DEBUG";
    break;
  default:
    level_str = "LOG";
    break;
  }

  fprintf(f, "%s: ", level_str);

  if (src) {
    for (i = 0; i < src->i; ++i, ++column) {
      if ('\n' == src->text[i]) {
        ++line;
        column = 0;
      }
    }
  
    fprintf(f, "%s:%i:%i: ", src->path, line, column);
  }

  vfprintf(f, fmt, args);
  putc('\n', stdout);
}

void log_error(const Source* src, const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  bfc_log(stderr, LOG_LEVEL_ERROR, src, fmt, args);
  va_end(args);
}

void log_debug(const Source* src, const char* fmt, ...) {
  va_list args;

  va_start(args, fmt);
  bfc_log(stderr, LOG_LEVEL_DEBUG, src, fmt, args);
  va_end(args);
}

