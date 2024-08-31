#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client.h"
#include "server.h"

#define CLIENT_MIN_SIZE_REQUESTED_PATH 4
#define CLIENT_MAX_DIR_ENTRIES 50
#define CLIENT_PDF_SIGNATURE 0x0000002D46445025
#define CLIENT_JPEG_SIGNATURE 0x000000FFE0FFD8FF

static int
client_list_folder_entries(const char *dir_path, char **directory_entries, int *nr_entries)
{
    DIR *dir_stream;
    struct dirent *next_dir_entry;
    int i = 0;

    dir_stream = opendir(dir_path);

    if (dir_stream) {
        errno = 0;

        while ((next_dir_entry = readdir(dir_stream)) != NULL) {
            directory_entries[i] = malloc(sizeof(char) * strlen(next_dir_entry->d_name) + 1);

            if (!directory_entries[i] || i > CLIENT_MAX_DIR_ENTRIES) {
                closedir(dir_stream);
                *nr_entries = 0;
                return 1;
            }

            strcpy(directory_entries[i], next_dir_entry->d_name);
            i++;
        }

        if (errno != 0) {
            *nr_entries = 0;
            perror("error :");
            return 1;
        }

    } else {
        *nr_entries = 0;
        perror("error :");
        return 1;
    }

    closedir(dir_stream);
    *nr_entries = i;
    return 0;
}

static char *
client_build_html_directory_stream_page(const char *directory_path, char **directory_entries, const int nr_directory_entries)
{
    int nr_char;
    size_t html_page_head_length = 80 + 2 * strlen(directory_path) + 1;
    char *html_page_head = malloc(html_page_head_length);

    if (!html_page_head) {
        return NULL;
    }

    nr_char = snprintf(html_page_head, html_page_head_length,
            "<html>"
            "<head><title>Index of %s </title></head> "
            "<body>"
            "<h1>Index of %s </h1>"
            "<hr><pre>",
            directory_path, directory_path);

    if (nr_char >= html_page_head_length || nr_char < 0) {
        free(html_page_head);
        return NULL;
    }

    size_t html_page_body_length = ((22 + 2 * 256) * nr_directory_entries);
    char *html_page_body = malloc(html_page_body_length + 1);

    if (!html_page_body) {
        free(html_page_head);
        return NULL;
    }

    nr_char = 0;
    for (int i = 0; i < nr_directory_entries; i++) {
        printf("entries %s\n", *(directory_entries + i));
        nr_char += snprintf(html_page_body + nr_char, html_page_body_length - nr_char,
                "<a href='%s'>%s/</a><br>" ,
                *(directory_entries + i), *(directory_entries + i));

        if (nr_char >= html_page_body_length || nr_char < 0) {
            free(html_page_head);
            free(html_page_body);
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
        return NULL;
    }

    strcpy(html_page, html_page_head);
    strcat(html_page, html_page_body);
    strcat(html_page, html_page_bottom);

    free(html_page_head);
    free(html_page_body);

    return html_page;
}

static const char*
client_build_http_response_error(size_t *response_length) //TODO , how it works ?
{
    const char *response = "HTTP/1.1 404 Not Found\r\n"
                    "Content-Type: text/plain\r\n"
                    "Content-Length: 13\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                    "404 Not Found";

    *response_length = strlen(response);

    return response;
}

static char *
client_build_http_response_for_folder_request(const char *path, size_t *response_length)
{
    int nr_entries_found;
    int error;
    int nr_char;
    char **entries;
    char *http_body;
    char *response;

    entries = malloc(CLIENT_MAX_DIR_ENTRIES * sizeof(char*));

    if (!entries) {
        return NULL;
    }

    error = client_list_folder_entries(path, entries, &nr_entries_found);

    if (!error) {
        http_body = client_build_html_directory_stream_page
                                            (path, entries, nr_entries_found);

        if (http_body) {
            size_t http_header_len = 63 + sizeof(size_t);
            *response_length = http_header_len + strlen(http_body);
            response = malloc(*response_length + 1);

            nr_char = snprintf(response, *response_length,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %lu\r\n"
                "\r\n"
                "%s",
                strlen(http_body), http_body);

            free(entries);
            free(http_body);

            if (nr_char >= response_length || nr_char < 0) {
                return NULL;
            }

            return response;
        } else {
            free(entries);
            response = client_build_http_response_error(response_length);
            return response;
        }
    } else {
        response = client_build_http_response_error(response_length);
        return response;
    }
}

/*
 * Retrieve file signature.
 * Extract the first 8 bytes of the file and
 * compare the extracted value to different signature with the help of a mask
 * to adjust different size of signatures.
 */
static const char*
client_retrieve_file_signature(int file_fd)
{
    FILE *file= fdopen(file_fd, "r");
    const char *mime_type;
    unsigned long file_signature;

    fread(&file_signature, 1, 8, file);
    fseek(file, 0, SEEK_SET);

    if (((file_signature & 0x000000FFFFFFFFFF) ^ CLIENT_PDF_SIGNATURE) == 0) {
        mime_type = "application/pdf";
    } else if (((file_signature | 0xFFFFFFFF00000000) ^ CLIENT_JPEG_SIGNATURE) == 0) {
        mime_type = "image/jpeg";
    } else {
        mime_type = "";
    }

    return mime_type;
}

static char *
client_build_http_response_for_file_request(const char *file_name,
                                            size_t *response_len)
{
    const char *mime_type;
    char *response;
    int nr_char;
    struct stat file_stat;

    int file_fd = open(file_name, O_RDONLY);

    if (file_fd == -1) {
        *response_len = 66;
        response = malloc(*response_len);

        if (!response) {
            return NULL;
        }

        snprintf(response, *response_len,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "404 Not Found");

        return response;
    }

    mime_type = client_retrieve_file_signature(file_fd);

    fstat(file_fd, &file_stat);
    off_t file_size = file_stat.st_size;

    *response_len = 54 + file_size;
    response = malloc(*response_len + 1);

    if (!response) {
        *response_len = 0;
        return NULL;
    }

    nr_char = snprintf(response, *response_len,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             mime_type, file_size);

    if (nr_char >= response_len || nr_char < 0) {
        return NULL;
    }

    ssize_t bytes_read;
    size_t offset = 0;

    *response_len = nr_char;
    while ((bytes_read = read(file_fd,
                            response + *response_len,
                            *response_len - offset)) > 0) {
        *response_len += bytes_read;
    }

    close(file_fd);
    return response;
}

static int
client_handle_request(struct client *client, const char *path)
{
    int error;
    struct stat path_stat;
    char *response;
    size_t response_length;

    error = stat(path, &path_stat);

    if (error == -1) {
        response = client_build_http_response_error(&response_length);
        send(client->fd, response, response_length, 0);
        return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        response = client_build_http_response_for_folder_request(path, &response_length);
        send(client->fd, response, response_length, 0); //TODO
    } else if ((S_ISREG(path_stat.st_mode))) {
        response = client_build_http_response_for_file_request(path, &response_length); //client should be first arg?

        if (!response) {
            return 1;
        }

        send(client->fd, response, response_length, 0); //TODO
        free(response);
    } else {
        printf("It is unknowed type");
    }

    return 0;
}

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
client_extract_requested_ressource_path(const char *request)
{
    char *end;
    int ressource_path_len;
    const char *start_address_path;
    char *ressource_path;

    if ((strncmp(request,"GET", 3) != 0) || (strlen(request) < CLIENT_MIN_SIZE_REQUESTED_PATH)) {
        return NULL;
    }

    start_address_path = request + 4;

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

static void *
client_run(void *arg)
{
    struct client *client = arg;
    char *path = NULL;

    for (;;) {
        char buffer[512];
        ssize_t nr_bytes_rcv;

        nr_bytes_rcv = recv(client->fd, buffer, sizeof(buffer), 0);

        if (nr_bytes_rcv == -1) {
            perror("error : ");
            break;
        } else {

            if (nr_bytes_rcv == 0) {
                printf("client%d: closed\n", client->fd);
                break;
            } else {
                path = client_extract_requested_ressource_path(buffer);

                if (path) {
                    printf("path %s \n", path);
                    client_handle_request(client, path);
                    free(path);
                    break;
                }
            }
        }
    }

    shutdown(client->fd, SHUT_RDWR);
    close(client->fd);
    server_remove_client(client->server, client);

    free(client);

    return NULL;
}

struct client *
client_create(struct server *server, int fd)
{
    struct client *client;
    int error;

    client = malloc(sizeof(*client));

    if (!client) {
        return NULL;
    }

    client->server = server;
    client->fd = fd;

    error = pthread_create(&client->pthread, NULL, client_run, client);

    if (error != 0) {
        free(client);
        return NULL;
    }

    pthread_detach(client->pthread);

    return client;
}

int
client_get_fd(const struct client *client)
{
    assert(client);

    return client->fd;
}
