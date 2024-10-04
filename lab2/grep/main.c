#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED "\x1B[31m"
#define RESET "\x1B[0m"

void grep_command(const char *filename, const char *pattern) {
    FILE *file = NULL;
    if(filename) {
        file = fopen(filename, "r");
        if (!file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, (filename) ? file: stdin)) != -1) {
        char *match = strstr(line, pattern);
        if (match) {
            printf("%.*s" RED "%s" RESET "%s", (int)(match - line), line, pattern, match + strlen(pattern));
        }
    }

    free(line);
    if(filename)
        fclose(file);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s pattern 'file' or "
                        "Usage: command | %s pattern\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *pattern = argv[1];
    const char *filename = (argc > 2) ? argv[2] : NULL;
    grep_command(filename, pattern);
    return 0;
}