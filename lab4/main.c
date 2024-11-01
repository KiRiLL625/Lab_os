#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

mode_t convert_numeric_mode(const char *mode_str) {
    mode_t mode = 0;

    if (strlen(mode_str) != 3) {
        fprintf(stderr, "Invalid numeric mode format\n");
        exit(1);
    }

    mode = ((mode_str[0] - '0') * 64) + ((mode_str[1] - '0') * 8) + (mode_str[2] - '0');
    return mode;
}

void set_symbolic_mode(const char *mode_str, mode_t *current_mode) {
    char ugo[5];
    mode_t add_mask = 0;
    mode_t remove_mask = 0;

    int i = 0;
    while (mode_str[i]) {
        int who_index = 0;
        while (mode_str[i] == 'u' || mode_str[i] == 'g' || mode_str[i] == 'o' || mode_str[i] == 'a') {
            ugo[who_index++] = mode_str[i];
            i++;
        }

        if (strlen(ugo) == 0) {
            ugo[who_index++] = 'a';
        }

        ugo[who_index] = '\0';

        if (mode_str[i] == '+' || mode_str[i] == '-') {
            char op = mode_str[i];
            i++;

            while (mode_str[i] && (mode_str[i] == 'r' || mode_str[i] == 'w' || mode_str[i] == 'x')) {
                switch (mode_str[i]) {
                    case 'r':
                        for (int j = 0; j < who_index; j++) {
                            if (ugo[j] == 'u') add_mask |= S_IRUSR;
                            if (ugo[j] == 'g') add_mask |= S_IRGRP;
                            if (ugo[j] == 'o') add_mask |= S_IROTH;
                            if (ugo[j] == 'a') {
                                add_mask |= S_IRUSR | S_IRGRP | S_IROTH;
                            }
                        }
                        if (op == '-') remove_mask |= add_mask;
                        break;
                    case 'w':
                        for (int j = 0; j < who_index; j++) {
                            if (ugo[j] == 'u') add_mask |= S_IWUSR;
                            if (ugo[j] == 'g') add_mask |= S_IWGRP;
                            if (ugo[j] == 'o') add_mask |= S_IWOTH;
                            if (ugo[j] == 'a') {
                                add_mask |= S_IWUSR | S_IWGRP | S_IWOTH;
                            }
                        }
                        if (op == '-') remove_mask |= add_mask;
                        break;
                    case 'x':
                        for (int j = 0; j < who_index; j++) {
                            if (ugo[j] == 'u') add_mask |= S_IXUSR;
                            if (ugo[j] == 'g') add_mask |= S_IXGRP;
                            if (ugo[j] == 'o') add_mask |= S_IXOTH;
                            if (ugo[j] == 'a') {
                                add_mask |= S_IXUSR | S_IXGRP | S_IXOTH;
                            }
                        }
                        if (op == '-') remove_mask |= add_mask;
                        break;
                    default:
                        fprintf(stderr, "Invalid permission flag: %c\n", mode_str[i]);
                        exit(EXIT_FAILURE);
                }
                i++;
            }

            if (op == '+') {
                *current_mode |= add_mask;
            } else if (op == '-') {
                *current_mode &= ~remove_mask;
            }

            add_mask = 0;
            remove_mask = 0;
        } else {
            fprintf(stderr, "Invalid operator: %c\n", mode_str[i]);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <mode> <file>\n", argv[0]);
        return 1;
    }

    const char *mode_str = argv[1];
    const char *file_name = argv[2];

    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0) {
        perror("stat");
        return 2;
    }

    mode_t mode = file_stat.st_mode;

    if (mode_str[0] >= '0' && mode_str[0] <= '9') {
        mode = convert_numeric_mode(mode_str);
    } else {
        set_symbolic_mode(mode_str, &mode);
    }

    if (chmod(file_name, mode) < 0) {
        perror("chmod");
        return 3;
    }

    return 0;
}
