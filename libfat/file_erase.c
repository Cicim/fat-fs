/**
 * File erase function
 * @author Claziero
 */

#include <stdlib.h>
#include "internals.h"

/**
 * Erases the file from the given path
 * Returns an error if path is invalid
 * @author Claziero
 */
FatResult file_erase(FatFs *fs, const char *path) {
    FatResult res;

    // Split "path" in directory and file name
    char path_buffer[MAX_PATH_LENGTH];
    char *dir_path, *name;
    res = path_get_components(fs, path, path_buffer, &dir_path, &name);
    if (res != OK)
        return res;

    // Open the directory
    int dir_block;
    res = dir_get_first_block(fs, dir_path, &dir_block);
    if (res != OK)
        return res;

    // Delete the file
    int child_block;
    res = dir_delete(fs, dir_block, DIR_ENTRY_FILE, name, &child_block);
    if (res != OK)
        return res;

    // Unlink the file
    res = fat_unlink(fs, child_block);
    if (res != OK)
        return res;

    return OK;
}
