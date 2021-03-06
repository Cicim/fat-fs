/**
 * Functions for Accessing the FAT and Bitmap
 * @authors Claziero, Cicim
 */

#include <string.h>
#include "internals.h"

/**
 * Returns the bit value at the given block index
 * @author Claziero
 */
int bitmap_get(FatFs *fs, int block_number) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    return (fs->bitmap_ptr[byte_index] >> bit_index) & 1;
}

/**
 * Sets the bit value at the given block index
 * @author Claziero
 */
void bitmap_set(FatFs *fs, int block_number, int value) {
    // Get the bitmap sector (byte) and the bit index
    int byte_index = block_number / 8;
    int bit_index = block_number % 8;

    char old_value = (fs->bitmap_ptr[byte_index] >> bit_index) & 1;
    fs->header->free_blocks -= (value - old_value);

    // Set the bit
    if (value)
        fs->bitmap_ptr[byte_index] |= (1 << bit_index);
    else
        fs->bitmap_ptr[byte_index] &= ~(1 << bit_index);
}

/**
 * Returns the first free block in the bitmap
 * @author Claziero
 */
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

/**
 * Stores the absolute path of the given file/directory
 * TODO: implement cases when "./" or "../" are inside the path, not only at the beginning
 * @author Claziero
 */
FatResult path_get_absolute(FatFs *fs, const char *path, char *dest) {
    char result[MAX_PATH_LENGTH];

    // If the path is empty, return an error
    if (path == NULL || path[0] == '\0')
        return INVALID_PATH;

    // If the path is wrong, return an error
    if (strncmp(path, "..", 2) == 0 && path[2] != '\0' && path[2] != '/')
        return INVALID_PATH;

    // If the path is already absolute, copy it to the destination
    if (strncmp(path, fs->current_directory, strlen(fs->current_directory)) == 0) {
        if (strcmp(path, "/") == 0)
            strcpy(result, "/");
        else {
            strcpy(result, path);

            // Check if there are trailing slashes
            while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
                result[strlen(result) - 1] = '\0';
        }

        strcpy(dest, result);
        return OK;
    }

    // If the path contains only ".."
    if (strcmp(path, "..") == 0) {
        // Get the parent directory
        char *parent_dir = strrchr(fs->current_directory, '/');

        // If the current directory is the root directory, return an error
        if (strcmp(fs->current_directory, "/") == 0)
            return INVALID_PATH;

        // If the parent directory is the root directory
        if (parent_dir == fs->current_directory) {
            strcpy(result, "/");
            strcpy(dest, result);
            return OK;
        }

        // Otherwise, copy the parent directory to the destination
        strncpy(result, fs->current_directory, strlen(fs->current_directory) - strlen(parent_dir));
        result[strlen(fs->current_directory) - strlen(parent_dir)] = '\0';

        strcpy(dest, result);
        return OK;
    }

    // If the path begins with "../"
    int flag = 0;
    strcpy(result, fs->current_directory);
    while (strncmp(path, "../", 3) == 0) {
        flag = 1;
        // Get the parent directory
        char *parent_dir = strrchr(result, '/');

        // If the current directory is the root directory, return an error
        if (strcmp(result, "/") == 0)
            return INVALID_PATH;

        // If the parent directory is the root directory
        if (parent_dir == result) {
            strcpy(result, "/");
            path += 3;
            continue;
        }

        // Otherwise, copy the parent directory to the destination
        strncpy(result, fs->current_directory, strlen(fs->current_directory) - strlen(parent_dir));
        result[strlen(fs->current_directory) - strlen(parent_dir)] = '\0';
        
        // Get the next path component
        path += 3;
    }
    // If the path is terminated
    if (flag && strcmp(path, "") == 0) {
        strcpy(dest, result);
        return OK;
    }
    else if (flag) {
        // Append the rest of the path to the destination
        if (result[1] != '\0')
            strcat(result, "/");
        strcat(result, path);

        // Check if there are trailing slashes
        while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
            result[strlen(result) - 1] = '\0';
        
        strcpy(dest, result);
        return OK;
    }

    // If the path contains only "."
    if (strcmp(path, ".") == 0) {
        strcpy(result, fs->current_directory);

        // Check if there are trailing slashes
        while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
            result[strlen(result) - 1] = '\0';

        
        strcpy(dest, result);
        return OK;
    }
    
    // If the path begins with "./"
    flag = 0;
    while (strncmp(path, "./", 2) == 0) {
        flag = 1;

        // Copy the current directory to the destination
        strcpy(result, fs->current_directory);
        
        // Get the next path component
        path += 2;
    }
    // If the path is terminated
    if (flag && strcmp(path, "") == 0) {
        strcpy(dest, result);
        return OK;
    }
    else if (flag) {
        // Append the rest of the path to the destination
        if (result[1] != '\0')
            strcat(result, "/");
        strcat(result, path);

        // Check if there are trailing slashes
        while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
            result[strlen(result) - 1] = '\0';
        
        strcpy(dest, result);
        return OK;
    }

    // If the path begins with "/", then copy the given path to the destination
    if (strncmp(path, "/", 1) == 0) {
        strcpy(result, path);

        // Check if there are trailing slashes
        while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
            result[strlen(result) - 1] = '\0';

        strcpy(dest, result);
        return OK;
    }
    
    // Else the path is relative, so concatenate the current directory and the path
    strcpy(result, fs->current_directory);
    if (strcmp(fs->current_directory, "/") != 0) 
        strcat(result, "/");
    strcat(result, path);

    // Check if there are trailing slashes
    while (result[strlen(result) - 1] == '/' && strlen(result) != 1)
        result[strlen(result) - 1] = '\0';

    strcpy(dest, result);
    return OK;
}

/**
 * Convert a path to absolute, then store the pointers to the
 * directory and the element
 * @author Cicim
 */
FatResult path_get_components(FatFs *fs, const char *path, char *path_buffer, char **dir_ptr, char **element_ptr) {
    // Get the absolute path
    FatResult res = path_get_absolute(fs, path, path_buffer);
    if (res != OK)
        return res;

    // If the path is the root, return an error
    if (path_buffer[0] == '/' && path_buffer[1] == '\0')
        return INVALID_PATH;

    // Get the last name in the path to create
    *element_ptr = strrchr(path_buffer, '/');
    // Split the two strings
    **element_ptr = '\0';
    // Skip the '\0'
    *element_ptr += 1;

    // Get the directory
    if (path_buffer[0] == '\0')
        *dir_ptr = "/";
    else
        *dir_ptr = path_buffer;

    return OK;
}

static const char *fat_result_str_table[] = {
    [OK]                          = "Ok",
    [-INVALID_BLOCKS_COUNT]       = "Invalid number of blocks",
    [-INVALID_BLOCK_SIZE]         = "Invalid block size",
    [-FAT_OPEN_ERROR]             = "Cannot open the FAT buffer file",
    [-FAT_BUFFER_ERROR]           = "Error with the buffer containing the FAT FS",
    [-FAT_CLOSE_ERROR]            = "Cannot close the FAT buffer file",
    [-INVALID_PATH]               = "Invalid path",
    [-DIR_END_NOT_FOUND]          = "Cannot find DIR_END entry in the last block of the directory",
    [-END_OF_DIR]                 = "Directory end",
    [-FILE_NOT_FOUND]             = "No such file or directory",
    [-NOT_A_DIRECTORY]            = "Not a directory",
    [-NO_FREE_BLOCKS]             = "Not enough free blocks",
    [-FILE_ALREADY_EXISTS]        = "File already exists",
    [-OUT_OF_MEMORY]              = "Out of memory",
    [-INVALID_BLOCK]              = "Invalid block",
    [-SEEK_INVALID_ARGUMENT]      = "Invalid argument for seek",
    [-NOT_A_FILE]                 = "Not a file",
    [-WRITE_INVALID_ARGUMENT]     = "Invalid argument for write",
    [-FILE_OPEN_INVALID_ARGUMENT] = "Invalid argument for file open",
    [-LS_INVALID_ARGUMENT]        = "Invalid argument for ls",
    [-SAME_PATH]                  = "Same paths",
};

/**
 * Returns a string with the description of the given error
 * @author Cicim
 */
const char *fat_result_string(FatResult res) {
    return fat_result_str_table[-res];
}

/** 
 * Unlink all blocks associated with a file and free them
 * @authors Cicim, Claziero
 */
FatResult fat_unlink(FatFs *fs, int block_number) {
    // Update the FAT table and bitmap references
    do {
        // Set the bitmap
        bitmap_set(fs, block_number, 0);
        
        // Update the FAT table
        int next = fat_get_next_block(fs, block_number);
        fat_set_next_block(fs, block_number, FAT_EOF);

        block_number = next;
    } while (block_number != FAT_EOF);

    return OK;
}

/**
 * Get a DirEntry given a name
 * @author Cicim
 */
FatResult dir_get_entry(FatFs *fs, int dir_block, const char *name, DirEntry **entry, DirHandle *dir) {
    FatResult res;
    
    DirEntry *curr;
    dir->block_number = dir_block;
    dir->count = 0;

    while (1) {
        res = dir_handle_next(fs, dir, &curr);

        // If you found a DIR_END, the file is not in this directory
        if (res == END_OF_DIR)
            return FILE_NOT_FOUND;
        else if (res != OK)
            return res;

        // Make sure this is the name of the entry to delete
        if (strcmp(curr->name, name) != 0) 
            continue;
        *entry = curr;
        break;
    }

    return res;
}


/**
 * Delete an entry in a directory
 * @author Claziero
 */
FatResult dir_delete(FatFs *fs, int block_number, DirEntryType type, const char *name, int *child_block) {
    FatResult res;
    
    // Get the directory size
    DirEntry *curr;
    DirHandle dir;
    res = dir_get_entry(fs, block_number, name, &curr, &dir);
    if (res != OK)
        return res;

    // If type is -1 ignore the entry type
    if (type != -1) {
        if (type == DIR_ENTRY_FILE && curr->type == DIR_ENTRY_DIRECTORY)
            return NOT_A_FILE;
        else if (type == DIR_ENTRY_DIRECTORY && curr->type == DIR_ENTRY_FILE)
            return NOT_A_DIRECTORY;
    }


    // Save the block number of the entry to be returned
    if (child_block != NULL)
        *child_block = curr->first_block;

    // Keep listing the directory until you find the end
    DirEntry *next;
    int last_entry_block;
    while (curr->type != DIR_END) {
        last_entry_block = dir.block_number;

        // Get the next entry
        res = dir_handle_next(fs, &dir, &next);
        if (res != OK && res != END_OF_DIR)
            return res;

        // Copy the next entry to the current entry
        *curr = *next;

        // Move pointers
        curr = next;
    }

    // Compact the directory if necessary
    if (dir.block_number != last_entry_block) {
        // Free the last block
        bitmap_set(fs, dir.block_number, 0);
        // Set the next block to FAT_EOF
        fat_set_next_block(fs, last_entry_block, FAT_EOF);
    }

    return OK;
}

#define CEIL(x, y) (((x) + (y) - 1) / (y))


/**
 * Get the size of a directory or a file
 * @author Cicim
 */
FatResult get_recursive_size(FatFs *fs, int block_number, int type, int *size, int *blocks) {
    if (type == DIR_ENTRY_FILE) {
        // Get the file header
        FileHeader *header = (FileHeader *)(fs->blocks_ptr + block_number * fs->header->block_size);

        // Get the size
        *size = header->size;
        // Get the number of occupied blocks
        *blocks = CEIL(header->size + sizeof(FileHeader), fs->header->block_size);

        return OK;
    }

    // Loop over the directory
    DirEntry *curr;
    DirHandle dir;
    FatResult res;
    dir.block_number = block_number;
    dir.count = 0;

    int total_blocks = 0;
    int total_size = 0;

    while (1) {
        res = dir_handle_next(fs, &dir, &curr);

        if (res == END_OF_DIR)
            break;
        else if (res != OK)
            return res;

        // Count the file size
        int file_blocks, file_size;
        res = get_recursive_size(fs, curr->first_block, curr->type, &file_size, &file_blocks);
        if (res != OK)
            return res;
        // Add the file size to the total
        total_size += file_size;
        // Add the number of blocks to the total
        total_blocks += file_blocks;
    }

    // Add the directory size
    total_blocks += CEIL(dir.count + 1, ENTRIES_PER_BLOCK(fs));
    total_size += (dir.count + 1) * sizeof(DirEntry);

    *blocks = total_blocks;
    *size = total_size;

    return OK;
}
