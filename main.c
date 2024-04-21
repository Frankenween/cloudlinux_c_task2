#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>

static int recursive_walk = 1; // Need to visit subfolders?

enum skip_policy {
    PRINT_ALL,        // List all files in directory
    PRINT_ALMOST_ALL, // Skip '.' and '..' files
    SKIP_HIDDEN       // Skip files and directories starting with '.'
} skip_rule = SKIP_HIDDEN;

enum quote_policy {
    NO_QUOTES,    // Just print names
    QUOTE_NEEDED, // Write names quoted if they contain spaces
    QUOTE_ALL     // Write every file name in quotes
} quote_rule = QUOTE_NEEDED;

struct walk_state {
    int dirfd;
    unsigned int depth;
};

int is_current_dir_or_parent(const char *name) {
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

// Do something with an entry in directory
// state represents current dfs state - directory file descriptor and depth
void process_dirent(const struct dirent *entry, struct walk_state state) {
    if (skip_rule == SKIP_HIDDEN && entry->d_name[0] == '.') {
        // Should not look on hidden files
        return;
    }
    if (skip_rule == PRINT_ALMOST_ALL && is_current_dir_or_parent(entry->d_name)) {
        return;
    }

    printf("%*s", state.depth * 4, ""); // print padding

    if (quote_rule == QUOTE_ALL ||
        (quote_rule == QUOTE_NEEDED && strchr(entry->d_name, ' ') != NULL)) {
        printf("\"%s\"", entry->d_name);
    } else {
        printf("%s", entry->d_name);
    }
    printf("\n");
}

int filter_child_dir(const struct dirent *entry, struct walk_state state) {
    if (is_current_dir_or_parent(entry->d_name)) {
        // Need to ignore these directories, or it will be an infinite loop
        return 1;
    }
    if (skip_rule == SKIP_HIDDEN && entry->d_name[0] == '.') {
        return 1;
    }
    return 0;
}

int walk_tree(const char *name, struct walk_state state) {
    int exit_code = 0;
    int directory_fd = openat(state.dirfd, name, O_DIRECTORY);
    if (directory_fd == -1) {
        perror("Failed to get directory file descriptor");
        exit_code = 1; // Failed to open
        goto DONE;
    }
    DIR *current_dir = fdopendir(directory_fd);
    if (current_dir == NULL) {
        perror("Couldn't open directory via fdopendir");
        exit_code = 1;
        goto CLOSE_FILE;
    }
    int current_dir_fd = dirfd(current_dir);
    if (current_dir_fd == -1) {
        perror("Couldn't get fd from dirfd");
        exit_code = 1;
        goto CLOSE_DIR;
    }
    errno = 0;
    struct dirent *entry;
    while ((entry = readdir(current_dir)) != NULL) {
        process_dirent(entry, state);
        if (entry->d_type == DT_DIR && recursive_walk) {
            if (filter_child_dir(entry, state)) {
                continue;
            }
            struct walk_state new_state = {
                    .dirfd = current_dir_fd,
                    .depth = state.depth + 1,
            };
            exit_code |= walk_tree(entry->d_name, new_state);
        }
        errno = 0; // Maybe some errors from previous calls, need to flush them before new readdir call
    }
    if (errno != 0) {
        perror("Failed to iterate over directory");
        exit_code = 1;
    }

    CLOSE_DIR:
    closedir(current_dir);
    CLOSE_FILE:
    close(directory_fd);
    DONE:
    return exit_code;
}

int update_flags(const char *arg) {
    if (strcmp(arg, "--no-req") == 0) {
        recursive_walk = 0;
    } else if (strcmp(arg, "--quote-all") == 0) {
        quote_rule = QUOTE_ALL;
    } else if (strcmp(arg, "--no-quotes") == 0) {
        quote_rule = NO_QUOTES; // Default option is W_QUOTE_NEEDED
    } else if (strcmp(arg, "--all") == 0) {
        skip_rule = PRINT_ALL;
    } else if (strcmp(arg, "--almost-all") == 0) {
        skip_rule = PRINT_ALMOST_ALL;
    } else {
        fprintf(stderr, "Unknown option '%s'\n", arg);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (update_flags(argv[i])) {
            // Found wrong flag, abort now
            return 1;
        }
    }
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("Couldn't get current working directory");
        return 2;
    }
    struct walk_state initial_state = {
            .dirfd = 0,
            .depth = 0,
    };
    int result = walk_tree(cwd, initial_state);
    free(cwd);
    return result;
}
