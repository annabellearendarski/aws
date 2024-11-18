#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "entry.h"
#include "list.h"
#include "macro.h"

struct entry_file {
    char *file_extension;
    char *http_content_type;
};

static const struct entry_file entry_files[] = {
    {".pdf", "application/pdf"},
    {".jpg", "image/jpeg"},
};

entry_type
entry_find_type(char *entry_path)
{
    int error;
    struct stat path_stat;

    error = stat(entry_path, &path_stat);

    if (error) {
        return ENTRY_UNK;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        return ENTRY_DIR;
    } else if (S_ISREG(path_stat.st_mode)) {
        return ENTRY_FILE;
    } else {
        return ENTRY_UNK;
    }
}

static void
entry_set_type(struct entry *entry, char *entry_path)
{
    assert(entry);

    entry->type = entry_find_type(entry_path);
}

static void
entry_set_name(struct entry *entry, char *name)
{
    entry->name = malloc(strlen(name) + 1);
    strcpy(entry->name, name);
}

void
entry_list_init(struct entry_list *list)
{
    assert(list);
    list_init(&list->entries);
}

int
entry_list_retrieve_folder_entries(struct entry_list *list,
                                   const char *dir_path)
{
    assert(list);

    DIR *dir_stream;
    struct dirent *next_dir_entry;
    int i = 0;

    dir_stream = opendir(dir_path);

    if (dir_stream) {
        errno = 0;

        while (((next_dir_entry = readdir(dir_stream)) != NULL) && !errno) {
            struct entry *entry;

            entry = malloc(sizeof(*entry));

            if (!errno) {
                entry_set_name(entry, next_dir_entry->d_name);
                entry_set_type(entry, next_dir_entry->d_name);
                list_insert_head(&list->entries, &entry->node);

                i++;
                errno = 0;
            }
        }
        closedir(dir_stream);
    }

    return errno;
}

void
entry_list_cleanup(struct entry_list *list)
{
    while (!list_empty(&list->entries)) {
        struct entry *entry =
            list_first_entry(&list->entries, struct entry, node);
        free(entry->name);
        list_remove(&entry->node);
    }
}

const char *
entry_retrieve_content_type(const char *file_extension)
{
    if (!file_extension) {
        return "text/html";
    }

    for (size_t i = 0; i < ARRAY_SIZE(entry_files); i++) {
        if (strcmp(file_extension, entry_files[i].file_extension) == 0) {
            return entry_files[i].http_content_type;
        }
    }

    return "text/html";
}

off_t
file_retrieve_file_size(int file_fd)
{
    struct stat file_stat;
    fstat(file_fd, &file_stat);

    return file_stat.st_size;
}
