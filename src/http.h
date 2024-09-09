
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

/*
 * http_transaction descriptor.
 */
struct http_transaction {
    char *request;
    char *response;
    char *requested_path;
    size_t response_len;
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
