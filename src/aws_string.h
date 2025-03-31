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
 * Destroy aws_string object.
 */
void aws_string_destroy(struct aws_string *aws_string);

/*
 * Set is_string attribute of aws_string object.
 */
void aws_string_set_is_string(struct aws_string *aws_string, bool is_string);

/*
 * Get string length.
 */
int aws_string_get_length(struct aws_string *aws_string);

/*
 * Get string buffer.
 */
char * aws_string_get_buffer(struct aws_string *aws_string);

struct aws_buffer * aws_string_get_buffer2(struct aws_string *aws_string);
/*
 * Get error status.
 */
int aws_string_get_error(struct aws_string *aws_string);

/*
 * Insert a buffer at the front of an aws_string instance.
 */
int aws_string_append_front(struct aws_string *aws_string,
                                   const char *buffer, size_t buffer_size);

/*
 * Append a buffer to an aws_string instance.
 */
int aws_string_append_buffer(struct aws_string *aws_string,
                             const char *buffer, size_t buffer_size);

/*
 * Extract a sub string after last dot found in aws_string instance.
 */
char * aws_string_extract_after_last_dot(struct aws_string *aws_string);

/*
 * Extract an aws_string between start and stop separators.
 * Result buffer is null if not found.
 */
void aws_string_extract_between(struct aws_string *aws_string,
                                const char *start_separators,
                                const char *end_separators,
                                struct aws_string *result);

#endif /* AWS_STRING_H */
