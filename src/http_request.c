#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "aws_string.h"
#include "http_request.h"

#define HTTP_REQUEST_MAX_SIZE 576

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

    if ((http_request->request.length + nr_bytes_rcv) > HTTP_REQUEST_MAX_SIZE) {
        return EINVAL;
    }

    if (strcmp((buffer + (nr_bytes_rcv - 4)), "\r\n")) {
        is_end_of_request = true;
    }

    http_request->request.length += nr_bytes_rcv;
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

    aws_string_extract_between(&http_request->request, " \t", " \t\n",
                               extracted_path);

    if (extracted_path->buffer) {

        if (extracted_path->buffer[0] != '.') {
            error =
                aws_string_append_front_buffer(extracted_path, ".", 1);
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
