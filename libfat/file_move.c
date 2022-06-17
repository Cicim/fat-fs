/**
 * Move or copy files or directories.
 * @author Cicim
 */
#include <stdio.h>
#include <string.h>
#include "internals.h"


/**
 * Move a file or directory.
 * @author Cicim
 */
FatResult file_move(FatFs *fs, const char *source_path, const char *dest_path) {
    FatResult res;

    if (strcmp(source_path, dest_path) == 0)
        return SAME_PATH;

    /**
     * Part 1
     * Discover everything about the source file
     */
    DirEntryType src_type;
    int src_block;

    // Get the source path components
    char source_path_buffer[MAX_PATH_LENGTH];
    char *source_dir, *source_name;
    res = path_get_components(fs, source_path, source_path_buffer, &source_dir, &source_name);
    // path_get_components will not allow you to move the root directory
    if (res != OK)
        return res;


    int src_dir_block;
    // Get the block number of the source directory
    res = dir_get_first_block(fs, source_dir, &src_dir_block);
    if (res != OK)
        return res;

    // Get the type of the source file
    DirHandle dir;
    DirEntry *entry;
    // Get the block number of the source file
    res = dir_get_entry(fs, src_dir_block, source_name, &entry, &dir);
    if (res != OK)
        return res;

    src_type = entry->type;
    src_block = entry->first_block;

    /**
     * Part 2
     * Discover everything about the destination directory block
     */
    int destination_block;
    char *destination_name = source_name;

    // If the destination is the root
    if (strcmp(dest_path, "/") == 0)
        destination_block = ROOT_DIR_BLOCK;
    else {
        // Get the destination path components
        char dest_path_buffer[MAX_PATH_LENGTH];
        char *dest_dir, *dest_name;
        res = path_get_components(fs, dest_path, dest_path_buffer, &dest_dir, &dest_name);
        if (res != OK)
            return res;

        // Get the block number of the destination directory
        int dest_dir_block;
        res = dir_get_first_block(fs, dest_dir, &dest_dir_block);
        if (res != OK)
            return res;

        // Check if the destination name exists
        DirHandle dest_dir_handle;
        DirEntry *dest_entry;

        res = dir_get_entry(fs, dest_dir_block, dest_name, &dest_entry, &dest_dir_handle);
        if (res == OK) {
            // If it exists, and it is a file, return an error
            if (dest_entry->type == DIR_ENTRY_FILE)
                return FILE_ALREADY_EXISTS;
            // If it exists, and it is a directory, move to it
            else
                destination_block = dest_entry->first_block;
        }
        // If the entry does not exist, move it to the destination directory
        else if (res == FILE_NOT_FOUND) {
            destination_block = dest_dir_block;
            destination_name = dest_name;
        }
        else
            return res;
    }


    // Create an entry in the root
    DirEntry *new_entry;
    // Write the entry to the root
    res = dir_insert(fs, destination_block, &new_entry, src_block, src_type, destination_name);
    if (res != OK)
        return res;
        
    
    // Delete the entry from the source directory
    return dir_delete(fs, src_dir_block, -1, source_name, NULL);
}
