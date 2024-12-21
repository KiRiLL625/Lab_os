#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define HEADER_SIZE 256

typedef struct {
    char filename[HEADER_SIZE];
    off_t filesize;
    struct stat file_stat;
} FileHeader;

void print_help() {
    printf("Usage:\n");
    printf("./archiver arch_name -i file1\n");
    printf("./archiver arch_name -e file1\n");
    printf("./archiver arch_name -s\n");
    printf("./archiver -h\n");
}

void add_file_to_archive(const char *archive_name, const char *filename) {
    int archive_fd = open(archive_name, O_RDWR | O_CREAT, 0644);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        exit(EXIT_FAILURE);
    }

    FileHeader header;
    while (read(archive_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        if (strcmp(header.filename, filename) == 0) {
            fprintf(stderr, "File '%s' already exists in the archive\n", filename);
            close(archive_fd);
            return;
        }
        lseek(archive_fd, header.filesize, SEEK_CUR);
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open input file");
        close(archive_fd);
        exit(EXIT_FAILURE);
    }

    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Failed to get file stats");
        close(file_fd);
        close(archive_fd);
        exit(EXIT_FAILURE);
    }

    strncpy(header.filename, filename, HEADER_SIZE);
    header.filesize = file_stat.st_size;
    header.file_stat = file_stat;

    if (write(archive_fd, &header, sizeof(FileHeader)) != sizeof(FileHeader)) {
        perror("Failed to write header to archive");
        close(file_fd);
        close(archive_fd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        if (write(archive_fd, buffer, bytes_read) != bytes_read) {
            perror("Failed to write file to archive");
            close(file_fd);
            close(archive_fd);
            exit(EXIT_FAILURE);
        }
    }

    close(file_fd);
    close(archive_fd);

    if (remove(filename) != 0) {
        perror("Failed to delete the original file");
        exit(EXIT_FAILURE);
    }
}

void extract_file_from_archive(const char *archive_name, const char *filename) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        exit(EXIT_FAILURE);
    }

    char temp_archive_name[] = "temp_archiveXXXXXX";
    int temp_archive_fd = mkstemp(temp_archive_name);
    if (temp_archive_fd < 0) {
        perror("Failed to create temporary archive");
        close(archive_fd);
        exit(EXIT_FAILURE);
    }

    FileHeader header;
    int file_found = 0;
    while (read(archive_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        if (strcmp(header.filename, filename) == 0) {
            int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, header.file_stat.st_mode);
            if (file_fd < 0) {
                perror("Failed to create output file");
                close(archive_fd);
                close(temp_archive_fd);
                exit(EXIT_FAILURE);
            }

            if (fchmod(file_fd, header.file_stat.st_mode) < 0) {
                perror("Failed to save file permissions");
                close(file_fd);
                close(archive_fd);
                close(temp_archive_fd);
                exit(EXIT_FAILURE);
            }

            char buffer[1024];
            ssize_t bytes_to_read = header.filesize;
            while (bytes_to_read > 0) {
                ssize_t bytes_read = read(archive_fd, buffer, sizeof(buffer) < bytes_to_read ? sizeof(buffer) : bytes_to_read);
                if (bytes_read < 0) {
                    perror("Failed to read from archive");
                    close(file_fd);
                    close(archive_fd);
                    close(temp_archive_fd);
                    exit(EXIT_FAILURE);
                }
                if (write(file_fd, buffer, bytes_read) != bytes_read) {
                    perror("Failed to write to output file");
                    close(file_fd);
                    close(archive_fd);
                    close(temp_archive_fd);
                    exit(EXIT_FAILURE);
                }
                bytes_to_read -= bytes_read;
            }

            close(file_fd);
            file_found = 1;
        } else {
            if (write(temp_archive_fd, &header, sizeof(FileHeader)) != sizeof(FileHeader)) {
                perror("Failed to write header to temporary archive");
                close(archive_fd);
                close(temp_archive_fd);
                exit(EXIT_FAILURE);
            }

            char buffer[1024];
            ssize_t bytes_to_read = header.filesize;
            while (bytes_to_read > 0) {
                ssize_t bytes_read = read(archive_fd, buffer, sizeof(buffer) < bytes_to_read ? sizeof(buffer) : bytes_to_read);
                if (bytes_read < 0) {
                    perror("Failed to read from archive");
                    close(archive_fd);
                    close(temp_archive_fd);
                    exit(EXIT_FAILURE);
                }
                if (write(temp_archive_fd, buffer, bytes_read) != bytes_read) {
                    perror("Failed to write to temporary archive");
                    close(archive_fd);
                    close(temp_archive_fd);
                    exit(EXIT_FAILURE);
                }
                bytes_to_read -= bytes_read;
            }
        }
    }

    close(archive_fd);
    close(temp_archive_fd);

    if (file_found) {
        if (remove(archive_name) < 0) {
            perror("Failed to remove old archive");
            exit(EXIT_FAILURE);
        }
        if (rename(temp_archive_name, archive_name) < 0) {
            perror("Failed to rename temporary archive");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "File not found in archive\n");
        remove(temp_archive_name);
    }
}

void print_archive_stat(const char *archive_name) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        exit(EXIT_FAILURE);
    }

    FileHeader header;
    while (read(archive_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        printf("File: %s, size: %ld bytes\n", header.filename, header.filesize);
        printf("File permissions: ");
        printf((header.file_stat.st_mode & S_IRUSR) ? "r" : "-");
        printf((header.file_stat.st_mode & S_IWUSR) ? "w" : "-");
        printf((header.file_stat.st_mode & S_IXUSR) ? "x" : "-");
        printf((header.file_stat.st_mode & S_IRGRP) ? "r" : "-");
        printf((header.file_stat.st_mode & S_IWGRP) ? "w" : "-");
        printf((header.file_stat.st_mode & S_IXGRP) ? "x" : "-");
        printf((header.file_stat.st_mode & S_IROTH) ? "r" : "-");
        printf((header.file_stat.st_mode & S_IWOTH) ? "w" : "-");
        printf((header.file_stat.st_mode & S_IXOTH) ? "x" : "-");
        printf("\n");
        lseek(archive_fd, header.filesize, SEEK_CUR);
    }

    close(archive_fd);
}

int main(int argc, char *argv[]) {
    int opt;
    const char *archive_name = NULL;
    const char *filename = NULL;
    char flag = 0;

    if (argc < 2) {
        print_help();
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "-h") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    archive_name = argv[1];

    while ((opt = getopt(argc - 1, argv + 1, "i:e:s")) != -1) {
        switch (opt) {
            case 'i':
                flag = 'i';
                filename = optarg;
                break;
            case 'e':
                flag = 'e';
                filename = optarg;
                break;
            case 's':
                flag = 's';
                break;
            default:
                print_help();
                return EXIT_FAILURE;
        }
    }

    if (!archive_name) {
        fprintf(stderr, "Archive name is required\n");
        print_help();
        return EXIT_FAILURE;
    }

    switch (flag) {
        case 'i':
            if (filename) {
                add_file_to_archive(archive_name, filename);
            } else {
                fprintf(stderr, "Expected file name after -i\n");
                return EXIT_FAILURE;
            }
            break;
        case 'e':
            if (filename) {
                extract_file_from_archive(archive_name, filename);
            } else {
                fprintf(stderr, "Expected file name after -e\n");
                return EXIT_FAILURE;
            }
            break;
        case 's':
            print_archive_stat(archive_name);
            break;
        default:
            print_help();
            return EXIT_FAILURE;
    }

    return 0;
}
