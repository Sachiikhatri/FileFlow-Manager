#define _XOPEN_SOURCE 500  // Required for nftw
#define _GNU_SOURCE         // Required for some GNU-specific features
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>           // For nftw()function to traverse directories

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <errno.h>
#include <limits.h>

// Global variables to store command line arguments and counters
static char *g_command = NULL;    // Stores the command given by the user
static char *g_extension = NULL;  // Stores the file extension for filtering
static char *g_destination = NULL;// Stores the destination directory path
static char *g_root_dir = NULL;   // Stores the root directory path to process
static int g_file_count = 0;      // Counter to track number of files processed
static int g_dir_count = 0;       // Counter to track number of directories processed
static long g_total_size = 0;     // Total size of files counted
static int g_error_occurred = 0;  // Flag to track errors


// Function to validate file extension
// Only allows specific extensions (C, TXT, and PDF files)

int validate_extension(const char *ext) {
    if (!ext) return 1;// If no extension is provided, accept all files
    return (strcmp(ext, ".c") == 0 ||
            strcmp(ext, ".txt") == 0 ||
            strcmp(ext, ".pdf") == 0);// Only allow these extensions
}

// Function to validate directory path
// Ensures that the directory exists and is inside the user's home directory
int validate_directory(const char *path) {
    struct stat st;

    if (!path) {
        fprintf(stderr, "Error: Directory path is NULL\n");
        return 0;
    }

    if (strlen(path) > PATH_MAX) {
        fprintf(stderr, "Error: Path exceeds maximum length\n");
        return 0;
    }

    if (stat(path, &st) != 0) {
        fprintf(stderr, "Error: Cannot access '%s': %s\n", path, strerror(errno));
        return 0;
    }

    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a directory\n", path);
        return 0;
    }

    char *home = getenv("HOME");// Get the home directory of the user
    if (!home || strncmp(path, home, strlen(home)) != 0) {
        fprintf(stderr, "Error: Path must be under home directory\n");
        return 0;
    }

    return 1;// directory is valid
}

// Function to create directory if it doesn't exist
// It also ensures that parent directories are created first

int create_directory(const char *path) {
    if (!path) return 0;

    char *temp_path = strdup(path);
    if (!temp_path) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return 0;
    }

    char *dir = dirname(temp_path);//extract the parent dir path

    if (access(dir, F_OK) != 0) {//check if parent dir exists
        if (!create_directory(dir)) {
            free(temp_path);
            return 0;
        }
    }

    if (mkdir(path, 0777) != 0 && errno != EEXIST) {////create new dir
        fprintf(stderr, "Error creating directory '%s': %s\n", path, strerror(errno));
        free(temp_path);
        return 0;
    }

    free(temp_path);
    return 1;// Directory created successfully
}

// Function to check if a file has specific extension
// It compares the file extension with the given one
int has_extension(const char *filename, const char *ext) {
    if (!ext) return 1;  // If no extension specified, return true
    if (!filename) return 0;

    const char *file_ext = strrchr(filename, '.'); // Get the last occurrence of '.'
    return file_ext && strcmp(file_ext, ext) == 0;
}

// Function to safely copy file
// It reads data in chunks and writes it to the new file

int copy_file(const char *source_path, const char *dest_path) {
    FILE *source = NULL, *dest = NULL;// Buffer to read and write file data
    char buffer[8192];
    size_t bytes;
    int success = 0;

    source = fopen(source_path, "rb");// Open the source file for reading mode
    if (!source) {
        fprintf(stderr, "Error opening source file '%s': %s\n",
                source_path, strerror(errno));
        return 0;
    }

    dest = fopen(dest_path, "wb");// Open the destination file for writing mode
    if (!dest) {
        fprintf(stderr, "Error opening destination file '%s': %s\n",
                dest_path, strerror(errno));
        fclose(source);
        return 0;
    }

    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {// Copy the contents of the source file to the destination

        if (fwrite(buffer, 1, bytes, dest) != bytes) {
            fprintf(stderr, "Error writing to '%s': %s\n",
                    dest_path, strerror(errno));
            goto cleanup;
        }
    }

    if (ferror(source)) {
        fprintf(stderr, "Error reading from '%s': %s\n",
                source_path, strerror(errno));
        goto cleanup;
    }

    success = 1;// If all operations succeed, mark as successful


cleanup:
    fclose(source);
    fclose(dest);
    if (!success) {
        unlink(dest_path);  // Remove partially copied file
    }
    return success;
}

// Callback function for nftw()that walks through a directory tree
// and performs actions based on the command specified by the user
static int tree_walker(const char *path, const struct stat *sb,
                      int typeflag, struct FTW *ftwbuf) {
    char dest_path[PATH_MAX]; // Buffer to store destination path

    if (typeflag == FTW_DNR) { // Handle unreadable directories
        fprintf(stderr, "Warning: Cannot read directory '%s': %s\n",
                path, strerror(errno));
        g_error_occurred = 1;
        return 0;
    }

    if (typeflag == FTW_NS) {// Handle files that cannot be stat-ed
        fprintf(stderr, "Warning: Cannot stat '%s': %s\n",
                path, strerror(errno));
        g_error_occurred = 1;
        return 0;
    }

    switch(typeflag) { // Only process regular files and directories
        case FTW_F:  // Regular file
        case FTW_D:  // Directory
            break;
        default:
            fprintf(stderr, "Warning: Skipping special file '%s'\n", path);
            return 0;
    }

    if (strcmp(g_command, "-ls") == 0) { // Handle different commands
        printf("%s\n", path);
    }
    else if (strcmp(g_command, "-ext") == 0) {
        if (typeflag == FTW_F && has_extension(path, g_extension)) {
            printf("%s\n", path);// Print files matching a specific extension
        }
    }
    else if (strcmp(g_command, "-fc") == 0) {// Count files
        if (typeflag == FTW_F) g_file_count++;
    }
    else if (strcmp(g_command, "-dc") == 0) {// Count directorie
        if (typeflag == FTW_D) g_dir_count++;
    }
    else if (strcmp(g_command, "-fs") == 0) {
        if (typeflag == FTW_F) {
            if (sb->st_size < 0) { // Check for valid file size before adding to total
                fprintf(stderr, "Warning: Invalid file size for '%s'\n", path);
                g_error_occurred = 1;
            } else {
                g_total_size += sb->st_size;
            }
        }
    }
    else if (strcmp(g_command, "-cp") == 0) {
        char dest_path[PATH_MAX];
        const char *relative_path = path + strlen(g_root_dir);

        // Skip the leading slash in relative_path if present
        if (relative_path[0] == '/') {
            relative_path++;
        }

        // Construct destination path
        if (snprintf(dest_path, sizeof(dest_path), "%s/%s",
                    g_destination, relative_path) >= sizeof(dest_path)) {
            fprintf(stderr, "Error: Destination path too long for '%s'\n", path);
            g_error_occurred = 1;
            return 0;
        }

        if (typeflag == FTW_D) { // Create directory in destination
            // Create directory in destination
            if (!create_directory(dest_path)) {
                g_error_occurred = 1;
                return 0;
            }
        }
        else if (typeflag == FTW_F) {
            // Skip files with the specified extension
            if (g_extension && has_extension(path, g_extension)) {
                return 0;
            }

            // Copy the file
            if (!copy_file(path, dest_path)) {
                g_error_occurred = 1;
                return 0;
            }
        }
    }
    else if (strcmp(g_command, "-mv") == 0) {
        if (snprintf(dest_path, sizeof(dest_path), "%s/%s",
                    g_destination, path + strlen(dirname(strdup(path))) + 1) >= sizeof(dest_path)) {
            fprintf(stderr, "Error: Destination path too long for '%s'\n", path);
            g_error_occurred = 1;
            return 0;
        }

        if (typeflag == FTW_F) {
            // For files, use rename to move them
            if (rename(path, dest_path) != 0) {
                // If rename fails (e.g., across filesystems), fallback to copy and delete
                if (!copy_file(path, dest_path)) {
                    g_error_occurred = 1;
                    return 0;
                }
            }
        }
        else if (typeflag == FTW_D) {
            // For directories, create in destination
            if (strcmp(path, g_root_dir) != 0) {  // Skip creating root dir itself
                if (!create_directory(dest_path)) {
                    g_error_occurred = 1;
                    return 0;
                }
            }
        }
    }
    else if (strcmp(g_command, "-del") == 0) {
        if (typeflag == FTW_F && has_extension(path, g_extension)) {
            if (unlink(path) != 0) {
                fprintf(stderr, "Error deleting '%s': %s\n", path, strerror(errno));
                g_error_occurred = 1;
                return 0;
            }
        }
    }

    return 0;
}

// Function to print program usage and supported commands
void print_usage(const char *program_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s -ls <root_dir>\n", program_name);
    fprintf(stderr, "  %s -ext <root_dir> <file_extension>\n", program_name);
    fprintf(stderr, "  %s -fc <root_dir>\n", program_name);
    fprintf(stderr, "  %s -dc <root_dir>\n", program_name);
    fprintf(stderr, "  %s -fs <root_dir>\n", program_name);
    fprintf(stderr, "  %s -cp <source_dir> <destination_dir> [file_extension]\n", program_name);
    fprintf(stderr, "  %s -mv <source_dir> <destination_dir>\n", program_name);
    fprintf(stderr, "  %s -del <root_dir> <file_extension>\n", program_name);
}

// Function to remove directory and its contents
int remove_directory(const char *path) {
    return nftw(path,
               (int (*)(const char *, const struct stat *, int, struct FTW *))remove,
               64,
               FTW_DEPTH | FTW_PHYS);  // FTW_DEPTH ensures bottom-up traversal
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    g_command = argv[1];
    g_root_dir = argv[2];

    // Validate command
    const char *valid_commands[] = {"-ls", "-ext", "-fc", "-dc", "-fs", "-cp", "-mv", "-del", NULL};
    int valid_command = 0;
    for (const char **cmd = valid_commands; *cmd; cmd++) {
        if (strcmp(g_command, *cmd) == 0) {
            valid_command = 1;
            break;
        }
    }

    if (!valid_command) {
        fprintf(stderr, "Error: Invalid command '%s'\n", g_command);
        print_usage(argv[0]);
        return 1;
    }

    // Validate root directory
    if (!validate_directory(g_root_dir)) {
        return 1;
    }

    // Validate and set up additional parameters
    if (strcmp(g_command, "-ext") == 0 || strcmp(g_command, "-del") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: File extension required\n");
            return 1;
        }
        g_extension = argv[3];
        if (!validate_extension(g_extension)) {
            fprintf(stderr, "Error: Invalid file extension '%s'\n", g_extension);
            return 1;
        }
    }
    else if (strcmp(g_command, "-cp") == 0 || strcmp(g_command, "-mv") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: Destination directory required\n");
            return 1;
        }
  g_destination = argv[3];

        // Validate destination directory
        struct stat st;
        if (stat(g_destination, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "Error: '%s' exists but is not a directory\n", g_destination);
                return 1;
            }
        } else if (errno == ENOENT) {
            if (!create_directory(g_destination)) {
                return 1;
            }
        } else {
            fprintf(stderr, "Error accessing '%s': %s\n", g_destination, strerror(errno));
            return 1;
        }

        if (argc > 4) {
            g_extension = argv[4];
            if (!validate_extension(g_extension)) {
                fprintf(stderr, "Error: Invalid file extension '%s'\n", g_extension);
                return 1;
            }
        }
    }

    // Traverse the directory tree
    if (nftw(g_root_dir, tree_walker, 20, FTW_PHYS) == -1) {
        fprintf(stderr, "Error traversing directory tree: %s\n", strerror(errno));
        return 1;
    }

    // For move command, delete the source directory after moving everything
    if (strcmp(g_command, "-mv") == 0 && !g_error_occurred) {
        if (remove_directory(g_root_dir) != 0) {
            fprintf(stderr, "Error removing source directory '%s': %s\n",
                    g_root_dir, strerror(errno));
            return 1;
        }
    }

    // Print results for counting operations
    if (strcmp(g_command, "-fc") == 0) {
        printf("Total files: %d\n", g_file_count);
    }
    else if (strcmp(g_command, "-dc") == 0) {
        printf("Total directories: %d\n", g_dir_count);
    }
    else if (strcmp(g_command, "-fs") == 0) {
        printf("Total size: %ld bytes\n", g_total_size);
    }

            return g_error_occurred ? 1 : 0;
}

// video link: https://drive.google.com/drive/folders/1qG7ci4oAv0w4LJXdNk98STQd11jyPLM4?usp=sharing 

