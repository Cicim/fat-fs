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


/**
 * Utility functions
 */
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

// Pipeline
#define TEST(n, c) \
    static const int test_count_##n = (c); \
    int test_##n(int ext) {                \
        int score = 0;                     \
        int test_number = 0;               \

#define INIT_TEMP_FS(fs, size, blocks) {\
    if (fat_init(TEMP_FILE, size, blocks) != OK) TEST_ABORT("Could not initialize temp FS");\
    if (fat_open(&fs, TEMP_FILE) != OK) TEST_ABORT("Could not open temp FS"); }

#define TEST_TITLE(text) \
    if (ext) printf(TEXT_BOLD "   %2d) " TEXT_RESET TEXT_UNDERLINE text TEXT_RESET "\n", ++test_number)

#define TEST_RESULT(operation, expected) {\
    FatResult result = operation;                               \
    if (result == expected) {                                   \
        score++;                                                \
        if (ext) printf("       " TEXT_CORRECT "✓ Got "         \
                        TEXT_BOLD "[%s] (%d)"                   \
                        TEXT_RESET TEXT_CORRECT                 \
                        " as expected\n" TEXT_RESET,            \
                        fat_result_string(result), result);     \
    } else {                                                    \
        if (ext) printf("       " TEXT_WRONG "✘ Got "           \
                        TEXT_BOLD "[%s] (%d)"                   \
                        TEXT_RESET TEXT_WRONG                   \
                        " but expected " TEXT_BOLD              \
                        "[%s] (%d)" TEXT_RESET TEXT_WRONG       \
                        "\n" TEXT_RESET,                        \
                        fat_result_string(result), result,      \
                        fat_result_string(expected), expected); \
        }                                                       \
    }

#define COMPARE_STRINGS(got, expected) \
    if (strcmp(got, expected) == 0) {                         \
        ++score;                                              \
        if (ext) printf("       " TEXT_CORRECT "✓ Got "       \
                        TEXT_BOLD "\"%s\"" TEXT_RESET         \
                        TEXT_CORRECT " as expected\n"         \
                        TEXT_RESET, got);                     \
    }                                                         \
    else if (ext) printf("       " TEXT_WRONG "✘ Got "        \
                        TEXT_BOLD "\"%s\"" TEXT_RESET         \
                        TEXT_WRONG " but expected "           \
                        TEXT_BOLD "\"%s\"" TEXT_RESET "\n",   \
                        got, expected);


#define OK_MESSAGE(text) { if (ext) puts("       " TEXT_CORRECT "✓ " text TEXT_RESET); score++; }
#define KO_MESSAGE(text) { if (ext) puts("       " TEXT_WRONG "✘ " text TEXT_RESET); }
#define TEST_ABORT(text) { KO_MESSAGE(text); goto cleanup; }

#define END return score; }

/** 
 * Helper functions
 */
FatResult file_exists(FatFs *fs, const char *path, DirEntryType type, int *block_number) {
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

#define TEST_EXISTS(path, type, block_number_ptr) { \
    FatResult result = file_exists(fs, path, type, block_number_ptr);           \
    if (ext) {                                                                  \
        if (result == FILE_NOT_FOUND)                                           \
            printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""               \
                   TEXT_RESET TEXT_WRONG " does not exist\n", path);            \
        else if (result == NOT_A_DIRECTORY)                                     \
            printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""               \
                   TEXT_RESET TEXT_WRONG " is not a directory\n", path);        \
        else if (result == NOT_A_FILE)                                          \
            printf(TEXT_WRONG "       ✘ Path " TEXT_BOLD "\"%s\""               \
                   TEXT_RESET TEXT_WRONG " is not a file\n", path);             \
        else if (result != OK)                                                  \
            TEST_ABORT("Error while checking the path");                        \
    }                                                                           \
    if (result == OK) {                                                         \
        score++;                                                                \
        if (ext) printf(TEXT_CORRECT "       ✓ Path " TEXT_BOLD "\"%s\""        \
                        TEXT_RESET TEXT_CORRECT " exists\n" TEXT_RESET, path);  \
    } }

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
    if (ext) {                                                   \
        printf("       FAT chain" TEXT_RESET ": "); \
        print_fat_links(fs, block_number); } }
    


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
    if (ftell(file) != 1156 + sizeof(FatHeader)) 
        KO_MESSAGE("Incorrect file size");
    OK_MESSAGE("The file has the correct size");
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
TEST(fat_open, 5) {
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
    TEST_PATH_COMPONENTS("No trailing slashes", "/dir/file/", "/dir", "file");
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
    if (header->date_created != 0 && header->date_modified != 0 && header->size == 0) {
        OK_MESSAGE("The header was initialized");
    } else KO_MESSAGE("The header was not initialized");

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
    file_exists(fs, "/file", DIR_ENTRY_FILE, &block_number);
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
    file_exists(fs, "/dir/file", DIR_ENTRY_FILE, &block_number);
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
    TEST_RESULT(dir_erase(fs, "/file/test"), NOT_A_FILE);

cleanup:
    fat_close(fs);
    if (file) file_close(file);
    END
}

// @author Cicim
TEST(dir_erase, 22) {
    FatFs *fs;
    DirHandle *dir = NULL;
    INIT_TEMP_FS(fs, 64, 32);

    TEST_TITLE("Erasing empty directory: /dir");
    int dir_block;
    dir_create(fs, "/dir");
    file_exists(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
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
    file_exists(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
    file_create(fs, "/dir/file1");
    file_exists(fs, "/dir/file1", DIR_ENTRY_FILE, &file1_block);
    file_create(fs, "/dir/file2");
    file_exists(fs, "/dir/file2", DIR_ENTRY_FILE, &file2_block);
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
    file_exists(fs, "/dir", DIR_ENTRY_DIRECTORY, &dir_block);
    file_create(fs, "/dir/file1");
    file_exists(fs, "/dir/file1", DIR_ENTRY_FILE, &file1_block);
    dir_create(fs, "/dir/subdir");
    file_exists(fs, "/dir/subdir", DIR_ENTRY_DIRECTORY, &subdir_block);
    file_create(fs, "/dir/subdir/file2");
    file_exists(fs, "/dir/subdir/file2", DIR_ENTRY_FILE, &file2_block);
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
    file_exists(fs, "/dir", DIR_ENTRY_DIRECTORY, &block_number);
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
    TEST_ENTRY(file_create),
    TEST_ENTRY(file_erase),
    TEST_ENTRY(dir_erase),
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
    printf(TEXT_CORRECT "All passed! %d/%d\n" TEXT_RESET, cum_score, max_score);

    return 0;
}
