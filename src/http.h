
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#include "awsString.h"

/*
 * http_transaction descriptor.
 */
struct http_transaction {
    char *request;
    struct awsString response_header;
    struct awsString response_body;
    struct awsString response;
    char *requested_path;
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
