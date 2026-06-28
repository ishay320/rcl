#include "ansi.h"

#include <stdarg.h>
#include <stdio.h>
/**
 * Log an informational message to stdout.
 *
 * you can use the define `REPLACE_PRINTF` to replace the printf and
 * fprintf functions with the log_info and log_error functions respectively.
 */

/**
 * @brief Log an informational message to stdout.
 *
 * @param format printf-style format string.
 * @param ...    Format arguments.
 *
 * @return The number of characters printed, or a negative value if an error
 * occurs.
 */
static inline int log_info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stdout, AN_GREEN "[INFO] %s:%d: " AN_RESET, __FILE__, __LINE__);
  int ret = vfprintf(stdout, format, args);
  va_end(args);
  return ret;
}

/**
 * @brief Log a warning message to stderr.
 *
 * @param format printf-style format string.
 * @param ...    Format arguments.
 *
 * @return The number of characters printed, or a negative value if an error
 * occurs.
 */
static inline int log_warning(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, AN_YELLOW "[WARNING] %s:%d: " AN_RESET, __FILE__, __LINE__);
  int ret = vfprintf(stderr, format, args);
  va_end(args);
  return ret;
}

/**
 * @brief Log an error message to stderr.
 *
 * @param format printf-style format string.
 * @param ...    Format arguments.
 *
 * @return The number of characters printed, or a negative value if an error
 * occurs.
 */
static inline int log_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, AN_RED "[ERROR] %s:%d: " AN_RESET, __FILE__, __LINE__);
  int ret = vfprintf(stderr, format, args);
  va_end(args);
  return ret;
}

#ifdef REPLACE_PRINTF
#define printf(...) log_info(__VA_ARGS__)
#define fprintf(stream, ...)                                                   \
  ({                                                                           \
    int ret;                                                                   \
    if (stream == stdout) {                                                    \
      ret = log_info(__VA_ARGS__);                                             \
    } else if (stream == stderr) {                                             \
      ret = log_error(__VA_ARGS__);                                            \
    } else {                                                                   \
      ret = fprintf(stream, __VA_ARGS__);                                      \
    }                                                                          \
    ret;                                                                       \
  })
#endif // REPLACE_PRINTF