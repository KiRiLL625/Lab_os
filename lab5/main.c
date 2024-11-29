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
    printf("./archiver -i arch_name file1\n");
    printf("./archiver -e arch_name file1\n");
    printf("./archiver -s arch_name\n");
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
}

void extract_file_from_archive(const char *archive_name, const char *filename) {
    int archive_fd = open(archive_name, O_RDONLY);
    if (archive_fd < 0) {
        perror("Failed to open archive");
        exit(EXIT_FAILURE);
    }

    FileHeader header;
    while (read(archive_fd, &header, sizeof(FileHeader)) == sizeof(FileHeader)) {
        if (strcmp(header.filename, filename) == 0) {
            int file_fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, header.file_stat.st_mode);
            if (file_fd < 0) {
                perror("Failed to create output file");
                close(archive_fd);
                exit(EXIT_FAILURE);
            }

            if (fchmod(file_fd, header.file_stat.st_mode) < 0) {
                perror("Failed to save file permissions");
                close(file_fd);
                close(archive_fd);
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
                    exit(EXIT_FAILURE);
                }
                if (write(file_fd, buffer, bytes_read) != bytes_read) {
                    perror("Failed to write to output file");
                    close(file_fd);
                    close(archive_fd);
                    exit(EXIT_FAILURE);
                }
                bytes_to_read -= bytes_read;
            }

            close(file_fd);
            close(archive_fd);
            return;
        } else {
            lseek(archive_fd, header.filesize, SEEK_CUR);
        }
    }

    fprintf(stderr, "File not found in archive\n");
    close(archive_fd);
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

    if (argc < 2) {
        print_help();
        return EXIT_FAILURE;
    }

    while ((opt = getopt(argc, argv, "i:e:sh")) != -1) {
        switch (opt) {
            case 'i':
                if (optind < argc) {
                    add_file_to_archive(optarg, argv[optind]);
                }
                else {
                    fprintf(stderr, "Expected argument after options\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'e':
                if (optind < argc) {
                    extract_file_from_archive(optarg, argv[optind]);
                }
                else {
                    fprintf(stderr, "Expected argument after options\n");
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                if (optind < argc) {
                    print_archive_stat(argv[optind]);
                }
                else {
                    fprintf(stderr, "Expected argument after options\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'h':
            default:
                print_help();
                return EXIT_FAILURE;
        }
    }

    return 0;
}
