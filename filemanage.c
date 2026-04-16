#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define BUFFER_SIZE 1048576

void cmd_list(char *output, size_t outsz){
    DIR *d = opendir("server_files");
    if (!d) { mkdir("server_files", 0755); d = opendir("server_files");}
    struct dirent *entry;
    output[0] = '\0';

    while ((entry = readdir(d)) !=NULL){
        if (entry->d_name[0] == '.') continue;
        strncat (output, entry->d_name, outsz - strlen(output) - 2);
        strncat(output, "\n", outsz - strlen(output) - 1);

    }
    closedir(d);
    if (output[0] == '\0') strncpy(output, "(ska file)", outsz - 1);
    output[outsz - 1] = '\0';
}