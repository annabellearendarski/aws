#ifndef FILE_H
#define FILE_H

#include <sys/types.h>

#include "list.h"

/*
 * File list descriptor.
 */
struct file_list {
    struct list entries;
};

/*
 * Entry descriptor.
 */
struct entry {
    unsigned int type;
    char *name;
    struct list node;
    //size
    //date
};

/*
 * Enum which describes possible file mode.
 */
typedef enum entry_type{
    ENTRY_DIR,
    ENTRY_FILE,
    ENTRY_UNK,
} entry_type;

/*
 * Initialize the list of files
 */
void file_list_init(struct file_list *list);

/*
 * Clean up list of files.
 */
void file_list_cleanup(struct file_list *list);

/*
 * Given a path to a directory, list all entries under it.
 */
errorCode file_list_retrieve_folder_entries(struct file_list *list, const char *dir_path);

/*
 * Get the mime type of a file.
 * Possible returned values are "application/pdf", "image/jpeg" , "".
 */
const char * file_retrieve_signature(FILE *file);

/*
 * Get the file mode (Dir, file, unknown).
 */
entry_type file_find_entry_type(char *entry_path);

/*
 * Get the size in bytes of a file.
 */
off_t file_retrieve_file_size(int file_fd);

#endif /* FILE_H */
