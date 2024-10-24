
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#include "aws_string.h"

/*
 * http_transaction descriptor.
 */
struct http_transaction {
    struct aws_string request;
    struct aws_string response_header;
    struct aws_string response_body;
    struct aws_string response;
    struct aws_string requested_path;
};

/*
 * Build a http_transaction.
 */
struct http_transaction * http_transaction_create(char *request);

/*
 * Destroy http_transaction object.
 */
void http_transaction_destroy(struct http_transaction *http_transaction);

#endif /* HTTP_H */
