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
    DirHandle *dir;
    res = dir_open(fs, dir_path, &dir);
    if (res != OK)
        return res;

    // Delete the file
    int child_block;
    res = dir_delete(fs, dir->block_number, DIR_ENTRY_FILE, name, &child_block);
    if (res != OK)
        return res;

    // Update the FAT table and bitmap references
    do {
        // Set the bitmap
        bitmap_set(fs, child_block, 0);
        
        // Update the FAT table
        int next = fat_get_next_block(fs, child_block);
        fat_set_next_block(fs, child_block, FAT_EOF);

        child_block = next;
    } while (child_block != FAT_EOF);

    return OK;
}
