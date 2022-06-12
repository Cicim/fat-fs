/**
 * Header File for Internal Structs and Functions
 * @author Cicim
 */
#include "fat.h"


/**
 * Bitmap
 */
// Returns the bit value at the given block index
int bitmap_get(FatFs *fs, int block_number);
// Sets the bit value at the given block index
void bitmap_set(FatFs *fs, int block_number, int value);
// Returns the first free block in the bitmap
int bitmap_get_free_block(FatFs *fs);

/**
 * FAT
 */
#define FAT_EOF -1

// Returns the next block in the FAT table
#define fat_get_next_block(fs, block_number) (fs->fat_ptr[block_number])
// Sets the next block in the FAT table
#define fat_set_next_block(fs, block_number, next_block)\
    (fs->fat_ptr[block_number] = next_block)

/**
 * Paths
 */
// Stores the absolute path of the given file/directory
FatResult path_get_absolute(FatFs *fs, const char *path, char *dest);

/**
 * Directories
 */
#define ROOT_DIR_BLOCK 0

#define DIR_ENTRY_SIZE 32
#define DIR_ENTRY_BITS 5

#define ENTRIES_PER_BLOCK(fs) (fs->header->block_size >> DIR_ENTRY_BITS)

// Returns the first block of the directory given the path
FatResult dir_get_first_block(FatFs *fs, const char *path, int *block_number);
// Puts the next directory entry in *entry given the block number
FatResult dir_handle_next(FatFs *fs, DirHandle *dir, DirEntry **entry);
// Creates a new directory entry in the given directory
FatResult dir_insert(FatFs *fs, int block_number, DirEntry **entry, DirEntryType type, const char *name);
// Divides the given path into a directory and an element name
FatResult path_get_components(FatFs *fs, const char *path, char *path_buffer, char **dir_ptr, char **element_ptr);
