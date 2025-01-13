#ifndef AWS_STRING_H
#define AWS_STRING_H

#include <stddef.h>
#include "aws_buffer.h"

/*
 * aws_string object.
 */
struct aws_string {
    struct aws_buffer buffer;
};

/*
 * aws_string iterator.
 */
struct aws_string_iterator {
    const char *cursor;
};

/*
 * Initialize aws_string object.
 */
void aws_string_init_empty(struct aws_string *aws_string);

/*
 * Insert a buffer at the front of an aws_string instance.
 */
int aws_string_insert_front_buffer(struct aws_string *aws_string,
                                   const char *buffer, size_t buffer_size);
/*
 * Append an aws_string object to an aws_string instance.
 */
int aws_string_append(struct aws_string *aws_string, struct aws_string *aws_appended_string);

char * aws_string_extract_after_last_dot(struct aws_string *aws_string);

/*
 * Extract an aws_string between start and stop separators.
 * result buffer is null if not found
 */
void aws_string_extract_between(struct aws_string *aws_string,
                                const char *start_separators,
                                const char *end_separators,
                                struct aws_string *result);

#endif /* AWS_STRING_H */
