/**
 * FAT FS Test file
 * @author Cicim
 */
#include <stdio.h>
#include <string.h>
#include "libfat/internals.h"

/**
 * Defines
 */
#define TEMP_FILE "temp_buffer.dat"

// Struct for a test function type
typedef int(*TestFunctionType)(int ext);
// Struct for defining a test
typedef struct TestData {
    char *name;
    int max_score;
    TestFunctionType test_fn;
} TestData;
// Test shorthand
#define TEST_ENTRY(n) {.name = #n, .max_score = test_count_##n, .test_fn = test_##n}

// Text styles
#define TEXT_BOLD "\033[1m"
#define TEXT_UNDERLINE "\033[4m"
#define TEXT_STRIKE "\033[9m"
#define TEXT_BLUE "\033[34m"
#define TEXT_RED "\033[31m"
#define TEXT_RESET "\033[0m"
// The ANSI escape sequence for RGB foreground
#define TEXT_RGB_FG(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
// The ANSI escape sequence for RGB background
#define TEXT_RGB_BG(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"

#define TEXT_FN TEXT_BOLD TEXT_RGB_FG(170, 171, 40)

#define TEXT_CORRECT TEXT_RGB_FG(10, 190, 0)
#define TEXT_WRONG TEXT_RGB_FG(231, 88, 90)


/**
 * Utility functions
 */
/**
 * Gets the block number of the entry at the given path
 * @author Cicim
 */
FatResult get_file_blocknum(FatFs *fs, const char *path, DirEntryType type, int *block_number) {
    FatResult res;

    // Split "path" in directory and file name
    char path_buffer[MAX_PATH_LENGTH];
    char *dir_path, *name;
    res = path_get_components(fs, path, path_buffer, &dir_path, &name);
    if (res != OK)
        return res;

    
    // Look for the element in the directory
    DirHandle dir;
    DirEntry *entry;
    dir.count = 0;
    res = dir_get_first_block(fs, dir_path, &dir.block_number);
    if (res != OK)
        return res;
    
    while (1) {
        res = dir_handle_next(fs, &dir, &entry);
        
        if (res == END_OF_DIR)
            return FILE_NOT_FOUND;
        else if (res != OK)
            return res;


        if (strcmp(entry->name, name) != 0)
            continue;
        
        // Check if the type is right
        if (entry->type != type) {
            if (type == DIR_ENTRY_FILE)
                return NOT_A_FILE;
            else if (type == DIR_ENTRY_DIRECTORY)
                return NOT_A_DIRECTORY;
        }
        if (block_number)
            *block_number = entry->first_block;
        return OK;
    }
}

/**
 * Print the chain of links starting at the given block
 */
void print_fat_links(FatFs *fs, int bn) {
    while (bn != FAT_EOF) {
        if (bitmap_get(fs, bn) == 0)
            printf(TEXT_RED TEXT_STRIKE "%d" TEXT_RESET " -> ", bn);
        else
            printf(TEXT_BLUE TEXT_BOLD "%d" TEXT_RESET " -> ", bn);
        bn = fat_get_next_block(fs, bn);
    }
    printf("EOF\n");
}
#define PRINT_FAT_LINKS(block_number) { \
    if (ext) {                                      \
        printf("       FAT chain" TEXT_RESET ": "); \
        print_fat_links(fs, block_number); } }


void print_directory_tree(FatFs *fs, int block_number, int spaces) {
    DirHandle dir;
    DirEntry *entry;
    dir.count = 0;
    dir.block_number = block_number;

    while (1) {
        FatResult res = dir_handle_next(fs, &dir, &entry);
        if (res == END_OF_DIR)
            break;
        else if (res != OK) {
            printf(TEXT_WRONG "Error while reading directory\n");
            return;
        }
        
        if (entry->type == DIR_ENTRY_DIRECTORY) {
            printf(TEXT_BLUE "%*s%s/ " TEXT_RESET "-> ", spaces, " ", entry->name);
            print_fat_links(fs, entry->first_block);
            print_directory_tree(fs, entry->first_block, spaces + 2);
        } else {
            printf("%*s%s -> ", spaces, " ", entry->name);
            print_fat_links(fs, entry->first_block);
        }
    }
}
#define PRINT_DIRECTORY_TREE(block_number) { \
    if (ext) {                                      \
        printf("       Directory tree for %d" TEXT_RESET ": \n", block_number); \
        print_directory_tree(fs, block_number, 8); } }



/**
 * Testing functions and defines
 */
// Pipeline
#define TEST(n, c) \
    static const int test_count_##n = (c); \
    int test_##n(int ext) {                \
        int score = 0;                     \
        int test_number = 0;

#define INIT_TEMP_FS(fs, size, blocks) {\
    if (fat_init(TEMP_FILE, size, blocks) != OK) TEST_ABORT("Could not initialize temp FS");\
    if (fat_open(&fs, TEMP_FILE) != OK) TEST_ABORT("Could not open temp FS"); }

#define INIT_FILE(path, mode) {\
    if (file_open(&file, path, mode "+") != OK) TEST_ABORT("Could not open file");\
    get_file_blocknum(fs, path, DIR_ENTRY_BITS_FILE, &block_number); }

#define TEST_TITLE(text) \
    if (ext) printf(TEXT_BOLD "   %2d) " TEXT_RESET TEXT_UNDERLINE text TEXT_RESET "\n", ++test_number)


#define OK_MESSAGE(text) { if (ext) puts("       " TEXT_CORRECT "✓ " text TEXT_RESET); score++; }
#define KO_MESSAGE(text) { if (ext) puts("       " TEXT_WRONG "✘ " text TEXT_RESET); }
#define TEST_ABORT(text) { KO_MESSAGE(text); goto cleanup; }

#define END return score; }

// Test whether two results match and print a human-readable message
int _test_result(FatResult result, FatResult expected, int ext) {
    if (result == expected) {
        if (ext) printf("       " TEXT_CORRECT "✓ Got " TEXT_BOLD "[%s] (%d)"
                        TEXT_RESET TEXT_CORRECT " as expected\n" TEXT_RESET,
                        fat_result_string(result), result);
        return 1;
    }

    if (ext) 
        printf("       " TEXT_WRONG "✘ Got " TEXT_BOLD "[%s] (%d)"
                TEXT_RESET TEXT_WRONG " but expected " TEXT_BOLD "[%s] (%d)\n" TEXT_RESET,
                fat_result_string(result), result,
                fat_result_string(expected), expected);
    return 0;
}
#define TEST_RESULT(got, expected) score += _test_result(got, expected, ext)

// Test whether two strings match and print a human-readable message
int _test_string(const char *got, const char *expected, int ext) {
    if (strcmp(got, expected) == 0) {
        if (ext) 
            printf("       " TEXT_CORRECT "✓ Got " TEXT_BOLD "\"%s\"" TEXT_RESET
                    TEXT_CORRECT " as expected\n" TEXT_RESET, got);
        return 1;
    }
    if (ext) 
        printf("       " TEXT_WRONG "✘ Got " TEXT_BOLD "\"%s\"" TEXT_RESET
                TEXT_WRONG " but expected " TEXT_BOLD "\"%s\"" TEXT_RESET "\n",
                got, expected);
    return 0;
}
#define COMPARE_STRINGS(got, expected) score += _test_string(got, expected, ext)

// Test whether a file exists and print a human-readable message
int _test_exists(FatFs *fs, const char *path, int ext, DirEntryType type, int *block_number) {
    FatResult result = get_file_blocknum(fs, path, type, block_number);
    if (result == FILE_NOT_FOUND) {
        if (ext) printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""
                        TEXT_RESET TEXT_WRONG " does not exist\n", path);
    }
    else if (result == NOT_A_DIRECTORY) {
        if (ext) printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""
                        TEXT_RESET TEXT_WRONG " is not a directory\n", path);
    }
    else if (result == NOT_A_FILE) {
        if (ext) printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""
                        TEXT_RESET TEXT_WRONG " is not a file\n", path);
    }
    else if (result != OK) {
        if (ext) KO_MESSAGE("Error while checking the path");
        return -1;
    } else {
        if (ext) printf(TEXT_CORRECT "       ✓ Path " TEXT_BOLD "\"%s\""
                        TEXT_RESET TEXT_CORRECT " exists\n" TEXT_RESET, path);
        return 1;
    }

    return 0;
}
#define TEST_EXISTS(path, type, block_number_ptr) { \
    int result = _test_exists(fs, path, ext, type, block_number_ptr); \
    if (result == -1) goto cleanup; else score += result; }

void print_int_result(int result) {
    if (result <= 0) printf("[%s] [%d]", fat_result_string(result), result);
    else printf("%d bytes", result);
}

// Test results but may be a byte number if positive
int _test_int_result(int result, int expected, int ext) {
    if (result != expected) {
        if (ext) {
            printf("       " TEXT_WRONG "✘ Got " TEXT_BOLD);
            print_int_result(result);
            printf(TEXT_RESET TEXT_WRONG " but expected " TEXT_BOLD);
            print_int_result(expected);
            printf(TEXT_RESET "\n");
        }
        return 0;
    }

    if (ext) {
        printf("       " TEXT_CORRECT "✓ Got " TEXT_BOLD);
        print_int_result(result);
        printf(" as expected" TEXT_RESET "\n");
    }

    return 1;
}
#define TEST_INT_RESULT(got, expected) score += _test_int_result(got, expected, ext)

// Test whether two ints match and print a human-readable message
int _test_int(int got, int expected, const char *name, int ext) {
    if (got == expected) {
        if (ext) printf("       " TEXT_CORRECT "✓ Got %s " TEXT_BOLD "%d"
                        TEXT_RESET TEXT_CORRECT " as expected\n" TEXT_RESET, name, got);
        return 1;
    }

    if (ext) 
        printf("       " TEXT_WRONG "✘ Got %s " TEXT_BOLD "%d"
                TEXT_RESET TEXT_WRONG " but expected " TEXT_BOLD "%d" TEXT_RESET "\n",
                name, got, expected);
    return 0;
}
#define TEST_INT(name, got, expected) score += _test_int(got, expected, name, ext)


/**
 * Tests for each function
 */
// @author Cicim
TEST(fat_init, 10) {
    remove(TEMP_FILE);

    TEST_TITLE("Correct parameters");
    TEST_RESULT(fat_init(TEMP_FILE, 32, 32), OK);
    FILE *file = fopen(TEMP_FILE, "r");
    if (file == NULL) 
        TEST_ABORT("The file was not created");
    OK_MESSAGE("The file was created");
    fseek(file, 0, SEEK_END);
    TEST_INT("file size", ftell(file), 1156 + sizeof(FatHeader));
    fclose(file);

    TEST_TITLE("Trying to create a buffer in /std/null");
    TEST_RESULT(fat_init("/std/null", 32, 32), FAT_BUFFER_ERROR);

    // Negative numbers
    TEST_TITLE("Negative blocks count");
    TEST_RESULT(fat_init(TEMP_FILE, 32, -1), INVALID_BLOCKS_COUNT);
    TEST_TITLE("Negative block size");
    TEST_RESULT(fat_init(TEMP_FILE, -1, 32), INVALID_BLOCK_SIZE);
    // Zero numbers
    TEST_TITLE("Zero blocks count");
    TEST_RESULT(fat_init(TEMP_FILE, 32, 0), INVALID_BLOCKS_COUNT);
    TEST_TITLE("Zero block size");
    TEST_RESULT(fat_init(TEMP_FILE, 0, 32), INVALID_BLOCK_SIZE);
    // Numbers not divisible by 32
    TEST_TITLE("Blocks count not divisible by 32");
    TEST_RESULT(fat_init(TEMP_FILE, 32, 33), INVALID_BLOCKS_COUNT);
    TEST_TITLE("Block size not divisible by 32");
    TEST_RESULT(fat_init(TEMP_FILE, 33, 32), INVALID_BLOCK_SIZE);
cleanup:
    remove(TEMP_FILE);
    END
}

// @author Cicim
TEST(fat_open, 6) {
    FatFs *fs;

    remove(TEMP_FILE);
    fat_init(TEMP_FILE, 32, 32);

    TEST_TITLE("Correct parameters");
    TEST_RESULT(fat_open(&fs, TEMP_FILE), OK);
    // Make sure the current directory is the root
    if (!IS_ROOT(fs->current_directory))
        KO_MESSAGE("The current directory is not the root");
    OK_MESSAGE("The current directory is the root");

    TEST_TITLE("Closing with " TEXT_FN "fat_close()" TEXT_RESET);
    TEST_RESULT(fat_close(fs), OK);

    // Errors with file
    TEST_TITLE("Trying to open /std/null");
    TEST_RESULT(fat_open(&fs, "/std/null"), FAT_BUFFER_ERROR);
    TEST_TITLE("Trying to open a non-existing file");
    TEST_RESULT(fat_open(&fs, "non-existing-file"), FAT_BUFFER_ERROR);

    // Errors with buffer
    fopen(TEMP_FILE, "w");
    fwrite("Hello world", 1, 12, fopen(TEMP_FILE, "r"));
    TEST_TITLE("Trying to open a buffer with the wrong magic");
    TEST_RESULT(fat_open(&fs, TEMP_FILE), FAT_OPEN_ERROR);

cleanup:
    remove(TEMP_FILE);
    END
}

// @author Cicim
TEST(path_get_absolute, 30) {
    #define TEST_PATH_SUM(text, from, path, expected)                        \
        TEST_TITLE(text ": " from " + " path " = " expected);                \
        strcpy(fs->current_directory, from);                                 \
        TEST_RESULT(path_get_absolute(fs, path, fs->current_directory), OK); \
        COMPARE_STRINGS(fs->current_directory, expected);
    #define TEST_INVALID_PATH_SUM(text, from, path)     \
        TEST_TITLE(text ": " from " + " path);          \
        strcpy(fs->current_directory, from);            \
        TEST_RESULT(path_get_absolute(fs, path, fs->current_directory), INVALID_PATH);

    // Init a temp fs
    FatFs *fs;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("The current directory starts from the root");
    COMPARE_STRINGS(fs->current_directory, "/");

    TEST_TITLE("Empty-string addition");
    TEST_RESULT(path_get_absolute(fs, "", fs->current_directory), INVALID_PATH);

    // Move to different directories
    TEST_PATH_SUM("Relative addition", "/", "dir", "/dir");
    TEST_PATH_SUM("Nested directories", "/dir", "dir", "/dir/dir");
    TEST_PATH_SUM("Absolute addition", "/dir", "/dir/dir", "/dir/dir");
    TEST_PATH_SUM("Absolute ending in slash", "/", "/dir/", "/dir");
    TEST_PATH_SUM("Trailing slash", "/", "dir/", "/dir");
    TEST_PATH_SUM("Trailing slash", "/dir", "dir/", "/dir/dir");
    TEST_PATH_SUM("Directory up", "/dir", "..", "/");
    TEST_PATH_SUM("Directory up", "/dir", "../", "/");
    TEST_PATH_SUM("Directory up", "/dir", "../test", "/test");
    TEST_PATH_SUM("Directory up", "/dir/dir", "../test", "/dir/test");
    TEST_PATH_SUM("Same directory", "/dir", ".", "/dir");
    TEST_PATH_SUM("Same directory", "/", "./dir", "/dir");
    TEST_INVALID_PATH_SUM("Invalid token", "/dir", "...");
    TEST_INVALID_PATH_SUM("Too many directory up", "/", "..");
    TEST_INVALID_PATH_SUM("Too many directory up", "/dir", "../../test");
    TEST_INVALID_PATH_SUM(".. without the slash", "/dir", "..dir");

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(path_get_components, 14) {
    // Test the get_components function
    #define TEST_PATH_COMPONENTS(text, path, dir, file)\
        TEST_TITLE(text ": " path " = " dir " and " file);\
        TEST_RESULT(path_get_components(fs, path, path_buffer, &dir_path, &name), OK);\
        COMPARE_STRINGS(dir_path, dir);\
        COMPARE_STRINGS(name, file);


    FatFs *fs;
    char path_buffer[MAX_PATH_LENGTH];
    char *dir_path, *name;
    INIT_TEMP_FS(fs, 32, 32);

    TEST_TITLE("The current directory starts from the root");
    COMPARE_STRINGS(fs->current_directory, "/");

    TEST_TITLE("Splitting / should be invalid");\
    TEST_RESULT(path_get_components(fs, "/", path_buffer, &dir_path, &name), INVALID_PATH);

    TEST_PATH_COMPONENTS("Inside root", "/file", "/", "file");
    TEST_PATH_COMPONENTS("Inside subdirectory", "/dir/file", "/dir", "file");
    TEST_PATH_COMPONENTS("Trailing slashes", "/dir/file/", "/dir", "file");
    TEST_PATH_COMPONENTS("More subdirectories", "/dir/dir/file", "/dir/dir", "file");

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(dir_create, 10) {
    FatFs *fs;
    int block_number;
    INIT_TEMP_FS(fs, 32, 32);

    TEST_TITLE("Creating /dir");
    TEST_RESULT(dir_create(fs, "/dir"), OK);
    TEST_EXISTS("/dir", DIR_ENTRY_DIRECTORY, &block_number);
    TEST_TITLE("Creates the DIR_END entry");
    DirEntry *entry = (DirEntry *)(fs->blocks_ptr + block_number * fs->header->block_size);
    if (entry->type == DIR_END) {
        OK_MESSAGE("The DIR_END entry was created");
    } else KO_MESSAGE("The DIR_END entry is not created");

    TEST_TITLE("Creating /dir/dir");
    TEST_RESULT(dir_create(fs, "/dir/dir"), OK);
    TEST_EXISTS("/dir/dir", DIR_ENTRY_DIRECTORY, NULL);
    TEST_TITLE("Creating /dir/dir/dir");
    TEST_RESULT(dir_create(fs, "/dir/dir/dir"), OK);
    TEST_EXISTS("/dir/dir/dir", DIR_ENTRY_DIRECTORY, NULL);

    TEST_TITLE("Creating directory in a non-existing directory /test/dir");
    TEST_RESULT(dir_create(fs, "/test/dir"), FILE_NOT_FOUND);

    TEST_TITLE("Creating a directory that already exists: /dir");
    TEST_RESULT(dir_create(fs, "/dir"), FILE_ALREADY_EXISTS);
    TEST_TITLE("Creating a directory that already exists: /dir/dir");
    TEST_RESULT(dir_create(fs, "/dir/dir"), FILE_ALREADY_EXISTS);

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(file_create, 7) {
    FatFs *fs;
    int block_number;
    INIT_TEMP_FS(fs, 32, 32);

    TEST_TITLE("Creating /file");
    TEST_RESULT(file_create(fs, "/file"), OK);
    TEST_EXISTS("/file", DIR_ENTRY_FILE, &block_number);
    FileHeader *header = (FileHeader *)(fs->blocks_ptr + block_number * fs->header->block_size);
    TEST_INT("size", header->size, 0);

    // Try to create a file in a non-existing directory
    TEST_TITLE("Creating a file in a non-existing directory /test/file");
    TEST_RESULT(file_create(fs, "/test/file"), FILE_NOT_FOUND);
    // Try to create a file that already exists
    TEST_TITLE("Creating a file that already exists: /file");
    TEST_RESULT(file_create(fs, "/file"), FILE_ALREADY_EXISTS);

    // Try to create a file inside of a directory
    TEST_TITLE("Creating a file inside a directory: /dir/file");
    dir_create(fs, "/dir");
    TEST_RESULT(file_create(fs, "/dir/file"), OK);
    TEST_EXISTS("/dir/file", DIR_ENTRY_FILE, &block_number);

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(file_erase, 13) {
    FatFs *fs;
    FileHandle *file = NULL;
    int block_number;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("Erasing /file");
    file_create(fs, "/file");
    get_file_blocknum(fs, "/file", DIR_ENTRY_FILE, &block_number);
    PRINT_FAT_LINKS(block_number);
    TEST_RESULT(file_erase(fs, "/file"), OK);
    PRINT_FAT_LINKS(block_number);
    if (bitmap_get(fs, block_number)) {
        KO_MESSAGE("The block was not erased");
    } else OK_MESSAGE("The block was erased");
    TEST_RESULT(file_open(fs, "/file", &file, "r"), FILE_NOT_FOUND);


    TEST_TITLE("Erasing a file inside a directory: /dir/file");
    dir_create(fs, "/dir");
    file_create(fs, "/dir/file");
    get_file_blocknum(fs, "/dir/file", DIR_ENTRY_FILE, &block_number);
    PRINT_FAT_LINKS(block_number);
    TEST_RESULT(file_erase(fs, "/dir/file"), OK);
    PRINT_FAT_LINKS(block_number);
    TEST_RESULT(file_open(fs, "/dir/file", &file, "r"), FILE_NOT_FOUND);

    TEST_TITLE("Erasing a file spread over multiple blocks");
    file_create(fs, "/file");
    int next = bitmap_get_free_block(fs);
    fat_set_next_block(fs, block_number, next);
    bitmap_set(fs, next, 1);
    PRINT_FAT_LINKS(block_number);
    TEST_RESULT(file_erase(fs, "/file"), OK);
    PRINT_FAT_LINKS(block_number);
    if (bitmap_get(fs, block_number)) {
        KO_MESSAGE("The first block was not erased");
    } else OK_MESSAGE("The first block was erased");
    if (bitmap_get(fs, next)) {
        KO_MESSAGE("The second block was not erased");
    } else OK_MESSAGE("The second block was erased");
    if (fat_get_next_block(fs, block_number) != FAT_EOF) {
        KO_MESSAGE("The first block's next block was not set to EOF");
    } else OK_MESSAGE("The first block's next block was set to EOF");
    TEST_RESULT(file_open(fs, "/file", &file, "r"), FILE_NOT_FOUND);


    TEST_TITLE("Erasing a non-existing file: /file");
    TEST_RESULT(file_erase(fs, "/file"), FILE_NOT_FOUND);

    TEST_TITLE("Erasing a file inside a non-existing directory");
    TEST_RESULT(file_erase(fs, "/test/file"), FILE_NOT_FOUND);

    TEST_TITLE("Erasing a directory with file_erase: /dir/test");
    dir_create(fs, "/dir/test");
    TEST_RESULT(file_erase(fs, "/dir/test"), NOT_A_FILE);

cleanup:
    fat_close(fs);
    if (file) file_close(file);
    END
}

// @author Cicim
TEST(dir_erase, 21) {
    FatFs *fs;
    DirHandle *dir = NULL;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("Erasing empty directory: /dir");
    int dir_block;
    dir_create(fs, "/dir");
    get_file_blocknum(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
    PRINT_FAT_LINKS(dir_block);
    TEST_RESULT(dir_erase(fs, "/dir"), OK);
    PRINT_FAT_LINKS(dir_block);
    TEST_RESULT(dir_open(fs, "/dir", &dir), FILE_NOT_FOUND);
    if (bitmap_get(fs, dir_block)) {
        KO_MESSAGE("The block was not erased");
    } else OK_MESSAGE("The block was erased");

    int file1_block, file2_block;
    TEST_TITLE("Erasing a directory with files: /dir");
    dir_create(fs, "/dir");
    get_file_blocknum(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
    file_create(fs, "/dir/file1");
    get_file_blocknum(fs, "/dir/file1", DIR_ENTRY_FILE, &file1_block);
    file_create(fs, "/dir/file2");
    get_file_blocknum(fs, "/dir/file2", DIR_ENTRY_FILE, &file2_block);
    int next = fat_get_next_block(fs, dir_block);
    PRINT_FAT_LINKS(dir_block);
    TEST_RESULT(dir_erase(fs, "/dir"), OK);
    PRINT_FAT_LINKS(dir_block);
    if (bitmap_get(fs, dir_block)) {
        KO_MESSAGE("The first block was not erased");
    } else OK_MESSAGE("The first block was erased");
    if (bitmap_get(fs, next)) {
        KO_MESSAGE("The second block was not erased");
    } else OK_MESSAGE("The second block was erased");
    if (fat_get_next_block(fs, dir_block) != FAT_EOF) {
        KO_MESSAGE("The first block's next block was not set to EOF");
    } else OK_MESSAGE("The first block's next block was set to EOF");

    if (bitmap_get(fs, file1_block)) {
        KO_MESSAGE("file1 was not erased");
    } else OK_MESSAGE("file1 was erased");
    if (bitmap_get(fs, file2_block)) {
        KO_MESSAGE("file2 was not erased");
    } else OK_MESSAGE("file2 was erased");

    TEST_RESULT(dir_open(fs, "/dir", &dir), FILE_NOT_FOUND);

    int subdir_block;
    // Erasing a directory with subdirectories
    TEST_TITLE("Erasing a directory with subdirectories: /dir");
    dir_create(fs, "/dir");
    get_file_blocknum(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
    file_create(fs, "/dir/file1");
    get_file_blocknum(fs, "/dir/file1", DIR_ENTRY_FILE, &file1_block);
    dir_create(fs, "/dir/subdir");
    get_file_blocknum(fs, "/dir/subdir", DIR_ENTRY_DIRECTORY, &subdir_block);
    file_create(fs, "/dir/subdir/file2");
    get_file_blocknum(fs, "/dir/subdir/file2", DIR_ENTRY_FILE, &file2_block);
    next = fat_get_next_block(fs, dir_block);
    PRINT_FAT_LINKS(dir_block);
    PRINT_FAT_LINKS(subdir_block);
    TEST_RESULT(dir_erase(fs, "/dir"), OK);
    PRINT_FAT_LINKS(dir_block);
    PRINT_FAT_LINKS(subdir_block);
    if (bitmap_get(fs, dir_block)) {
        KO_MESSAGE("The first block was not erased");
    } else OK_MESSAGE("The first block was erased");
    if (bitmap_get(fs, next)) {
        KO_MESSAGE("The second block was not erased");
    } else OK_MESSAGE("The second block was erased");
    if (fat_get_next_block(fs, dir_block) != FAT_EOF) {
        KO_MESSAGE("The first block's next block was not set to EOF");
    } else OK_MESSAGE("The first block's next block was set to EOF");

    if (bitmap_get(fs, file1_block)) {
        KO_MESSAGE("file1 was not erased");
    } else OK_MESSAGE("file1 was erased");
    if (bitmap_get(fs, file2_block)) {
        KO_MESSAGE("file2 was not erased");
    } else OK_MESSAGE("file2 was erased");
    if (bitmap_get(fs, subdir_block)) {
        KO_MESSAGE("subdir was not erased");
    } else OK_MESSAGE("subdir was erased");

    TEST_RESULT(dir_open(fs, "/dir", &dir), FILE_NOT_FOUND);
    
    TEST_TITLE("Erasing a non-existing directory: /dir");
    TEST_RESULT(dir_erase(fs, "/dir"), FILE_NOT_FOUND);

    TEST_TITLE("Erasing a directory inside a non-existing directory");
    TEST_RESULT(dir_erase(fs, "/test/dir"), FILE_NOT_FOUND);

    TEST_TITLE("Erasing a file with dir_erase: /file");
    file_create(fs, "/file");
    TEST_RESULT(dir_erase(fs, "/file"), NOT_A_DIRECTORY);


cleanup:
    fat_close(fs);
    if (dir) dir_close(dir);
    END
}

// @author Cicim
TEST(dir_open, 12) {
    FatFs *fs;
    DirHandle *dir = NULL;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("Opening the root: /");
    TEST_RESULT(dir_open(fs, "/", &dir), OK);
    if (dir->block_number != 0) {
        KO_MESSAGE("The block number was not correctly initialized");
    } else OK_MESSAGE("The block number was correctly initialized");
    if (dir->count != 0) {
        KO_MESSAGE("The count was not correctly initialized");
    } else OK_MESSAGE("The count was correctly initialized");
    if (dir->fs != fs) {
        KO_MESSAGE("The fs pointer was not correctly initialized");
    } else OK_MESSAGE("The fs pointer was correctly initialized");

    TEST_TITLE("Closing it");
    TEST_RESULT(dir_close(dir), OK);

    int block_number;
    TEST_TITLE("Opening another directory: /dir");
    dir_create(fs, "/dir");
    TEST_RESULT(dir_open(fs, "/dir", &dir), OK);
    get_file_blocknum(fs, "/dir", DIR_ENTRY_DIRECTORY, &block_number);
    if (dir->block_number != block_number) {
        KO_MESSAGE("The block number was not correctly initialized");
    } else OK_MESSAGE("The block number was correctly initialized");
    if (dir->count != 0) {
        KO_MESSAGE("The count was not correctly initialized");
    } else OK_MESSAGE("The count was correctly initialized");
    if (dir->fs != fs) {
        KO_MESSAGE("The fs pointer was not correctly initialized");
    } else OK_MESSAGE("The fs pointer was correctly initialized");

    TEST_TITLE("Opening a non-existing directory: /fake");
    TEST_RESULT(dir_open(fs, "/fake", &dir), FILE_NOT_FOUND);

    TEST_TITLE("Opening a directory inside a non-existing directory: /test/dir");
    TEST_RESULT(dir_open(fs, "/test/dir", &dir), FILE_NOT_FOUND);

    TEST_TITLE("Opening a file with dir_open: /file");
    file_create(fs, "/file");
    TEST_RESULT(dir_open(fs, "/file", &dir), NOT_A_DIRECTORY);

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(dir_list, 16) {
    FatFs *fs;
    DirHandle *dir = NULL;
    DirEntry entry;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("Listing / with only file1 (no block jumping)");
    file_create(fs, "/file1");
    dir_open(fs, "/", &dir);
    TEST_RESULT(dir_list(dir, &entry), OK);
    COMPARE_STRINGS(entry.name, "file1");
    if (entry.type != DIR_ENTRY_FILE) {
        KO_MESSAGE("The entry type was not correctly initialized");
    } else OK_MESSAGE("The entry type was correctly initialized");
    TEST_RESULT(dir_list(dir, &entry), END_OF_DIR);
    dir_close(dir);

    TEST_TITLE("Listing / with 4 files (block jumping)");
    file_create(fs, "/file2");
    file_create(fs, "/file3");
    file_create(fs, "/file4");
    dir_open(fs, "/", &dir);
    TEST_RESULT(dir_list(dir, &entry), OK);
    COMPARE_STRINGS(entry.name, "file1");
    if (entry.type != DIR_ENTRY_FILE) {
        KO_MESSAGE("The entry type was not correctly initialized");
    } else OK_MESSAGE("The entry type was correctly initialized");
    TEST_RESULT(dir_list(dir, &entry), OK);
    COMPARE_STRINGS(entry.name, "file2");
    if (entry.type != DIR_ENTRY_FILE) {
        KO_MESSAGE("The entry type was not correctly initialized");
    } else OK_MESSAGE("The entry type was correctly initialized");
    TEST_RESULT(dir_list(dir, &entry), OK);
    COMPARE_STRINGS(entry.name, "file3");
    if (entry.type != DIR_ENTRY_FILE) {
        KO_MESSAGE("The entry type was not correctly initialized");
    } else OK_MESSAGE("The entry type was correctly initialized");
    TEST_RESULT(dir_list(dir, &entry), OK);
    COMPARE_STRINGS(entry.name, "file4");
    if (entry.type != DIR_ENTRY_FILE) {
        KO_MESSAGE("The entry type was not correctly initialized");
    } else OK_MESSAGE("The entry type was correctly initialized");

cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(file_open, 8) {
    FatFs *fs;
    FileHandle *file = NULL;
    INIT_TEMP_FS(fs, 64, 32);

    // Create a file
    file_create(fs, "/file");

    TEST_TITLE("Opening a file with no flags");
    TEST_RESULT(file_open(fs, "/file", &file, ""), FILE_OPEN_INVALID_ARGUMENT);

    TEST_TITLE("Opening a file with the READ flag");
    TEST_RESULT(file_open(fs, "/file", &file, "r"), OK);
    file_close(file);

    TEST_TITLE("Opening a file with the WRITE flag");
    TEST_RESULT(file_open(fs, "/file", &file, "w"), OK);
    file_close(file);

    TEST_TITLE("Opening a file with the APPEND flag");
    TEST_RESULT(file_open(fs, "/file", &file, "a"), OK);
    file_close(file);

    TEST_TITLE("Open with + and the file exists");
    TEST_RESULT(file_open(fs, "/file", &file, "w+"), OK);
    file_close(file);

    TEST_TITLE("Opening without + but the file does not exist");
    TEST_RESULT(file_open(fs, "/test", &file, "w"), FILE_NOT_FOUND);

    TEST_TITLE("Open with + but the file does not exist");
    TEST_RESULT(file_open(fs, "/test", &file, "w+"), OK);
    file_close(file);

    dir_create(fs, "/dir");
    TEST_TITLE("Opening a directory with file_open");
    TEST_RESULT(file_open(fs, "/dir", &file, "r"), NOT_A_FILE);


cleanup:
    fat_close(fs);
    END
}

// @author Cicim
TEST(file_write, 12) {
    FatFs *fs;
    FileHandle *file = NULL;
    int block_number;
    const char *str = "123456789ABCDEFGH012345678abcdefgh";

    // Create a file and open it
    INIT_TEMP_FS(fs, 32, 32);
    file_create(fs, "/file");
    get_file_blocknum(fs, "/file", DIR_ENTRY_FILE, &block_number);
    file_open(fs, "/file", &file, "w");

    TEST_TITLE("Writing 1 byte at the start of an empty file");
    PRINT_FAT_LINKS(block_number);
    TEST_INT_RESULT(file_write(file, str, 1), 1);
    PRINT_FAT_LINKS(block_number);
    TEST_INT("next block", fat_get_next_block(fs, 1), FAT_EOF);
    TEST_INT("size", file->fh->size, 1);

    TEST_TITLE("Adding 10 more bytes to the same file");
    PRINT_FAT_LINKS(block_number);
    TEST_INT_RESULT(file_write(file, str + 1, 10), 10);
    PRINT_FAT_LINKS(block_number);
    TEST_INT("next block", fat_get_next_block(fs, 1), FAT_EOF);
    TEST_INT("size", file->fh->size, 11);
    PRINT_FAT_LINKS(block_number);

    TEST_TITLE("If the size is 16 (block size - file header size) it should not allocate another block");
    PRINT_FAT_LINKS(block_number);
    TEST_INT_RESULT(file_write(file, str + 11, 5), 5);
    PRINT_FAT_LINKS(block_number);
    TEST_INT("next block", fat_get_next_block(fs, 1), FAT_EOF);
    TEST_INT("size", file->fh->size, 16);
    file_erase(fs, "/file");
    file_close(file);

    TEST_TITLE("Write 32 bytes to a file that is empty");
    file_open(fs, "/file2", &file, "a+");
    PRINT_FAT_LINKS(block_number);
    TEST_INT_RESULT(file_write(file, str, 32), 32);
    PRINT_FAT_LINKS(block_number);
    TEST_INT("next block", fat_get_next_block(fs, 1), 4);
    TEST_INT("size", file->fh->size, 32);


cleanup:
    file_close(file);

    END
}

// @author Cicim
TEST(file_move, 20) {
    FatFs *fs;
    int block_number;
    int block_number_2;
    INIT_TEMP_FS(fs, 32, 32);

    // Create a file structure
    // /dir
    //  /file
    //  /file2
    //  /file3
    // /dir2
    //  /file4
    //  /file5
    dir_create(fs, "/dir1");
    dir_create(fs, "/dir2");
    file_create(fs, "/dir1/file");
    file_create(fs, "/dir1/file2");
    file_create(fs, "/dir1/file3");
    file_create(fs, "/dir2/file4");
    file_create(fs, "/dir2/file5");

    // Save the block number of file5
    get_file_blocknum(fs, "/dir2/file5", DIR_ENTRY_FILE, &block_number_2);
    TEST_TITLE("Move /dir2/file5 to /dir1/file5");
    PRINT_DIRECTORY_TREE(0);
    TEST_RESULT(file_move(fs, "/dir2/file5", "/dir1/file5"), OK);
    PRINT_DIRECTORY_TREE(0);
    // Compare it with the block number of /dir1/file5
    TEST_EXISTS("/dir1/file5", DIR_ENTRY_FILE, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    // Move /dir1 inside /
    TEST_TITLE("Move /dir1 to /");
    PRINT_DIRECTORY_TREE(0);
    get_file_blocknum(fs, "/dir1", DIR_ENTRY_DIRECTORY, &block_number_2);
    TEST_RESULT(file_move(fs, "/dir1", "/"), FILE_ALREADY_EXISTS);
    PRINT_DIRECTORY_TREE(0);
    TEST_EXISTS("/dir1", DIR_ENTRY_DIRECTORY, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    // Move /dir1 inside /dir2
    TEST_TITLE("Move /dir1 to /dir2");
    PRINT_DIRECTORY_TREE(0);
    TEST_RESULT(file_move(fs, "/dir1", "/dir2"), OK);
    PRINT_DIRECTORY_TREE(0);
    TEST_EXISTS("/dir2/dir1", DIR_ENTRY_DIRECTORY, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    INIT_TEMP_FS(fs, 32, 32);
    // Create file structure
    // /dir
    //  /subdir
    //  /file
    dir_create(fs, "/dir");
    dir_create(fs, "/dir/subdir");
    file_create(fs, "/dir/file");

    TEST_TITLE("Move /dir/file to /");
    PRINT_DIRECTORY_TREE(0);
    get_file_blocknum(fs, "/dir/file", DIR_ENTRY_FILE, &block_number_2);
    TEST_RESULT(file_move(fs, "/dir/file", "/"), OK);
    PRINT_DIRECTORY_TREE(0);
    TEST_EXISTS("/file", DIR_ENTRY_FILE, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    TEST_TITLE("Move /subdir to /");
    PRINT_DIRECTORY_TREE(0);
    get_file_blocknum(fs, "/dir/subdir", DIR_ENTRY_DIRECTORY, &block_number_2);
    TEST_RESULT(file_move(fs, "/dir/subdir", "/"), OK);
    PRINT_DIRECTORY_TREE(0);
    TEST_EXISTS("/subdir", DIR_ENTRY_DIRECTORY, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    TEST_TITLE("Rename /subdir to /sd");
    PRINT_DIRECTORY_TREE(0);
    get_file_blocknum(fs, "/subdir", DIR_ENTRY_DIRECTORY, &block_number_2);
    TEST_RESULT(file_move(fs, "/subdir", "/sd"), OK);
    PRINT_DIRECTORY_TREE(0);
    TEST_EXISTS("/sd", DIR_ENTRY_DIRECTORY, &block_number);
    TEST_INT("block number", block_number, block_number_2);

    // Same path
    TEST_TITLE("Move /subdir to /subdir");
    TEST_RESULT(file_move(fs, "/dir/subdir", "/dir/subdir"), SAME_PATH);

    TEST_TITLE("Move the root around");
    TEST_RESULT(file_move(fs, "/", "/subdir"), INVALID_PATH);
    fat_close(fs);

cleanup:
    fat_close(fs);
    END
}



/**
 * Test selector
 */
#define TEST_COUNT sizeof(tests) / sizeof(TestData)
const struct TestData tests[] = {
    TEST_ENTRY(fat_init),
    TEST_ENTRY(fat_open),
    TEST_ENTRY(path_get_absolute),
    TEST_ENTRY(path_get_components),
    TEST_ENTRY(dir_create),
    TEST_ENTRY(dir_open),
    TEST_ENTRY(dir_list),
    TEST_ENTRY(file_create),
    TEST_ENTRY(file_erase),
    TEST_ENTRY(dir_erase),
    TEST_ENTRY(file_open),
    TEST_ENTRY(file_write),
    TEST_ENTRY(file_move),
};

int main(int argc, char **argv) {
    int is_all = 0;

    // Check if the first argument is "all"
    if (argc == 1 || strcmp(argv[1], "all") == 0)
        is_all = 1;

    if (is_all)
        printf("Running all tests:\n");

    // Run all tests
    int max_score = 0;
    int cum_score = 0;
    for (int i = 0; i < TEST_COUNT; i++) {
        if (is_all || strcmp(argv[1], tests[i].name) == 0) {
            if (!is_all)
                printf("Testing " TEXT_FN "%s" TEXT_RESET ":\n", tests[i].name);

            int score = tests[i].test_fn(!is_all);
            cum_score += score;
            max_score += tests[i].max_score;

            if (is_all) {
                if (score != tests[i].max_score) {
                    printf(" - Testing" TEXT_FN " %s " TEXT_WRONG "failed with score %d/%d\n" TEXT_RESET, tests[i].name, score, tests[i].max_score);
                    tests[i].test_fn(1);
                } else
                    printf(" - Testing" TEXT_FN " %s " TEXT_CORRECT "passed with score %d/%d\n" TEXT_RESET, tests[i].name, score, tests[i].max_score);
            }
        }
    }

    if (max_score == 0) {
        printf(TEXT_WRONG "No test with the name " TEXT_FN "%s" TEXT_RESET "\n", argv[1]);
        return 2;
    }

    printf("Finished testing!\n");
    if (cum_score != max_score) {
        printf(TEXT_WRONG "Something failed: %d/%d\n" TEXT_RESET, cum_score, max_score);
        printf("See above for a full trace of everything that failed\n");
        return 1;
    }
    const char *msg_start = is_all ? "All tests" : "Test";
    printf(TEXT_CORRECT "%s passed! %d/%d\n" TEXT_RESET, msg_start, cum_score, max_score);

    return 0;
}
