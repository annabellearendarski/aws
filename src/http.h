
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

struct http_transaction
{
    char *request;
    char *response;
    char *requested_path;
    size_t response_len;
};

struct http_transaction * http_response_create(char *request);

#endif /* HTTP_H */
