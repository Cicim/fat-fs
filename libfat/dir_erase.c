/**
 * Directory deletion
 * @author Cicim
 */
#include <string.h>
#include "internals.h"


/**
 * Recursively empty a directory and all its subdirectories
 * @author Cicim
 */
FatResult dir_empty(FatFs *fs, int dir_block) {
    FatResult res;

    // Loop over its entries
    DirEntry *entry;
    DirHandle dir;
    dir.block_number = dir_block;
    dir.count = 0;

    while (1) {
        res = dir_handle_next(fs, &dir, &entry);
        if (res == END_OF_DIR)
            break;
        else if (res != OK)
            return res;

        // If it's a directory, empty it
        if (entry->type == DIR_ENTRY_DIRECTORY) {
            res = dir_empty(fs, entry->first_block);
            if (res != OK)
                return res;
        }

        // Unlink the fat from the entry start
        res = fat_unlink(fs, entry->first_block);
        if (res != OK)
            return res;
    }

    return OK;
}


/**
 * Delete and empty a directory
 * @author Cicim
 */
FatResult dir_erase(FatFs *fs, const char *path) {
    FatResult res;

    // Convert the path to an absolute path
    char path_buffer[MAX_PATH_LENGTH];
    res = path_get_absolute(fs, path, path_buffer);
    if (res != OK)
        return res;

    // If the absolute path is the root
    if (path_buffer[0] == '/' && path_buffer[1] == '\0') {
        // Empty the root directory
        res = dir_empty(fs, ROOT_DIR_BLOCK);
        if (res != OK)
            return res;
        // Unlink it
        res = fat_unlink(fs, ROOT_DIR_BLOCK);
        if (res != OK)
            return res;
        
        // Re-add it to the bitmap
        bitmap_set(fs, ROOT_DIR_BLOCK, 1);
        // Add a directory end to the root directory
        memset(&fs->blocks_ptr[ROOT_DIR_BLOCK], 0, sizeof(DirEntry));

        return OK;
    }

    // Divide the path into its components
    char *dir_path, *name;
    res = path_get_components(fs, path, path_buffer, &dir_path, &name);
    if (res != OK)
        return res;

    // Open the directory
    int dir_block;
    res = dir_get_first_block(fs, dir_path, &dir_block);
    if (res != OK)
        return res;

    // Delete the child directory
    int child_block;
    res = dir_delete(fs, dir_block, DIR_ENTRY_DIRECTORY, name, &child_block);
    if (res != OK)
        return res;

    // Empty the child directory
    res = dir_empty(fs, child_block);
    if (res != OK)
        return res;

    // Unlink the fat from the entry start
    res = fat_unlink(fs, child_block);
    if (res != OK)
        return res;

    return OK;
}

