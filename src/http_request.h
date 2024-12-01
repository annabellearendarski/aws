
#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stddef.h>

#include "aws_string.h"

/*
 * http_request descriptor.
 */

struct http_request {
    struct aws_string request;
};

/*
 * Initialize a http_request.
 */
void http_request_init(struct http_request *http_request);

/*
 * Extract ressource path from an http_request;
 * Returns 0 if ressource path found, EINVAL if not.
 */
int http_retrieve_requested_ressource_path(struct http_request *http_request,
                                        struct aws_string *extracted_path);

/*
 * Append text to an HTTP request
 *
 * Returns 0 if the request is valid and complete, EAGAIN if valid but
 * incomplete, EINVAL if the request is invalid
 */
int http_request_append(struct http_request *http_request, const char *buffer,
                        int nr_bytes_rcv);

/*
 * Destroy http_request object.
 */
void http_request_destroy(struct http_request *http_request);

#endif /* HTTP_REQUEST_H */
