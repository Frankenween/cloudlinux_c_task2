#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>

#define W_NOREC (1 << 0)

struct walk_state {
    int dirfd;
    unsigned int depth;
    long flags;
};

void process_dirent(const struct dirent *entry, struct walk_state state) {
    printf("%*s%s\n", state.depth * 4, "", entry->d_name);
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
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        process_dirent(entry, state);
        if (entry->d_type == DT_DIR && (state.flags & W_NOREC) == 0) {
            struct walk_state new_state = {
                    .dirfd = current_dir_fd,
                    .depth = state.depth + 1,
                    .flags = state.flags
            };
            exit_code |= walk_tree(entry->d_name, new_state);
        }
    }

    CLOSE_DIR:
    closedir(current_dir);
    CLOSE_FILE:
    close(directory_fd);
    DONE:
    return exit_code;
}

int main(int argc, char* argv[]) {
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        perror("Couldn't get current working directory");
        return 2;
    }
    struct walk_state initial_state = {
            .dirfd = 0,
            .depth = 0,
            .flags = 0
    };
    int result = walk_tree(cwd, initial_state);
    free(cwd);
    return result;
}
