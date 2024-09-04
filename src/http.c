#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "file.h"
#include "http.h"
#include "list.h"

#define CLIENT_MIN_SIZE_REQUESTED_PATH 4

/*
 * Extract ressource path from GET http request.
 * Ressource path is starting at index 4 :
 * GET /x
 * 012345
 * Ressource path is returned by adding "." as first caracter
 * which result to the following returned path : ./x
 * The minimal ressource path is the one which is pointing to the current folder
 * which will result to GET / .Thus request needs to be at least 5 bytes when GET
 * is detected in the request.
 */
static char *
http_retrieve_requested_ressource_path(struct http_transaction *http_transaction)
{
    char *end;
    int ressource_path_len;
    char *start_address_path;
    char *ressource_path;

    if ((strncmp(http_transaction->request,"GET", 3) != 0) || (strlen(http_transaction->request) < CLIENT_MIN_SIZE_REQUESTED_PATH)) {
        return NULL;
    }

    start_address_path = http_transaction->request + 4;

    end = start_address_path;

    while( *end != ' ') {
        end++;
    }

    ressource_path_len = (end - start_address_path) + 1;
    ressource_path = malloc(ressource_path_len + 1 + 1);

    if (!ressource_path) {
        return NULL;
    }

    ressource_path[0] = '.';
    snprintf(ressource_path + 1, ressource_path_len, "%s\n", start_address_path);

    return ressource_path;

}

static char *
http_build_html_body_for_folder_request(struct http_transaction *http_transaction)
{
    int nr_char;
    int nr_entries_found;

    size_t html_page_head_length = 80 + 2 * strlen(http_transaction->requested_path) + 1;
    char *html_page_head = malloc(html_page_head_length);

    if (!html_page_head) {
        return NULL;
    }

    struct list *entries = file_list_folder_entries(http_transaction->requested_path, &nr_entries_found);

    if(!entries) {
        return NULL;
    }

    nr_char = snprintf(html_page_head, html_page_head_length,
            "<html>"
            "<head><title>Index of %s </title></head> "
            "<body>"
            "<h1>Index of %s </h1>"
            "<hr><pre>",
            http_transaction->requested_path, http_transaction->requested_path);

    if (nr_char >= (int)html_page_head_length || nr_char < 0) {
        free(entries);
        free(html_page_head);
        return NULL;
    }

    size_t html_page_body_length = ((23 + 2 * 256) * nr_entries_found);
    char *html_page_body = malloc(html_page_body_length + 1);

    if (!html_page_body) {
        free(entries);
        free(html_page_head);
        return NULL;
    }


    nr_char = 0;
    struct entry *entry;

    list_for_each_entry(entries, entry, node) {
        if (entry->type == ENTRY_DIR) {
            nr_char += snprintf(html_page_body + nr_char, html_page_body_length - nr_char,
                "<a href='%s/'>%s/</a><br>" ,
                entry->name, entry->name);
        } else {
           nr_char += snprintf(html_page_body + nr_char, html_page_body_length - nr_char,
                "<a href='%s'>%s</a><br>" ,
                entry->name, entry->name);
        }

        if (nr_char >= (int)html_page_body_length || nr_char < 0) {
            free(html_page_head);
            free(html_page_body);
            free(entries);
            return NULL;
        }
    }

    const char *html_page_bottom = "</pre><hr></body></html>";
    size_t html_page_bottom_length = strlen(html_page_body);

    size_t html_page_length = html_page_head_length + html_page_body_length + html_page_bottom_length;
    char *html_page = malloc(html_page_length + 1);

    if(!html_page) {
        free(html_page_head);
        free(html_page_body);
        free(entries);
        return NULL;
    }

    strcpy(html_page, html_page_head);
    strcat(html_page, html_page_body);
    strcat(html_page, html_page_bottom);


    free(html_page_head);
    free(html_page_body);
    free(entries);

    return html_page;
}

static void
http_build_response_error(struct http_transaction *http_transaction)
{
    http_transaction->response = "HTTP/1.1 404 Not Found\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: 13\r\n"
                                "Connection: close\r\n"
                                "\r\n"
                                "404 Not Found";

    http_transaction->response_len = strlen(http_transaction->response);

}

static void
http_build_response_for_folder_request(struct http_transaction *http_transaction)
{
    int nr_char;
    char *http_body;

    http_body = http_build_html_body_for_folder_request(http_transaction);

    if (http_body) {
        size_t http_header_len = 63 + sizeof(size_t);
        http_transaction->response_len = http_header_len + strlen(http_body);
        http_transaction->response = malloc(http_transaction->response_len + 1);

        if(!http_transaction->response) {
            http_build_response_error(http_transaction);
        } else {
            nr_char = snprintf(http_transaction->response, http_transaction->response_len,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %lu\r\n"
            "\r\n"
            "%s",
            strlen(http_body), http_body);


            if (nr_char >= (int)http_transaction->response_len || nr_char < 0) {
                http_build_response_error(http_transaction);
            }
        }

        free(http_body);
    } else {
        http_build_response_error(http_transaction);
    }
}


static void
http_build_response_for_file_request(struct http_transaction *http_transaction)
{
    const char *mime_type;
    int nr_char;

    int file_fd = open(http_transaction->requested_path, O_RDONLY);

    if (file_fd == -1) {
        http_build_response_error(http_transaction);
    } else {
        mime_type = file_retrieve_signature(file_fd);
        __off_t file_size = file_retrieve_file_size(file_fd);

        int content_lenght_nr_digit = (file_size == 0)? 1 : log10(file_size) + 1;
        int http_header_len = 53;

        http_transaction->response_len = http_header_len + strlen(mime_type) + content_lenght_nr_digit + file_size;
        http_transaction->response = malloc(http_transaction->response_len + 1);

        if (!http_transaction->response) {
            http_build_response_error(http_transaction);
        } else {
            nr_char = snprintf(http_transaction->response, http_transaction->response_len,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %ld\r\n"
                "\r\n",
                mime_type, file_size);

            if (nr_char >= (int)http_transaction->response_len || nr_char < 0) {
                http_build_response_error(http_transaction);
            } else {
                ssize_t bytes_read;
                size_t offset = nr_char;

                while ((bytes_read = read(file_fd,
                                        http_transaction->response + offset,
                                        http_transaction->response_len - offset)) > 0) {
                    offset += bytes_read;
                }
            }
        }
        close(file_fd);
    }
}

static void
http_build_response(struct http_transaction *http_transaction)
{
    unsigned int entry_type;
    entry_type = file_find_entry_type(http_transaction->requested_path);
    printf("entry type %d\n", entry_type);
    printf("path %s\n",http_transaction->requested_path);

    if (entry_type == ENTRY_DIR) {
        printf("it is a dir\n");
        http_build_response_for_folder_request(http_transaction);
    } else if (entry_type == ENTRY_FILE) {
        printf("It is file\n");
        http_build_response_for_file_request(http_transaction);
    } else {
        printf("It is not known");
        http_transaction->response_len = 0;
        http_transaction->response = NULL;
    }

}

struct http_transaction *
http_response_create(char *request)
{
    struct http_transaction *http_transaction;
    char *requested_path;

    http_transaction = malloc(sizeof(*http_transaction));

    if(!http_transaction) {
        return NULL;
    }

    http_transaction->request = request;

    requested_path = http_retrieve_requested_ressource_path(http_transaction);

    if (!requested_path) {
        free(http_transaction);
        return NULL;
    }

    http_transaction->requested_path = requested_path;
    http_build_response(http_transaction);

    return http_transaction;

}

