#ifndef LOGGER_H
#define LOGGER_H

#define LOG_FIELD_SEPARATOR "^"
#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_ERROR "ERROR"

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <string>

void log_init(const char * aFilename);
void log_close();
void log_info(const char * aMessage, ...);
void log_error(const char * aMessage, ...);
void log_write(const char * aPrefix, const char * aMessage, va_list aArgs);

#endif
