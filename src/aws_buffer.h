#ifndef AWS_BUFFER_H
#define AWS_BUFFER_H

struct aws_buffer {
    unsigned char *buffer;
    size_t length;
    int error;
};

/*
 * Initialize aws_buffer object.
 */
void aws_buffer_init_empty(struct aws_buffer *aws_buffer);

/*
 * Append formated string to an aws_buffer instance.
 */
int aws_buffer_append_format(struct aws_buffer *aws_buffer, char *format, ...);

/*
 * Free all allocated ressources.
 */
void aws_buffer_destroy(struct aws_buffer *aws_buffer);

#endif /* AWS_BUFFER_H */
