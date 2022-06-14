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
        int test_count = test_count_##n;   \
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
#define KO_MESSAGE(text) { if (ext) puts("       " TEXT_WRONG text "✘ " TEXT_RESET); }
#define TEST_ABORT(text) { KO_MESSAGE(text); goto cleanup; }

#define END if (ext) printf("   Score: %d/%d\n", score, test_count); return score; }



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
TEST(path_get_absolute, 28) {
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


/**
 * Test selector
 */

#define TEST_COUNT sizeof(tests) / sizeof(TestData)
const struct TestData tests[] = {
    TEST_ENTRY(fat_init),
    TEST_ENTRY(fat_open),
    TEST_ENTRY(path_get_absolute),
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
