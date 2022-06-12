/**
 * Functions for opening and closing a directory
 * @author Cicim
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "internals.h"



/**
 * Opens a directory given the path
 * @author Cicim
 */
FatResult dir_open(FatFs *fs, const char *path, DirHandle **dir) {
    FatResult res;
    
    // Convert the path to absolute
    char abs_path[MAX_PATH_LENGTH];
    res = path_get_absolute(fs, path, abs_path);
    if (res != OK)
        return res;

    // Get the directory block given the path
    int block;
    res = dir_get_first_block(fs, abs_path, &block);
    if (res != OK)
        return res;

    // Allocate memory for the directory handle
    *dir = malloc(sizeof(DirHandle));
    if (*dir == NULL)
        return OUT_OF_MEMORY;

    // Initialize the directory handle
    (*dir)->fs = fs;
    (*dir)->block_number = block;
    (*dir)->count = 0;

    return OK;
}


/**
 * Closes a directory
 * @author Cicim
 */
FatResult dir_close(DirHandle *dir) {
    free(dir);
    return OK;
}

/**
 * Compare "name" with the first part of "path"
 * @example compare_name("foo/bar", "foo") -> 1
 * @example compare_name("foo/bar", "bar") -> 0
 * @author Cicim
 * @return 1 if the name is the same, 0 otherwise
 */
int compare_name(const char *path, const char *name) {
    for (int i = 0; i < MAX_FILENAME_LENGTH; i++) {
        if ((path[i] == '\0' || path[i] == '/') && name[i] == '\0')
            return 1;
        if (path[i] != name[i])
            break;
    }
    return 0;
}


/**
 * Returns the first block of the directory given the absolute path
 * @author Cicim
 */
FatResult dir_get_first_block(FatFs *fs, const char *path, int *block_number) {
    int block = block_number ? *block_number : ROOT_DIR_BLOCK;

    // If the path is absolute, start from the root directory
    if (*path == '/') {
        path++;
        block = ROOT_DIR_BLOCK;
    }
    // If the path was the root itself, return the root directory's block
    if (*path == '\0') {
        if (block_number != NULL)
            *block_number = ROOT_DIR_BLOCK;
        return OK;
    }

    // Get the name of the directory to look for
    FatResult res;
    DirHandle dir;

    while (path) {
        DirEntry *entry;
        dir.block_number = block;
        dir.count = 0;

        // Get the entry with the given name
        while (1) {
            res = dir_handle_next(fs, &dir, &entry);

            // Stop if there are no more entries
            if (res == END_OF_DIR)
                return FILE_NOT_FOUND;
            // Stop if there is some other error
            else if (res != OK)
                return res;

            // Continue if the name doesn't match
            if (!compare_name(path, entry->name))
                continue;

            // Stop if the entry is not a directory
            if (entry->type != DIR_ENTRY_DIRECTORY)
                return NOT_A_DIRECTORY;

            // Return the block number
            block = entry->first_block;
            // Advance the path
            path = strchr(path, '/');
            if (path) path++;
            // Stop the inner loop
            break;
        }
    }

    if (block_number != NULL)
        *block_number = block;

    return OK;
}
