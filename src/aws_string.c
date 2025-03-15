#include <assert.h>
#include <ctype.h>
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

    aws_buffer_init_empty(&aws_string->buffer);
    aws_string->buffer.is_string = true;
}

void
aws_string_destroy(struct aws_string *aws_string)
{
    aws_buffer_destroy(&aws_string->buffer);
}

int
aws_string_get_length(struct aws_string *aws_string)
{
    return aws_buffer_get_length(&aws_string->buffer);
}

char *
aws_string_get_buffer(struct aws_string *aws_string)
{
    return aws_buffer_get_buffer(&aws_string->buffer);
}

int
aws_string_append_front(struct aws_string *aws_string,
                               const char *buffer, size_t buffer_size)
{
    assert(aws_string);
    assert(buffer);

    int error = 0;

    if (aws_string->buffer.error == 0) {
        error = aws_buffer_append_front(&(aws_string)->buffer, buffer, buffer_size);
    }

    return error;
}

int
aws_string_append_buffer(struct aws_string *aws_string,
                               const char *buffer, size_t buffer_size)
{
    assert(aws_string);
    assert(buffer);

    int error = 0;

    if (aws_string->buffer.error == 0) {
        error = aws_buffer_append_buffer(&(aws_string)->buffer, buffer, buffer_size);
    }

    return error;
}

char *
aws_string_extract_after_last_dot(struct aws_string *aws_string)
{
    assert(aws_string);

    char *sub_string = strrchr(aws_buffer_get_buffer(&aws_string->buffer), '.');

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

    start_index.cursor = aws_buffer_get_buffer(&aws_string->buffer);
    error = aws_string_iterate_until(&start_index, start_separators);

    if (!error) {
        end_index.cursor = start_index.cursor;

        while ((*end_index.cursor != '\0') && !isspace(*end_index.cursor)) {
            end_index.cursor++;
        }
    }

    if (!error) {
        length = (end_index.cursor - start_index.cursor);
        error = aws_buffer_append_buffer(&result->buffer, start_index.cursor, length);
    }

    if (error) {
        result->buffer.buffer = NULL;
    }
}
