/**
 * Functions for Accessing the FAT and Bitmap
 * @author Claziero
 */

#include "internals.h"
#include <string.h>

// Returns the bit value at the given block index
int bitmap_get(FatFs *fs, int block_number) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    return (fs->bitmap_ptr[byte_index] >> bit_index) & 1;
}

// Sets the bit value at the given block index
void bitmap_set(FatFs *fs, int block_number, int value) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    // Set the bit
    if (value)
        fs->bitmap_ptr[byte_index] |= (1 << bit_index);
    else
        fs->bitmap_ptr[byte_index] &= ~(1 << bit_index);
}

// Returns the first free block in the bitmap
int bitmap_get_free_block(FatFs *fs) {
    int bitmap_size = fs->header->blocks_count / 8;

    // Loop through the bitmap
    for (int i = 0; i < bitmap_size; i++) {
        // For every byte in the bitmap check if its value is not 0xFF
        if (fs->bitmap_ptr[i] != 0xFF) {
            // Loop through the bits in the byte
            for (int j = 0; j < 8; j++) {
                // If the bit is 0, return the block index
                if (!((fs->bitmap_ptr[i] >> j) & 1))
                    return i * 8 + j;
            }
        }
    }

    // If no free block was found, return -1
    return -1;
}

// Stores the absolute path of the given file/directory
// TODO: implement cases when "./" or "../" are inside the path, not only at the beginning
FatResult path_get_absolute(FatFs *fs, const char *path, char *dest) {
    // If the path is empty, return an error
    if (path == NULL || path[0] == '\0')
        return INVALID_PATH;

    // If the path is wrong, return an error
    if (strncmp(path, "..", 2) == 0 && path[2] != '\0' && path[2] != '/')
        return INVALID_PATH;

    // If the path is already absolute, copy it to the destination
    if (strncmp(path, fs->current_directory, strlen(fs->current_directory)) == 0) {
        strcpy(dest, path);
        return OK;
    }

    // If the path contains only ".."
    if (strcmp(path, "..") == 0) {
        // Get the parent directory
        char *parent_dir = strrchr(fs->current_directory, '/');
        // If the parent directory is the root directory, return an error
        if (parent_dir == fs->current_directory)
            return INVALID_PATH;
        // Otherwise, copy the parent directory to the destination
        strncpy(dest, fs->current_directory, strlen(fs->current_directory) - strlen(parent_dir));
        dest[strlen(fs->current_directory) - strlen(parent_dir)] = '\0';

        return OK;
    }

    // If the path begins with "../"
    int flag = 0;
    while (strncmp(path, "../", 3) == 0) {
        flag = 1;

        // Get the parent directory
        char *parent_dir = strrchr(fs->current_directory, '/');
        // If the parent directory is the root directory, return an error
        if (parent_dir == fs->current_directory)
            return INVALID_PATH;
        // Otherwise, copy the parent directory to the destination
        strncpy(dest, fs->current_directory, strlen(fs->current_directory) - strlen(parent_dir));
        dest[strlen(fs->current_directory) - strlen(parent_dir)] = '\0';
        
        // Get the next path component
        path += 3;
    }
    // If the path is terminated
    if (flag && strcmp(path, "") == 0) 
        return OK;
    else if (flag) {
        // Append the rest of the path to the destination
        strcat(dest, "/");
        strcat(dest, path);
        return OK;
    }

    // If the path contains only "."
    if (strcmp(path, ".") == 0) {
        strcpy(dest, fs->current_directory);
        return OK;
    }
    
    // If the path begins with "./"
    flag = 0;
    while (strncmp(path, "./", 2) == 0) {
        flag = 1;

        // Copy the current directory to the destination
        strcpy(dest, fs->current_directory);
        
        // Get the next path component
        path += 2;
    }
    // If the path is terminated
    if (flag && strcmp(path, "") == 0) 
        return OK;
    else if (flag) {
        // Append the rest of the path to the destination
        strcat(dest, "/");
        strcat(dest, path);
        return OK;
    }

    // If the path begins with "/", then copy the given path to the destination
    if (strncmp(path, "/", 1) == 0) {
        strcpy(dest, path);
        return OK;
    }
    
    // Else the path is relative, so concatenate the current directory and the path
    strcpy(dest, fs->current_directory);
    strcat(dest, "/");
    strcat(dest, path);

    return OK;
}
