#ifndef STRING_BUILDER_MEMSTREAM_H
#define STRING_BUILDER_MEMSTREAM_H

/*
 * String builder backed by POSIX open_memstream()
 * Writes go directly into a kernel-managed buffer that grows automatically.
 * sb_get_string() flushes and returns the internal buffer - do NOT free() it,
 * the buffer is invalidated after sb_destroy():
 *
 *   sb_memstream* sb = sb_create();
 *   sb_append(sb, "hello, ");
 *   sb_fappend(sb, "world %d", 42);
 *   const char* s = sb_get_string(sb);  // points into sb, do NOT free()
 *   ...
 *   sb_destroy(sb); // frees also the get_string() buffer
 *
 * Note: this implementation is slow in the init faze but fast in the append faze,
 * because it uses a kernel-managed buffer that grows automatically.
 */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief String builder handle.
 *
 * Treat as opaque; allocate with sb_create() and release with sb_destroy().
 */
typedef struct {
    char* buffer;
    size_t size;
    FILE* sb;
} sb_memstream;

/**
 * @brief Allocate and initialise a new string builder.
 *
 * @return Pointer to a new sb_memstream, or NULL on failure.
 */
sb_memstream* sb_create(void);

/**
 * @brief Append a string to the builder.
 *
 * @param s   The string builder.
 * @param str Null-terminated string to append.
 *
 * @return true on success, false on allocation failure.
 */
bool sb_append(sb_memstream* s, const char* str);

/**
 * @brief Append a printf-style formatted string to the builder.
 *
 * @param s      The string builder.
 * @param format printf format string.
 * @param ...    Format arguments.
 *
 * @return true on success, false on allocation failure.
 */
bool sb_fappend(sb_memstream* s, const char* format, ...);

/**
 * @brief Flush and return the current buffer contents.
 *
 * @param s The string builder.
 * @return Pointer to the internal null-terminated buffer. Valid until the next
 *         append call or sb_destroy(). Do NOT free() this pointer.
 */
const char* sb_get_string(const sb_memstream* s);

/**
 * @brief Free all resources owned by the string builder.
 *
 * @param s The string builder to destroy. No-op if NULL.
 */
void sb_destroy(sb_memstream* s);

#ifdef STRING_BUILDER_MEMSTREAM_IMPLEMENTATION

sb_memstream* sb_create(void) {
    sb_memstream* s = malloc(sizeof(*s));
    if (s == NULL) {
        perror("Failed to allocate memory for sb_memstream");
        return NULL;
    }
    s->sb = open_memstream(&s->buffer, &s->size);
    if (s->sb == NULL) {
        perror("Failed to open memstream");
        free(s);
        return NULL;
    }
    return s;
}

bool sb_append(sb_memstream* s, const char* str) { return fprintf(s->sb, "%s", str) >= 0; }

bool sb_fappend(sb_memstream* s, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int r = vfprintf(s->sb, format, args);
    va_end(args);
    return (r >= 0);
}

const char* sb_get_string(const sb_memstream* s) {
    fflush(s->sb);
    return s->buffer;
}

void sb_destroy(sb_memstream* s) {
    if (s == NULL) return;
    fclose(s->sb);
    free(s->buffer);
    free(s);
}

#endif  // STRING_BUILDER_MEMSTREAM_IMPLEMENTATION
#endif  // STRING_BUILDER_MEMSTREAM_H
