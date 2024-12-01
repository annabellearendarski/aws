#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aws_buffer.h"
#include "aws_string.h"
#include "entry.h"
#include "http_response.h"
#include "list.h"


void
http_response_init(struct http_response *http_response)
{
    aws_string_init_empty(&http_response->header);
    aws_buffer_init_empty(&http_response->body);
}

static int
http_build_html_body_for_folder_request(
    struct http_response *response, struct entry_list *list, const char *requested_path)
{
    assert(&response->body);
    int error = 0;

    error =
        aws_buffer_append_format(&response->body,
                                    "<html>"
                                    "<head><title>Index of %s </title></head> "
                                    "<body>"
                                    "<h1>Index of %s </h1>"
                                    "<hr><pre>",
                                    requested_path,
                                    requested_path);

    if (!error) {
        struct entry *entry;

        list_for_each_entry(&list->entries, entry, node)
        {

            if (entry->type == ENTRY_DIR) {
                error = aws_buffer_append_format(
                    &response->body,
                    "<a href='%s/'>%s/</a><br>", entry->name, entry->name);
            } else {
                error = aws_buffer_append_format(
                    &response->body,
                    "<a href='%s'>%s</a><br>", entry->name, entry->name);
            }

            if (error) {
                break;
            }
        }
    }

    if (!error) {
        error = aws_buffer_append_format(&response->body,
                                         "</pre><hr></body></html>");
    }

    return error;
}

int
http_response_add_response_error(struct http_response *response)
{
    assert(&response->header);
    assert(&response->body);

    int error = 0;

    error = aws_string_append_format(&response->header,
                                     "HTTP/1.1 404 Not Found\r\n"
                                     "Content-Type: text/plain\r\n"
                                     "Content-Length: 13\r\n"
                                     "Connection: close\r\n"
                                     "\r\n");

    error = aws_buffer_append_format(&response->body,
                                     "404 Not Found");

    return error;
}


int
http_response_add_response_for_folder_request(
    struct http_response *response, struct entry_list *list, const char *requested_path)
{
    int error = 0;

    error = http_build_html_body_for_folder_request(response, list, requested_path);

    if (!error) {
        error =
            aws_string_append_format(&response->header,
                                     "HTTP/1.1 200 OK\r\n"
                                     "Content-Type: text/html\r\n"
                                     "Content-Length: %lu\r\n"
                                     "\r\n",
                                     response->body.length);
    } else {
        error = http_response_add_response_error(response);
    }

    return error;
}

int
http_response_add_response_for_file_request(struct http_response *response,
                                            const char *mime_type,
                                            off_t file_size,
                                            const int file_fd)
{
    int error = 0;

    error = aws_string_append_format(&response->header,
                                        "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: %s\r\n"
                                        "Content-Length: %ld\r\n"
                                        "\r\n",
                                        mime_type, file_size);
    if (!error) {
        ssize_t bytes_read;
        size_t offset = 0;
        char *temp_resp = malloc(file_size + 1);

        while ((bytes_read = read(file_fd, temp_resp + offset,
                                    file_size - offset)) > 0) {
            offset += bytes_read;
        }

        error = aws_buffer_append_format(&response->body,
                                            temp_resp, file_size);
        free(temp_resp);
    }

    if (error) {
        error = http_response_add_response_error(response);
    }

    return error;
}

void
http_response_destroy(struct http_response *http_response)
{
    assert(http_response);

    aws_string_destroy(&http_response->header);
    aws_buffer_destroy(&http_response->body);

    free(http_response);
}
