/**
 * Initialize the FAT file system.
 * @authors Cicim, Claziero
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "internals.h"

/**
 * Create a file system and save it to a file
 * @author Claziero
 */
FatResult fat_init(const char *fat_path, int block_size, int blocks_count) {
    // Check if the number of blocks is valid (must be multiple of 32)
    if (blocks_count <= 0 || blocks_count % 32 != 0) 
        return INVALID_BLOCKS_COUNT;

    // Check if the block size is valid (must be multiple of 32)
    if (block_size <= 0 || block_size % 32 != 0) 
        return INVALID_BLOCK_SIZE;

    // Create and initialize the FAT header
    FatHeader header;
    header.magic = FAT_MAGIC;
    header.block_size = block_size;
    header.blocks_count = blocks_count;
    header.free_blocks = blocks_count - 1;
    
    // The bitmap begins after the header
    int bitmap_offset = sizeof(FatHeader);
    // The FAT table begins after the bitmap
    int fat_offset = bitmap_offset + (blocks_count / 8);
    // The blocks begin after the FAT
    int blocks_offset = fat_offset + (blocks_count * sizeof(int));


    // Open the FAT file
    int fat_fd = open(fat_path, O_RDWR | O_CREAT | O_TRUNC, 0660);
    if (fat_fd == -1)
        return FAT_BUFFER_ERROR;

    // Write the header to the FAT file
    int written_bytes = 0;
    while (written_bytes < sizeof(FatHeader)) {
        written_bytes += write(fat_fd, &header + written_bytes, sizeof(FatHeader) - written_bytes);   
        
        // Check if the write succeeded
        if (written_bytes == sizeof(FatHeader))
            break;
        else if (errno == EINTR)
            // If the write was interrupted by a signal, try again
            continue;
        else
            // If the write failed, return a FAT_BUFFER_ERROR
            return FAT_BUFFER_ERROR;
    }

    // Initialize the bitmap with zeros
    if (ftruncate(fat_fd, bitmap_offset + (blocks_count / 8)) != 0)
        return FAT_BUFFER_ERROR;

    // Set the first bit to 1 (always occupied by the root directory)
    if (lseek(fat_fd, bitmap_offset, SEEK_SET) == -1)
        return FAT_BUFFER_ERROR;
    if (write(fat_fd, "\x01", 1) != 1)
        return FAT_BUFFER_ERROR;

    // Move the file pointer to the beginning of the FAT table
    char ff[4] = "\xff\xff\xff\xff";
    if (lseek(fat_fd, fat_offset, SEEK_SET) == -1)
        return FAT_BUFFER_ERROR;
    
    // Initialize the FAT table with -1 (empty table)
    for (int i = 0; i < blocks_count; i++) {
        int written_bytes = 0;
        while (written_bytes < sizeof(FatHeader)) {
            written_bytes += write(fat_fd, &ff + written_bytes, sizeof(int) - written_bytes);   
            
            // Check if the write succeeded
            if (written_bytes == sizeof(int))
                break;
            else if (errno == EINTR)
                // If the write was interrupted by a signal, try again
                continue;
            else
                // If the write failed, return a FAT_BUFFER_ERROR
                return FAT_BUFFER_ERROR;
        }
    }

    // Initialize the blocks with zeros
    if (ftruncate(fat_fd, blocks_offset + (blocks_count * block_size)) != 0)
        return FAT_BUFFER_ERROR;

    // Close the FAT file
    close(fat_fd);

    return OK;
}

/**
 * Open an initialized FAT file system from a path
 * @author Cicim
 */
FatResult fat_open(FatFs **fs, char *fat_path) {
    *fs = NULL;

    // Open the file from the path
    int fd = open(fat_path, O_RDWR);
    if (fd == -1) {
        return FAT_BUFFER_ERROR;
    }

    // Get the file size
    struct stat st;
    fstat(fd, &st);
    int file_size = st.st_size;

    // Map the file to memory
    char *fat_buffer = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fat_buffer == MAP_FAILED) {
        close(fd);
        return FAT_OPEN_ERROR;
    }

    // If the magic is wrong
    if (*(int *)fat_buffer != FAT_MAGIC) {
        munmap(fat_buffer, file_size);
        close(fd);
        return FAT_OPEN_ERROR;
    }

    // Init the FatFs struct
    *fs = malloc(sizeof(FatFs));
    if (*fs == NULL) {
        munmap(fat_buffer, file_size);
        close(fd);
        return FAT_OPEN_ERROR;
    }
    (*fs)->buffer_fd = fd;
    (*fs)->buffer_size = file_size;
    (*fs)->header = (FatHeader *) fat_buffer;
    (*fs)->current_directory[0] = '/';
    (*fs)->current_directory[1] = '\0';

    int blocks_count = (*fs)->header->blocks_count;

    // The bitmap begins after the header
    (*fs)->bitmap_ptr = fat_buffer + sizeof(FatHeader);
    // The FAT table begins after the bitmap
    (*fs)->fat_ptr = (*fs)->bitmap_ptr + (blocks_count / 8);
    // The blocks begin after the FAT
    (*fs)->blocks_ptr = (*fs)->fat_ptr + (blocks_count * sizeof(int));

    return OK;
}

/**
 * Close a file system and save its contents to a file
 * @author Cicim
 */
FatResult fat_close(FatFs *fs) {
    // Unmap the file from memory
    int ret = munmap(fs->header, fs->buffer_size);
    if (ret == -1) {
        return FAT_CLOSE_ERROR;
    }

    // Close the file
    ret = close(fs->buffer_fd);
    if (ret == -1) {
        return FAT_CLOSE_ERROR;
    }

    // Free the memory
    free(fs);
    
    return OK;
}
