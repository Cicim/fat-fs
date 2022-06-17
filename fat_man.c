/**
 * FAT File System Manager
 * @authors Cicim, Claziero
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libfat/fat.h"

#define FALSE 0
#define TRUE 1

#define COMMAND_NAME "fat_man"
#define MAX_COMMAND_ARGUMENTS 4

/**
 * Utility functions
 */
// Text styles
#define TEXT_BLUE "\033[34m"
#define TEXT_GREEN "\033[32m"
#define TEXT_RESET "\033[0m"

/**
 * Commands
 */

/**
 * Change directory
 * @author Cicim
 */
FatResult cmd_cd(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    return dir_change(fs, path);
}

/**
 * Create a directory
 * @author Cicim
 */
FatResult cmd_mkdir(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    return dir_create(fs, path);
}

/**
 * Create a file
 * @author Cicim
 */
FatResult cmd_touch(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    return file_create(fs, path);
}

/**
 * Move a file
 * @author Cicim
 */
FatResult cmd_mv(FatFs *fs, const char *source_path, const char *dest_path) {
    if (source_path == NULL || dest_path == NULL)
        return INVALID_PATH;

    return file_move(fs, source_path, dest_path);
}

/**
 * Print the contents of the file to stdout
 * @author Cicim
 */
FatResult cmd_cat(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    FileHandle *file;
    FatResult res = file_open(fs, path, &file, "r");
    if (res != OK)
        return res;

    res = file_print(file);
    file_close(file);
    return res;
}

/**
 * Remove a file
 * @author Cicim
 */
FatResult cmd_rm(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    return file_erase(fs, path);
}

/**
 * Remove a directory
 * @author Cicim
 */
FatResult cmd_rmdir(FatFs *fs, const char *path) {
    if (path == NULL)
        return INVALID_PATH;

    return dir_erase(fs, path);
}

/**
 * Print the number of free blocks in the file system
 * @author Cicim
 */
FatResult cmd_free(FatFs *fs) {
    int free_blocks = fs->header->free_blocks;
    int free_bytes = free_blocks * fs->header->block_size;

    printf("%d free blocks (%d bytes)\n", free_blocks, free_bytes);

    return OK;
}

/**
 * Copy a file from the external fs to the FAT FS
 * @author Cicim
 */
FatResult cmd_ec(FatFs *fs, const char *external_path, const char *internal_path) {
    if (external_path == NULL || internal_path == NULL)
        return INVALID_PATH;
    
    // Open the external file
    FILE *external_file = fopen(external_path, "r");
    if (external_file == NULL) {
        printf("Could not open external file: %s\n", external_path);
        return OK;
    }
    // Get its size
    fseek(external_file, 0, SEEK_END);
    int size = ftell(external_file);
    // If the file is too large, exit
    if (size > (fs->header->free_blocks - 2) * fs->header->block_size) {
        printf("File does not fit in this file system\n");
        return OK;
    }
    // Seek to the beginning of the file
    fseek(external_file, 0, SEEK_SET);

    // Open the file in write mode in the internal fs
    FileHandle *file;
    FatResult res = file_open(fs, internal_path, &file, "w+");
    if (res != OK)
        return res;

    // Write the file
    char buffer[128];
    int read_bytes = 0;
    while (1) {
        read_bytes = fread(buffer, 1, 128, external_file);
        if (read_bytes == 0)
            break;
        if (read_bytes < 0) {
            printf("Error while reading the external file\n");
            return OK;
        }

        res = file_write(file, buffer, read_bytes);
        if (res != read_bytes)
            return res;
    }

    // Close the files
    file_close(file);
    fclose(external_file);

    return OK;
}

/**
 * Append a number of bytes to the file
 * @author Cicim
 */
FatResult cmd_append(FatFs *fs, const char *path, char *character, int bytes) {
    if (path == NULL)
        return INVALID_PATH;
    if (character == NULL) {
        printf("append error: No string passed\n");
        return OK;
    }

    // Open the file in write mode in the internal fs
    FileHandle *file;
    FatResult res = file_open(fs, path, &file, "a");
    if (res != OK)
        return res;
    
    // Write to the file
    for (int i = 0; i < bytes; i++)
        if (file_write(file, character, 1) != 1)
            return res;

    file_close(file);
    return OK;
}

/**
 * LS command argument parsing
 * @author Claziero
 */
typedef struct ls_args {
    char argument_all:1;
    char argument_long:1;
    char argument_unknown:1;
    char argument_path:1;
} ls_args;

void ls_parse_argument(char *arg, ls_args *args) {
    if (arg == NULL) 
        return;

    if (arg[0] == '-') {
        if (arg[1] == '-') {
            if (strcmp(arg, "--all") == 0)
                args->argument_all = TRUE;
            else
                args->argument_unknown = TRUE;
            return;
        }

        for (int i = 1; arg[i]; i++) {
            if (arg[i] == 'l')
                args->argument_long = TRUE;
            else if (arg[i] == 'a')
                args->argument_all = TRUE;
            else
                args->argument_unknown = TRUE;
        }
    } else args->argument_path = TRUE;
}

/**
 * List directory contents
 * @author Claziero
 */
FatResult cmd_ls(FatFs *fs, char **command_arguments) {
    if (command_arguments == NULL)
        return INVALID_PATH;

    DirHandle *dir = NULL;
    DirEntry entry;
    FatResult res;
    ls_args args = {0};
    char **arg = command_arguments;

    // Look for all arguments
    int i = 0, path = -1;
    while (*arg != NULL) {
        ls_parse_argument(*arg, &args);
        
        if (args.argument_unknown) return LS_INVALID_ARGUMENT;
        if (args.argument_path) path = i;

        i++;
        arg++;
    }

    // Open the correct directory
    if (!args.argument_path) {
        res = dir_open(fs, fs->current_directory, &dir);
        if (res != OK)
            return res;
    }
    else {
        res = dir_open(fs, command_arguments[path], &dir);
        if (res != OK)
            return res;
    }

    // If "-a" is used, list also "." and ".." directories
    if (args.argument_all) {
        printf(TEXT_BLUE ".   " TEXT_RESET);
        if (args.argument_long) printf("\n");
        
        printf(TEXT_BLUE "..   " TEXT_RESET);
        if (args.argument_long) printf("\n");
    }

    // List the contents
    while ((res = dir_list(dir, &entry)) == OK) {
        if (entry.type == DIR_ENTRY_DIRECTORY) {
            // Condition on hidden files
            if (!args.argument_all && entry.name[0] == '.') continue;

            printf(TEXT_BLUE "%s   " TEXT_RESET, entry.name);
            if (args.argument_long) printf("\n");
        }
        else {
            // Condition on hidden files
            if (!args.argument_all && entry.name[0] == '.') continue;

            printf("%s   ", entry.name);
            if (args.argument_long) printf("\n");
        }
    }
    if (!args.argument_long) printf("\n");

    // Close the directory
    res = dir_close(dir);
    if (res != OK)
        return res;
    
    return OK;
}

/**
 * Help printing
 * @author Cicim
 */
void help() {
    printf(
        "Usage: "COMMAND_NAME" [arguments] <file> [command]\n"
        "    --help (-h) Show this message\n"
        "    --shell (-s) Open the file system in shell mode\n"
        "    --init (-i) Initialize the file system\n"
        "Use --help <command> to get more information about each command\n"
    );
}
void help_init() {
    printf(
        "Usage: "COMMAND_NAME" -i <file> blocks <blocks count> size <block size>\n"
        " Initializes a file system with 160 blocks of size 128 Bytes\n"
        " Note: both values should be positive and divisible by 32\n"
        "Usage: "COMMAND_NAME" -i -s <file>\n"
        " Shows a prompt to initialize the file system\n"
    );
}


/**
 * Argument parsing
 * @author Cicim
 */
#define ARGUMENT_HELP 0
#define ARGUMENT_SHELL 1
#define ARGUMENT_INIT 2
#define ARGUMENT_UNKNOWN -1
#define ARGUMENT_NONE -2

int parse_argument(char *arg) {
    if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
        return ARGUMENT_HELP;
    else if (strcmp(arg, "--shell") == 0 || strcmp(arg, "-s") == 0)
        return ARGUMENT_SHELL;
    else if (strcmp(arg, "--init") == 0 || strcmp(arg, "-i") == 0)
        return ARGUMENT_INIT;
    else if (arg[0] == '-')
        return ARGUMENT_UNKNOWN;
    else
        return ARGUMENT_NONE;
}

/**
 * Commands parsing
 * @author Cicim
 */
void parse_command(FatFs *fs, char *command[MAX_COMMAND_ARGUMENTS]) {
    FatResult res = OK;

    char *cmd_name = command[0];
    if (cmd_name == NULL)
        return;

    // Compare each command's name
    if (strcmp(cmd_name, "cd") == 0)
        res = cmd_cd(fs, command[1]);
    else if (strcmp(cmd_name, "mkdir") == 0)
        res = cmd_mkdir(fs, command[1]);
    else if (strcmp(cmd_name, "touch") == 0)
        res = cmd_touch(fs, command[1]);
    else if (strcmp(cmd_name, "ls") == 0)
        res = cmd_ls(fs, command + 1);
    else if (strcmp(cmd_name, "mv") == 0)
        res = cmd_mv(fs, command[1], command[2]);
    else if (strcmp(cmd_name, "cat") == 0)
        res = cmd_cat(fs, command[1]);
    else if (strcmp(cmd_name, "rm") == 0)
        res = cmd_rm(fs, command[1]);
    else if (strcmp(cmd_name, "rmdir") == 0)
        res = cmd_rmdir(fs, command[1]);
    else if (strcmp(cmd_name, "free") == 0)
        res = cmd_free(fs);
    else if (strcmp(cmd_name, "ec") == 0)
        res = cmd_ec(fs, command[1], command[2]);
    else if (strcmp(cmd_name, "append") == 0)
        res = cmd_append(fs, command[1], command[2], command[3] ? atoi(command[3]) : 1);
    else
        printf("Unknown command: %s\n", cmd_name);

    if (res != OK)
        printf("%s error: %s\n", cmd_name, fat_result_string(res));
}


/** 
 * Shell mode
 * @author Cicim
 */
int run_shell(char *file) {
    FatResult res;

    // Open the file system
    FatFs *fs;
    res = fat_open(&fs, file);
    if (res != OK) {
        printf("Error opening file system: %d\n", res);
        return 1;
    }

    printf("Welcome to FAT Manager. Type 'exit' to quit.\n");
    char input[MAX_PATH_LENGTH * MAX_COMMAND_ARGUMENTS];
    while (1) {
        // Prompt
        printf("%s> ", fs->current_directory);
        // Ask for an input
        fgets(input, sizeof(input), stdin);
        // Remove the \n
        input[strlen(input) - 1] = '\0';

        // If it is "exit", break the loop
        if (strcmp(input, "exit") == 0)
            break;
        
        // Split the input into commands
        char *command[MAX_COMMAND_ARGUMENTS] = {0};
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL && i < MAX_COMMAND_ARGUMENTS) {
            command[i] = token;
            token = strtok(NULL, " ");
            i++;
        }

        parse_command(fs, command);
    }


    // Close the file system
    res = fat_close(fs);
    if (res != OK) {
        printf("Error closing file system: %d\n", res);
        return 1;
    }

    return 0;
}

/**
 * Initialize the file system
 * @author Cicim
 */
void ask_for_init_params(int *block_size, int *blocks_count) {
    while(1) {
        printf("Enter the block size:\n>>> ");
        scanf("%d", block_size);
        printf("Enter the blocks count:\n>>> ");
        scanf("%d", blocks_count);

        if (*block_size < 0 || *blocks_count < 0) {
            printf("Both numbers should be be positive integers!\n");
            continue;
        }
        else if (*block_size % 32 != 0 || *blocks_count % 32 != 0) {
            printf("Both numbers should be be divisible by 32!\n");
            continue;
        } else break;
    }
}


/**
 * Main function
 * @author Cicim
 */
int main(int argc, char **argv) {
    int show_interactive_shell = FALSE;
    int initialize_file_system = FALSE;

    // If there is no argument, show help
    if (argc == 1) {
        help();
        return 0;
    }

    // Look for all arguments
    int i = 1;
    int argument = parse_argument(argv[i]);
    while (argument != ARGUMENT_NONE) {
        switch (argument) {
            case ARGUMENT_HELP:
                help();
                return 0;
            case ARGUMENT_SHELL:
                show_interactive_shell = TRUE;
                break;
            case ARGUMENT_INIT:
                initialize_file_system = TRUE;
                break;
            case ARGUMENT_UNKNOWN:
                printf("Unknown argument: %s\n", argv[i]);
                return 1;
        }
        if (++i == argc)
            break;
        argument = parse_argument(argv[i]);
    }

    // Next must contain the file
    if (i == argc) {
        printf(COMMAND_NAME": Missing FAT FS file\n");
        return 1;
    }
    char *buffer_name = argv[i++];

    // If you want to init the file system, do it
    if (initialize_file_system) {
        int block_size = 0;
        int blocks_count = 0;

        // If show_interactive_shell is TRUE, init the file system in shell mode
        if (show_interactive_shell)
            ask_for_init_params(&block_size, &blocks_count);
        else {
            #define INIT_ARGS_ERROR() { help_init(); return 1; }

            // Get the blocks count
            if (i == argc) INIT_ARGS_ERROR();
            if (strcmp(argv[i], "blocks") != 0)
                INIT_ARGS_ERROR();
            if (++i == argc) INIT_ARGS_ERROR();
            blocks_count = atoi(argv[i]);

            // And size
            if (++i == argc) INIT_ARGS_ERROR();
            if (strcmp(argv[i], "size") != 0)
                INIT_ARGS_ERROR();
            if (++i == argc) INIT_ARGS_ERROR();
            block_size = atoi(argv[i]);
            if (++i != argc) INIT_ARGS_ERROR();

            if (block_size < 0 || blocks_count < 0 ||
                block_size % 32 != 0 || blocks_count % 32 != 0)
                INIT_ARGS_ERROR();
        }

        // Initialize the file system
        FatResult res = fat_init(buffer_name, block_size, blocks_count);
        if (res != OK) {
            printf("Error initializing the file system: %s\n", fat_result_string(res));
            return 1;
        } else {
            printf("Successfully initialized FAT FS \"%s\" with %d blocks of %d bytes\n", 
                buffer_name, blocks_count, block_size);
            printf("Total disk size: %ld bytes\n",
                blocks_count * (block_size + sizeof(int)) + blocks_count/8 + sizeof(FatHeader));
            return 0;
        }

        if (show_interactive_shell)
            return run_shell(buffer_name);
        else
            return 0;
    }

    // If you are in interactive mode, show the shell
    if (show_interactive_shell)
        return run_shell(buffer_name);

    // If there are no commands to run
    if (i == argc) {
        help();
        return 0;
    }

    char *command[MAX_COMMAND_ARGUMENTS] = {0};

    // Get the commands
    int j = 0;
    while (i < argc && j < MAX_COMMAND_ARGUMENTS)
        command[j++] = argv[i++];
    if (i != argc) {
        printf("Too many arguments!\n");
        return 1;
    }

    // Open the file system
    FatFs *fs;
    FatResult res = fat_open(&fs, buffer_name);
    if (res != OK) {
        printf("Error opening the file system: %s\n", fat_result_string(res));
        return 1;
    }
    parse_command(fs, command);

    // Close the file system
    res = fat_close(fs);
    if (res != OK) {
        printf("Error closing the file system: %s\n", fat_result_string(res));
        return 1;
    }


    return 0;
}
