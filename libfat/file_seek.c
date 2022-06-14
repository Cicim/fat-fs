/** 
 * File seek function
 * @author Claziero
 */

#include "internals.h"
#include <stdlib.h>

/**
 * Moves the offset in the file handle in the specified location
 * Returns an error if such location is outside of file boundaries
 * @author Claziero
 */
FatResult file_seek(FileHandle *file, int offset, int whence) {
    // Check if "whence" parameter is valid
    if (whence != FILE_SEEK_SET && whence != FILE_SEEK_CUR && whence != FILE_SEEK_END)
        return SEEK_INVALID_ARGUMENT;

    // Check if "file" parameter is valid
    if (file == NULL)
        return SEEK_INVALID_ARGUMENT;
    
    // Check if "offset" parameter is valid
    if (offset < 0 || offset > file->fh->size)
        return SEEK_INVALID_ARGUMENT;
    
    int num_blocks, last_offset;
    
    // Switch on "whence" parameter
    switch (whence) {
        case FILE_SEEK_SET:
            // Calculate the number of blocks to move starting from the first
            num_blocks = (offset + sizeof(FileHeader)) / file->fs->header->block_size;

            // Calculate the offset in the last block
            last_offset = (offset + sizeof(FileHeader)) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            file->current_block_number = file->initial_block_number;
            while (num_blocks--)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            file->offset = last_offset;  
            break;

        case FILE_SEEK_CUR:
            // Check if the current offset plus the offset is outside of file boundaries
            if (file->offset + offset > file->fh->size)
                return SEEK_INVALID_ARGUMENT;

            // Calculate the number of blocks to move starting from the current block
            num_blocks = (file->offset + offset) / file->fs->header->block_size;

            // Calculate the offset in the last block
            last_offset = (file->offset + offset) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            while (num_blocks--)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            file->offset = last_offset;
            break;

        case FILE_SEEK_END:
            // Calculate the number of blocks to move starting from the first
            num_blocks = (file->fh->size - offset) / file->fs->header->block_size;

            // Check if the offset is outside of file boundaries
            if (num_blocks < 0) {
                // Set the file offset to the last offset calculated
                file->offset = sizeof(FileHeader);
                file->current_block_number = file->initial_block_number;
                break;
            }

            // Calculate the offset in the last block
            last_offset = (file->fh->size - offset + sizeof(FileHeader)) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            file->current_block_number = file->initial_block_number;
            while (num_blocks--)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            if (num_blocks == -1 && last_offset == sizeof(FileHeader))
                file->offset = last_offset;
            else 
                file->offset = last_offset - 1;

            break;
    }

    return OK;
}
