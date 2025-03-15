#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aws_buffer.h"

void
aws_buffer_init_empty(struct aws_buffer *aws_buffer)
{
    assert(aws_buffer);

    aws_buffer->buffer = NULL;
    aws_buffer->length = 0;
    aws_buffer->error = 0;
    aws_buffer->is_string = false;

}

void
aws_buffer_destroy(struct aws_buffer *aws_buffer)
{
    assert(aws_buffer);

    free(aws_buffer->buffer);
}

char*
aws_buffer_get_buffer(struct aws_buffer *aws_buffer)
{
    return aws_buffer->buffer;
}

int
aws_buffer_get_length(struct aws_buffer *aws_buffer)
{
    return aws_buffer->length;
}

int
aws_buffer_append_buffer(struct aws_buffer *aws_buffer, const char *buffer,
                         size_t buffer_size)
{
    assert(aws_buffer);
    assert(buffer);

    size_t new_length;

    if (aws_buffer->error == 0) {
        new_length = aws_buffer->length + buffer_size;
        aws_buffer->buffer = realloc(aws_buffer->buffer, new_length + 1);

        if (!aws_buffer->buffer) {
            aws_buffer->error = errno;
        } else {

            memcpy(&aws_buffer->buffer[aws_buffer->length], buffer,
                   buffer_size);
            aws_buffer->length = new_length;

            if (aws_buffer->is_string) {
                aws_buffer->buffer[new_length] = '\0';
            }
        }
    }

    if (aws_buffer->error == 0) {
        return 0;
    } else {
        return aws_buffer->error;
    }
}

int
aws_buffer_append_format(struct aws_buffer *aws_buffer, char *format, ...)
{
    assert(aws_buffer);

    int new_length = 0;
    size_t zero_size = 0;
    char *p = NULL;
    va_list vlist;

    if (aws_buffer->error == 0) {
        va_start(vlist, format);
        new_length = vsnprintf(p, zero_size, format, vlist);
        va_end(vlist);

        aws_buffer->buffer =
            realloc(aws_buffer->buffer, aws_buffer->length + new_length + 1);

        if (!aws_buffer->buffer) {
            aws_buffer->error = errno;
        } else {
            va_start(vlist, format);
            vsnprintf(&aws_buffer->buffer[aws_buffer->length], new_length + 1,
                      format, vlist);
            va_end(vlist);

            if (aws_buffer->is_string) {
                aws_buffer->buffer[new_length] = '\0';
            }

            aws_buffer->length = aws_buffer->length + new_length;
        }
    }

    if (aws_buffer->error == 0) {
        return 0;
    } else {
        return aws_buffer->error;
    }
}


int aws_buffer_append_front(struct aws_buffer *aws_buffer, const char *buffer, int buffer_size)
{

    size_t new_length;

    new_length = aws_buffer->length + buffer_size;
    aws_buffer->buffer = realloc(aws_buffer->buffer, new_length + 3);
    memmove(aws_buffer->buffer + buffer_size, aws_buffer->buffer, aws_buffer->length);

    if (!aws_buffer->buffer) {
        aws_buffer->error = errno;
    } else {
        memcpy(&aws_buffer->buffer[0], buffer, buffer_size);

        if (aws_buffer->is_string) {
            aws_buffer->buffer[new_length] = '\0';
        }

        aws_buffer->length = new_length;
    }

    return aws_buffer->error;

}


