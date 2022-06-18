/**
 * Header File for User-Accessible Structs and Functions
 * @authors Cicim, Claziero
 */

typedef int date_t;

#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1
#define FILE_SEEK_END 2

#define MAX_FILENAME_LENGTH 27
#define MAX_PATH_LENGTH 512

typedef enum FatResult {
    OK = 0,

    FAT_BUFFER_ERROR = -2,
    INVALID_BLOCKS_COUNT = -3,
    FAT_OPEN_ERROR = -4,
    FAT_CLOSE_ERROR = -5,
    INVALID_PATH = -6,
    DIR_END_NOT_FOUND = -7,
    END_OF_DIR = -8,
    FILE_NOT_FOUND = -9,
    NOT_A_DIRECTORY = -10,
    NO_FREE_BLOCKS = -11,
    INVALID_BLOCK_SIZE = -12,
    FILE_ALREADY_EXISTS = -13,
    OUT_OF_MEMORY = -14,
    INVALID_BLOCK = -15,
    SEEK_INVALID_ARGUMENT = -16,
    NOT_A_FILE = -17,
    WRITE_INVALID_ARGUMENT = -18,
    FILE_OPEN_INVALID_ARGUMENT = -19,
    LS_INVALID_ARGUMENT = -20,
    SAME_PATH = -21,
} FatResult;

typedef enum DirEntryType {
    DIR_END = 0,
    DIR_ENTRY_FILE = 1,
    DIR_ENTRY_DIRECTORY = 2
} DirEntryType;


/**
 * Structs
 */
// Head bytes of the buffer
typedef struct FatHeader {
    unsigned int magic;
    unsigned int block_size;
    unsigned int blocks_count;
    unsigned int free_blocks;
} FatHeader;

// Handler for the file system
// stores both the header pointer and the current directory
typedef struct FatFs {
    char current_directory[MAX_PATH_LENGTH];
    FatHeader *header;
    int buffer_fd;
    int buffer_size;

    char *bitmap_ptr;
    int *fat_ptr;
    char *blocks_ptr;
} FatFs;

// Time struct
typedef struct DateTime {
    unsigned short sec:6;
    unsigned short min:6;
    unsigned short month:4;
    unsigned short hour:6;
    unsigned short day:5;
    unsigned short year;
} DateTime;

// File informations
typedef struct FileHeader {
    unsigned int size;
    DateTime date_created;
    DateTime date_modified;
} FileHeader;

// Data needed by operations on a file
typedef struct FileHandle { 
    FatFs *fs;
    FileHeader *fh;
    int initial_block_number;
    int current_block_number;
    int block_offset;
    int file_offset;
    char can_write:1;
    char can_read:1;
} FileHandle;

// Data needed by operations on a directory
typedef struct DirHandle {
    FatFs *fs;
    int block_number;
    int count;
} DirHandle;

// Data returned by listing a directory
typedef struct DirEntry {
    char name[MAX_FILENAME_LENGTH];
    char type;
    unsigned int first_block;
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

// Return a string representation of a FAT result
const char *fat_result_string(FatResult res);


/**
 * File Functions
 */
// Creates a file with the specified path
// returns an error if path is invalid
FatResult file_create(FatFs *fs, const char *path);

// Erases the file from the given path
// returns an error if path is invalid
FatResult file_erase(FatFs *fs, const char *path);

// Returns a file descriptor (struct FileHandle) given a block number 
FatResult file_open_by_block(FatFs *fs, int block_number, FileHandle **file);

// Creates a file handle given a path
// returns an error if path is invalid
FatResult file_open(FatFs *fs, const char *path, FileHandle **file, char *mode);

// Frees the memory occupied by a file handle
FatResult file_close(FileHandle *file);

// Prints the file contents to stdout
FatResult file_print(FileHandle *file);

// Changes file dimension
FatResult change_file_dimension(FileHandle *file, int size);

// Writes data from a buffer into file
// returns a FatResult or the number of written bytes
int file_write(FileHandle *file, const char *data, int size);

// Reads data from file into a buffer
// returns a FatResult or the number of read bytes
int file_read(FileHandle *file, char *buffer, int size);

// Moves the offset in the file handle in the specified location
// returns an error if such location is outside of file boundaries
FatResult file_seek(FileHandle *file, int offset, int whence);

// Moves a file or directory from a location to another
FatResult file_move(FatFs *fs, const char *source_path, const char *dest_path);

// Copy a file or directory from to another location
FatResult file_copy(FatFs *fs, const char *source_path, const char *dest_path);

// Gets the size and block size of a file or a directory
FatResult file_size(FatFs *fs, const char *path, int *size, int *blocks);

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
// returns END_OF_DIR if there are no more elements
FatResult dir_list(DirHandle *dir, DirEntry *entry);

// Changes the current directory to the given path
// returns an error if path is invalid
FatResult dir_change(FatFs *fs, const char *path);
