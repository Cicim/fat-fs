/**
 * File operations functions
 * @author Claziero
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "internals.h"

/**
 * Returns a file descriptor (struct FileHandle) given a block number 
 * @author Claziero
 */ 
FatResult file_open_by_block(FatFs *fs, int block_number, FileHandle **file) {
    // Check if the block number is valid
    if (block_number < 0 || block_number >= fs->header->blocks_count)
        return INVALID_BLOCK;

    // Allocate memory for the file handle
    *file = malloc(sizeof(FileHandle));
    if (*file == NULL)
        return OUT_OF_MEMORY;
    
    // Initialize the file handle
    (*file)->fs = fs;
    (*file)->initial_block_number = block_number;
    (*file)->current_block_number = block_number;
    (*file)->block_offset = sizeof(FileHeader); // Offset initially pointing to the actual data
    (*file)->file_offset = 0;
    (*file)->fh = (FileHeader *) (fs->blocks_ptr + block_number * fs->header->block_size);

    return OK;
}

/**
 * Creates a file handle given a path
 * Returns an error if "path" is invalid
 * @author Claziero
 */
FatResult file_open(FatFs *fs, const char *path, FileHandle **file, char *mode) {
    FatResult res;

    // Check if mode is valid
    if (mode == NULL || !*mode)
        return FILE_OPEN_INVALID_ARGUMENT;

    // Get the file open mode
    int can_read = 0, can_write = 0, create = 0, append = 0;
    do {
        if (*mode == 'r')
            can_read = 1;
        else if (*mode == 'w')
            can_write = 1;
        else if (*mode == 'a')
            append = can_write = 1;
        else if (*mode == '+')
            create = 1;
        else
            return FILE_OPEN_INVALID_ARGUMENT;
    } while (*++mode);

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

    // Create the Directory Handle
    DirHandle dir;
    dir.block_number = dir_block;
    dir.count = 0;

    // Look for the file in the directory
    DirEntry *entry;
    int file_block;
    while (1) {
        res = dir_handle_next(fs, &dir, &entry);
        
        // If the file does not exist
        if (res == END_OF_DIR) {
            // If you don't want to create it, return an error
            if (!create)
                return FILE_NOT_FOUND;

            // Else create it
            res = file_create(fs, path);
            if (res != OK)
                return res;
            break;
        }

        // Else there's another error
        else if (res != OK)
            return res;

        if (strcmp(entry->name, name) == 0) {
            if (entry->type != DIR_ENTRY_FILE)
                return NOT_A_FILE;
            break;
        }            
    }

    // Save the file block number
    file_block = entry->first_block;
    
    // Open the file
    res = file_open_by_block(fs, file_block, file);
    if (res != OK)
        return res;

    // Set the file mode
    (*file)->can_read = can_read;
    (*file)->can_write = can_write;

    // If the file is opened in append mode, seek to the end of the file
    if (append)
        return file_seek(*file, 0, FILE_SEEK_END);   

    return OK;
}

/**
 * Frees the memory occupied by a file handle
 * @author Claziero
 */
FatResult file_close(FileHandle *file) {
    free(file);
    return OK;
}

/** 
 * Prints the file contents to stdout
 * @author Claziero
 */
FatResult file_print(FileHandle *file, char *buffer) {
    for (int i = 0; i < file->fh->size; i++)
        printf("%c", buffer[i]);

    printf("\n");
    return OK;
}
