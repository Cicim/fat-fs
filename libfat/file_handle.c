/**
 * File operations functions
 * @author Claziero
 */

#include <stdlib.h>
#include <string.h>
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
    (*file)->offset = sizeof(FileHeader); // Offset initially pointing to the actual data
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

    // Get the file open mode
    FileMode mod;
    if (strcmp(mode, "r") == 0)
        mod = FILE_MODE_READ;
    else if (strcmp(mode, "w") == 0)
        mod = FILE_MODE_WRITE;
    else if (strcmp(mode, "a") == 0)
        mod = FILE_MODE_APPEND;
    else if (strcmp(mode, "w+") == 0)
        mod = FILE_MODE_WRITE_CREATE;
    else if (strcmp(mode, "a+") == 0)
        mod = FILE_MODE_APPEND_CREATE;
    else
        return FILE_OPEN_INVALID_ARGUMENT;    

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
        
        // If the file does not exist and you don't want to create it, return an error
        if (res == END_OF_DIR && 
            (mod == FILE_MODE_READ || mod == FILE_MODE_WRITE || mod == FILE_MODE_APPEND))
            return FILE_NOT_FOUND;

        // Else if file does not exist and you want to create it, create it
        else if (res == END_OF_DIR) {
            res = file_create(fs, path);
            if (res != OK)
                return res;
        }
            
        // Else there's another error
        else if (res != OK)
            return res;

        if (strcmp(entry->name, name) == 0) {
            // Save the file block number
            file_block = entry->first_block;
            break;
        }            
    }
    
    // Open the file
    res = file_open_by_block(fs, file_block, file);
    if (res != OK)
        return res;

    // Set the file mode
    (*file)->mode = mod;

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
