#ifndef BFC_LOG_H
#define BFC_LOG_H

#include <stdio.h>
#include <stdarg.h>

#include "source.h"

typedef enum {
  LOG_LEVEL_FATAL = 10,
  LOG_LEVEL_ERROR = 20,
  LOG_LEVEL_WARN = 30,
  LOG_LEVEL_INFO = 40,
  LOG_LEVEL_DEBUG = 50,
} LogLevel;

void bfc_log(FILE* f, const LogLevel level, const Source* src, const char* fmt, va_list args);
void log_error(const Source* src, const char* fmt, ...);

#endif /* define BFC_LOG_H */

