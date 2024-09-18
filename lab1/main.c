#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

int compare(const void* a, const void* b){
    return strcmp(((struct dirent*)a)->d_name, ((struct dirent*)b)->d_name);
}

void print_file_name(struct dirent* entry, struct stat* file_stat, char* path, bool long_format){
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s", path);

    if(S_ISLNK(file_stat->st_mode)){
        char target[1024];
        int len = readlink(file_path, target, sizeof(target));
        if(len == -1){
            perror("readlink");
            printf("%s", entry->d_name);
            return;
        }
        target[len] = '\0';

        struct stat target_stat;
        char target_path[1025];
        target_path[0] = '/';
        snprintf(target_path + 1, sizeof(target_path) - 1, "%s", target);
        lstat(target_path, &target_stat);
        printf("\033[1;36m%s\033[0m", entry->d_name);
        if(long_format){
            printf(" -> ");
            if(S_ISDIR(target_stat.st_mode)){
                printf("\033[1;34m%s\033[0m", target);
            }
            else if(target_stat.st_mode & S_IXUSR){
                printf("\033[1;32m%s\033[0m", target);
            }
            else{
                printf("%s", target);
            }
        }
    }
    else if(S_ISDIR(file_stat->st_mode)){
        printf("\033[1;34m%s\033[0m", entry->d_name);
    }
    else if(file_stat->st_mode & S_IXUSR){
        printf("\033[1;32m%s\033[0m", entry->d_name);
    }
    else{
        printf("%s", entry->d_name);
    }
    printf("  ");
}

void print_file_info(struct dirent* entry, char* path, int max_size_len, int max_link_len,
                     int max_user_len, int max_group_len){
    struct stat file_stat;
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
    lstat(file_path, &file_stat);

    printf((S_ISDIR(file_stat.st_mode)) ? "d" : "-"); // Проверка на директорию
    printf((file_stat.st_mode & S_IRUSR) ? "r" : "-"); // Право на чтение
    printf((file_stat.st_mode & S_IWUSR) ? "w" : "-"); // Право на запись
    printf((file_stat.st_mode & S_IXUSR) ? "x" : "-"); // Право на исполнение
    printf((file_stat.st_mode & S_IRGRP) ? "r" : "-"); // Право на чтение группы
    printf((file_stat.st_mode & S_IWGRP) ? "w" : "-"); // Право на запись группы
    printf((file_stat.st_mode & S_IXGRP) ? "x" : "-"); // Право на исполнение группы
    printf((file_stat.st_mode & S_IROTH) ? "r" : "-"); // Право на чтение остальных
    printf((file_stat.st_mode & S_IWOTH) ? "w" : "-"); // Право на запись остальных
    printf((file_stat.st_mode & S_IXOTH) ? "x" : "-"); // Право на исполнение остальных

    printf("%*ld ", max_link_len + 1, file_stat.st_nlink);
    struct passwd* pw = getpwuid(file_stat.st_uid);
    if(pw != NULL){
        printf("%*s ", max_user_len, pw->pw_name);
    }
    else{
        printf("%*d ", max_user_len, file_stat.st_uid);
    }
    struct group* gr = getgrgid(file_stat.st_gid);
    if(gr != NULL){
        printf("%*s ", max_group_len, gr->gr_name);
    }
    else{
        printf("%*d ", max_group_len, file_stat.st_gid);
    }
    printf("%*ld ", max_size_len, file_stat.st_size);

    char time_str[20];
    struct tm *tm_info;
    time_t now;
    time(&now);
    tm_info = localtime(&file_stat.st_mtime);
    if(file_stat.st_mtime > now - 6*30*24*60*60){ // 6 месяцев
        strftime(time_str, 20, "%b %d %H:%M", tm_info);
    }
    else{
        strftime(time_str, 20, "%b %d  %Y", tm_info);
    }
    printf("%s ", time_str);

    print_file_name(entry, &file_stat, file_path, 1);
}

long calculate_memory_usage_blocks(char* path, bool show_hidden){
    DIR* dir = opendir(path);
    if(dir == NULL){
        printf("Error: Unable to open directory\n");
        return 0;
    }

    int count = 0;
    struct dirent* entry;

    long memory_usage = 0;
    while((entry = readdir(dir)) != NULL){
        if(!show_hidden && entry->d_name[0] == '.'){
            continue;
        }
        count++;

        struct stat file_stat;
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
        lstat(file_path, &file_stat);

        memory_usage += file_stat.st_blocks;
    }
    closedir(dir);
    return memory_usage / 2;
}

void list_files(char* path, bool show_hidden, bool long_format){
    DIR* dir = opendir(path);
    if(dir == NULL){
        printf("Error: Unable to open directory\n");
        return;
    }

    struct dirent* entries = NULL;
    int count = 0;
    struct dirent* entry;

    int max_size_len = 0;
    int max_link_len = 0;
    int max_user_len = 0;
    int max_group_len = 0;

    while((entry = readdir(dir)) != NULL){
        if(!show_hidden && entry->d_name[0] == '.'){
            continue;
        }

        entries = realloc(entries, (count + 1) * sizeof(struct dirent));
        memcpy(&entries[count], entry, sizeof(struct dirent));
        count++;

        struct stat file_stat;
        char file_path[1024];
        snprintf(file_path, sizeof(file_path), "%s/%s", path, entry->d_name);
        stat(file_path, &file_stat);

        int size_len = snprintf(NULL, 0, "%ld", file_stat.st_size);
        int link_len = snprintf(NULL, 0, "%ld", file_stat.st_nlink);

        if(size_len > max_size_len){
            max_size_len = size_len;
        }

        if(link_len > max_link_len){
            max_link_len = link_len;
        }

        struct passwd* pw = getpwuid(file_stat.st_uid);
        if(pw != NULL){
            int user_len = strlen(pw->pw_name);
            if(user_len > max_user_len){
                max_user_len = user_len;
            }
        }
        else{
            int user_len = snprintf(NULL, 0, "%d", file_stat.st_uid);
            if(user_len > max_user_len){
                max_user_len = user_len;
            }
        }

        struct group* gr = getgrgid(file_stat.st_gid);
        if(gr != NULL){
            int group_len = strlen(gr->gr_name);
            if(group_len > max_group_len){
                max_group_len = group_len;
            }
        }
        else{
            int group_len = snprintf(NULL, 0, "%d", file_stat.st_gid);
            if(group_len > max_group_len){
                max_group_len = group_len;
            }
        }
    }
    closedir(dir);

    qsort(entries, count, sizeof(struct dirent), compare);

    for(int i = 0; i < count; i++){
        if(long_format){
            print_file_info(&entries[i], path, max_size_len, max_link_len,
                            max_user_len, max_group_len);
            printf("\n");
        }
        else{
            struct stat file_stat;
            char file_path[1024];
            snprintf(file_path, sizeof(file_path), "%s/%s", path, entries[i].d_name);
            lstat(file_path, &file_stat);
            print_file_name(&entries[i], &file_stat, file_path, 0);
        }
    }
    if(!long_format){
        printf("\n");
    }
    free(entries);
}

int main(int argc, char** argv){
    char* path = ".";
    bool show_hidden = false;
    bool long_format = false;

    int opt;
    while((opt = getopt(argc, argv, "la")) != -1){
        switch(opt){
            case 'l':
                long_format = true;
                break;
            case 'a':
                show_hidden = true;
                break;
            default:
                printf("Error: Invalid flag\n");
                return 1;
        }
    }

    if(optind < argc){
        path = argv[optind];
    }

    if(long_format){
        printf("total %ld\n", calculate_memory_usage_blocks(path, show_hidden));
    }
    list_files(path, show_hidden, long_format);

    return 0;
}