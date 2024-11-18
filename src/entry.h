#ifndef FILE_H
#define FILE_H

#include <sys/types.h>

#include "list.h"

/*
 * Entry list descriptor.
 */
struct entry_list {
    struct list entries;
};

/*
 * Entry descriptor.
 */
struct entry {
    unsigned int type;
    char *name;
    struct list node;
};

/*
 * Enum which describes possible entry type.
 */
typedef enum entry_type{
    ENTRY_DIR,
    ENTRY_FILE,
    ENTRY_UNK,
} entry_type;

/*
 * Initialize the list of files
 */
void entry_list_init(struct entry_list *list);

/*
 * Clean up list of entries.
 */
void entry_list_cleanup(struct entry_list *list);

/*
 * Given a path to a directory, list all entries under it.
 */
int entry_list_retrieve_folder_entries(struct entry_list *list, const char *dir_path);

/*
 * find file type (Dir, file, unknown).
 */
entry_type entry_find_type(char *entry_path);

/*
 * Get the size in bytes of a file.
 */
off_t file_retrieve_file_size(int file_fd);

const char * entry_retrieve_content_type(const char *file_extension);

#endif /* FILE_H */
