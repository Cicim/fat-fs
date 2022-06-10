/**
 * Header File for User-Accessible Structs and Functions
 * @author Cicim
 */

#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1
#define FILE_SEEK_END 2

typedef enum FatResult {
    OK = 0,

    DIRECTORY_END = -1,
    FAT_BUFFER_ERROR = -2,
    INVALID_BLOCKS_COUNT = -3,
    FAT_OPEN_ERROR = -4,
    FAT_CLOSE_ERROR = -5
} FatResult;

/**
 * Structs
 */
// Head bytes of the buffer
typedef struct FatHeader {
    unsigned int block_size;
    unsigned int blocks_count;

    unsigned int bitmap_offset;
    unsigned int fat_offset;
    unsigned int blocks_offset;
} FatHeader;

// Handler for the file system
// stores both the header pointer and the current directory
typedef struct FatFs {
    char current_directory[256];
    FatHeader *header;
    int buffer_fd;
    int buffer_size;

    char *bitmap_ptr;
    char *fat_ptr;
    char *blocks_ptr;
} FatFs;

// Data needed by operations on a file
typedef struct FileHandle { } FileHandle;

// Data needed by operations on a directory
typedef struct DirHandle { } DirHandle;

// Data returned by listing a directory
typedef struct DirEntry {
    char name[256];
    char type;
    unsigned int size;
} DirEntry;

/**
 * File System Functions
 */
// Create a file system and save it to a file
FatResult fat_init(const char *fat_path, int block_size, int blocks_count);

// Open an initialized FAT file system from a path
FatResult fat_open(FatFs **fs, char *fat_path);

// Close a file system and save its contents to a file
FatResult fat_close(FatFs *fs);

/**
 * File Functions
 */
// Creates a file with the specified path
// returns an error if path is invalid
FatResult file_create(FatFs *fs, const char *path);

// Erases the file from the given path
// returns an error if path is invalid
FatResult file_erase(FatFs *fs, const char *path);

// Creates a file handle given a path
// returns an error if path is invalid
FatResult file_open(FatFs *fs, const char *path, FileHandle **file);

// Frees the memory occupied by a file handle
FatResult file_close(FileHandle *file);

// Writes data from a buffer into file
// returns a FatResult or the number of written bytes
int file_write(FileHandle *file, const char *data, int size);

// Reads data from file into a buffer
// returns a FatResult or the number of read bytes
int file_read(FileHandle *file, char *buffer, int size);

// Moves the offset in the file handle in the specified location
// returns an error if such location is outside of file boundaries
FatResult file_seek(FileHandle *file, int offset, int whence);


/**
 * Directory Functions
 */
// Creates a directory with the specified path
// returns an error if path is invalid
FatResult dir_create(FatFs *fs, const char *path);

// Erases the directory given a path
// returns an error if path is invalid
FatResult dir_erase(FatFs *fs, const char *path);

// Creates a directory handle given a path
// returns an error if path is invalid
FatResult dir_open(FatFs *fs, const char *path, DirHandle **dir);

// Frees the memory occupied by a directory handle
FatResult dir_close(DirHandle *dir);

// Get the next element in the directory
// returns an error if path is invalid
// returns DIRECTORY_END if there are no more elements
FatResult dir_list(DirHandle *dir, DirEntry *entry);

// Changes the current directory to the given path
// returns an error if path is invalid
FatResult dir_change(FatFs *fs, const char *path);
