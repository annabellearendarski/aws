#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "aws_string.h"
#include "http_request.h"

#define HTTP_REQUEST_MAX_SIZE 1024

void
http_request_init(struct http_request *http_request)
{
    aws_string_init_empty(&http_request->request);
}

int
http_request_append(struct http_request *http_request, const char *buffer,
                    int nr_bytes_rcv)
{
    int error = 0;
    bool is_end_of_request = false;

    if ((aws_string_get_length(&http_request->request) + nr_bytes_rcv)
         > HTTP_REQUEST_MAX_SIZE) {
        return EINVAL;
    }

    if ((*(buffer + (nr_bytes_rcv - 1)) == '\n') &&
        (*(buffer + (nr_bytes_rcv - 1)) == '\n')) {
        is_end_of_request = true;
    }

    error = aws_string_append_buffer(&http_request->request, buffer,
                                     nr_bytes_rcv);

    if (error) {
        return EINVAL;
    } else if (is_end_of_request) {
        return 0;
    } else {
        return EAGAIN;
    }
}

int
http_retrieve_requested_ressource_path(struct http_request *http_request,
                                       struct aws_string *extracted_path)
{
    int error = 0;
    char *path;

    printf("request %s\n", aws_string_get_buffer(&http_request->request));
    aws_string_extract_between(&http_request->request, " \t", " \t\n",
                               extracted_path);

    path = aws_string_get_buffer(extracted_path);

    if (path) {

        if (path[0] != '.') {
            error = aws_string_append_front(extracted_path, ".", 1);
        }

    } else {
        error = EINVAL;
    }

    return error;
}

void
http_request_destroy(struct http_request *http_request)
{
    assert(http_request);

    aws_string_destroy(&http_request->request);
}
