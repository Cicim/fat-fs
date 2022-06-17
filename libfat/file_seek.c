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
    // Check if "file" parameter is valid
    if (file == NULL)
        return SEEK_INVALID_ARGUMENT;
    
    // Check if "offset" parameter is valid
    if (offset < 0)
        return SEEK_INVALID_ARGUMENT;
    
    int num_blocks, last_offset;
    
    // Switch on "whence" parameter
    switch (whence) {
        case FILE_SEEK_SET:
            if (offset > file->fh->size)
                return SEEK_INVALID_ARGUMENT;

            // Calculate the number of blocks to move starting from the first
            num_blocks = (offset + sizeof(FileHeader)) / file->fs->header->block_size;

            // Calculate the offset in the last block
            last_offset = (offset + sizeof(FileHeader)) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            file->current_block_number = file->initial_block_number;
            if (file->file_offset == file->fh->size && file->block_offset == 0)
                num_blocks--;
            while (num_blocks-- > 0)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            file->block_offset = last_offset;  
            file->file_offset = offset;
            break;

        case FILE_SEEK_CUR:
            if (file->file_offset + offset > file->fh->size)
                return SEEK_INVALID_ARGUMENT;

            // Calculate the number of blocks to move starting from the current block
            num_blocks = (file->block_offset + offset) / file->fs->header->block_size;

            // Calculate the offset in the last block
            last_offset = (file->block_offset + offset) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            if (file->file_offset == file->fh->size && file->block_offset == 0)
                num_blocks--;
            while (num_blocks-- > 0)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            file->block_offset = last_offset;
            file->file_offset += offset;
            break;

        case FILE_SEEK_END:            
            if (file->file_offset - offset < 0)
                return SEEK_INVALID_ARGUMENT;

            // Calculate the number of blocks to move starting from the first
            num_blocks = (file->fh->size + sizeof(FileHeader) - offset) / file->fs->header->block_size;

            // Calculate the offset in the last block
            last_offset = (file->fh->size - offset + sizeof(FileHeader)) % file->fs->header->block_size;

            // Move the file current block to the last block calculated
            file->current_block_number = file->initial_block_number;

            if (file->file_offset == file->fh->size && file->block_offset == 0)
                num_blocks--;
            while (num_blocks-- > 0)
                file->current_block_number = fat_get_next_block(file->fs, file->current_block_number);

            // Set the file offset to the last offset calculated
            file->block_offset = last_offset;
            file->file_offset = file->fh->size - offset;
            break;

        default:
            return SEEK_INVALID_ARGUMENT;
    }

    // If you reached the end of the file and it coincides with the end of a block
    if (file->file_offset == file->fh->size && file->block_offset == 0)
        file->block_offset = file->fs->header->block_size;        

    return OK;
}
