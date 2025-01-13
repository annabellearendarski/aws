#ifndef AWS_BUFFER_H
#define AWS_BUFFER_H

#include <stdbool.h>

/*
 * aws_buffer object.
 */
struct aws_buffer {
    char *buffer;
    size_t length;
    int error;
    bool is_string;
};

/*
 * Initialize aws_buffer object.
 */
void aws_buffer_init_empty(struct aws_buffer *aws_buffer);

/*
 * Free all allocated ressources.
 */
void aws_buffer_destroy(struct aws_buffer *aws_buffer);

/*
 * Append formated string to an aws_buffer instance.
 */
int aws_buffer_append_format(struct aws_buffer *aws_buffer, char *format, ...);

/*
 * Append a buffer to an aws_buffer instance.
 */
int aws_buffer_append_buffer(struct aws_buffer *aws_buffer, const char *buffer,
                            size_t buffer_size);

#endif /* AWS_BUFFER_H */
