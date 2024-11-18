#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aws_string.h"

void
aws_string_init_empty(struct aws_string *aws_string)
{
    assert(aws_string);

    aws_string->buffer = NULL;
    aws_string->length = 0;
    aws_string->error = 0;
}

int
aws_string_append_format(struct aws_string *aws_string, char *format, ...)
{
    assert(aws_string);

    int new_length = 0;
    size_t zero_size = 0;
    char *p = NULL;
    va_list vlist;

    if (aws_string->error == 0) {
        va_start(vlist, format);
        new_length = vsnprintf(p, zero_size, format, vlist);
        va_end(vlist);

        aws_string->buffer =
            realloc(aws_string->buffer, aws_string->length + new_length + 1);

        if (!aws_string->buffer) {
            aws_string->error = errno;
        } else {
            va_start(vlist, format);

            vsnprintf(&aws_string->buffer[aws_string->length], new_length + 1,
                      format, vlist);
            va_end(vlist);
            aws_string->length = aws_string->length + new_length;
        }
    }

    if (aws_string->error == 0) {
        return 0;
    } else {
        return aws_string->error;
    }
}

int
aws_string_append_buffer(struct aws_string *aws_string, const char *buffer,
                         size_t buffer_size)
{
    assert(aws_string);
    assert(buffer);

    size_t new_length;

    if (aws_string->error == 0) {
        new_length = aws_string->length + buffer_size;
        aws_string->buffer = realloc(aws_string->buffer, new_length + 1);

        if (!aws_string->buffer) {
            aws_string->error = errno;
        } else {
            memcpy(&aws_string->buffer[aws_string->length], buffer,
                   buffer_size);
            aws_string->buffer[new_length] = '\0';
            aws_string->length = new_length;
        }
    }

    if (aws_string->error == 0) {
        return 0;
    } else {
        return aws_string->error;
    }
}

int
aws_string_append(struct aws_string *aws_string,
                  struct aws_string *aws_string_to_append)
{
    assert(aws_string);
    assert(aws_string_to_append);

    size_t length;

    if (aws_string->error == 0) {
        length = aws_string->length + aws_string_to_append->length;

        aws_string->buffer = realloc(aws_string->buffer, length + 1);

        if (!aws_string->buffer) {
            aws_string->error = errno;
        } else {
            memcpy(&aws_string->buffer[aws_string->length],
                   aws_string_to_append->buffer,
                   aws_string_to_append->length + 1);
            aws_string->length = length;
        }
    }

    if (aws_string->error == 0) {
        return 0;
    } else {
        return aws_string->error;
    }
}

void
aws_string_destroy(struct aws_string *aws_string)
{
    assert(aws_string);

    free(aws_string->buffer);
}

char *
aws_string_extract_after_last_dot(struct aws_string *aws_string)
{
    assert(aws_string);

    char *sub_string = strrchr(aws_string->buffer, '.');

    return sub_string;
}

static int
aws_string_iterate_until(struct aws_string_iterator *iterator,
                         const char *separators)
{
    assert(iterator);
    assert(separators);

    int error = 0;

    if (iterator->cursor) {
        char *separator;
        char *next;

        separator = strpbrk(iterator->cursor, separators);

        if (separator) {
            size_t nrSeparators;
            nrSeparators = strspn(separator, separators);
            next = &separator[nrSeparators];
        } else {
            next = NULL;
        }

        iterator->cursor = next;

    } else {
        error = EINVAL;
    }

    return error;
}

void
aws_string_extract_between(struct aws_string *aws_string,
                           const char *start_separators,
                           const char *end_separators,
                           struct aws_string *result)
{
    assert(aws_string);
    assert(start_separators);
    assert(end_separators);
    assert(result);

    struct aws_string_iterator start_index;
    struct aws_string_iterator end_index;
    int error;
    size_t length;

    start_index.cursor = aws_string->buffer;
    error = aws_string_iterate_until(&start_index, start_separators);

    if (!error) {
        end_index.cursor = start_index.cursor;
        error = aws_string_iterate_until(&end_index, end_separators);
    }

    if (!error) {
        length = ((end_index.cursor - 2) - start_index.cursor) + 1;
        error = aws_string_append_buffer(result, start_index.cursor, length);
    }

    if (error) {
        result->buffer = NULL;
    }
}
