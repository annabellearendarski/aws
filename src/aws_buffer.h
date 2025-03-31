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
 * Set is_string attribute.
 */
void aws_buffer_set_is_string(struct aws_buffer *aws_buffer, bool is_string);

/*
 * Get buffer from aws_buffer object.
 */
char * aws_buffer_get_buffer(struct aws_buffer *aws_buffer);

/*
 * Get the length of the buffer.
 */
int aws_buffer_get_length(struct aws_buffer *aws_buffer);

/*
 * Get error status of the aws_buffer object.
 */
int aws_buffer_get_error(struct aws_buffer *aws_buffer);

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

/*
 * Insert a buffer at the front of an aws_buffer instance.
 */
int aws_buffer_append_front(struct aws_buffer *aws_buffer, const char *buffer, int buffer_size);

#endif /* AWS_BUFFER_H */
