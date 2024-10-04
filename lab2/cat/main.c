#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

void print_file(const char *filename, bool number_lines, bool number_nonblank, bool show_ends) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("mycat: No such file or directory\n");
        return;
    }

    int ch;
    int line_number = 1;
    bool at_beginning_of_line = true;

    while(ch = fgetc(file), ch != EOF) {
        if (at_beginning_of_line) {
            if (number_lines) {
                if(number_nonblank && ch != '\n') {
                    printf("%6d  ", line_number);
                    line_number++;
                }
                else if (!number_nonblank) {
                    printf("%6d  ", line_number);
                    line_number++;
                }
            }
            at_beginning_of_line = false;
        }
        if (ch == '\n') {
            if (show_ends) {
                putchar('$');
            }
            at_beginning_of_line = true;
        }
        putchar(ch);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    int opt;
    bool number_lines = false;
    bool number_nonblank = false;
    bool show_ends = false;

    while ((opt = getopt(argc, argv, "nbsE")) != -1) {
        switch (opt) {
            case 'n':
                number_lines = true;
                break;
            case 'b':
                number_lines = true;
                number_nonblank = true;
                break;
            case 'E':
                show_ends = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n] [-b] [-E] file...\n", argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected argument after options\n");
        return 1;
    }

    for (; optind < argc; optind++) {
        print_file(argv[optind], number_lines, number_nonblank, show_ends);
    }

    return 0;
}