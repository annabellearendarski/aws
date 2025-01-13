
#ifndef HTTP_H
#define HTTP_H

#include <stddef.h>

#include "aws_buffer.h"
#include "aws_string.h"


/*
 * http_transaction descriptor.
 */
struct http_response {
    struct aws_string header;
    struct aws_buffer body;
};

/*
 * Initialize a http_response.
 */
void http_response_init(struct http_response *http_response);

/*
 * Build a http_transaction.
 */
struct http_transaction * http_transaction_create(char *request);

/*
 * Build an http reponse to a folder request.
 */
int http_response_add_response_for_folder_request(struct http_response *response,
                                                  struct entry_list *list,
                                                  const char *requested_path);

/*
 * Build an http reponse to a file request.
 */
int
http_response_add_response_for_file_request(struct http_response *response,
                                            const char *mime_type,
                                            off_t file_size,
                                            const int file_fd);
/*
 * Build an http error reponse.
 */
int
http_response_add_response_error(struct http_response *response);

/*
 * Destroy http_transaction object.
 */
void http_response_destroy(struct http_response *response);

#endif /* HTTP_H */
