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

void cmd_read(const char *filename, char *output, size_t outsz){
    char path[300];
    snprintf(path, sizeof(path), "server_files/%s", filename);
    FILE *f = fopen(path, "r");
    if (!f){
        strncpy(output, "[gabim] file nuk u gjet.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    size_t n = fread(output, 1, outsz -1, f);
    output[n] = '\0';
    fclose(f);
}

void cmd_delete(const char *filename, char *output, size_t outsz){
    char path[300];
    snprintf(path, sizeof(path), "server_files/%s", filename);
    if(remove(path) == 0)
    strncpy(output, "[ok] file u fshi.",outsz - 1);
    elsestrncpy(output, "[gabim] nuk u fshi dot file.", outsz - 1);
    output[outsz - 1] = '\0';
}

void cmd_search(const char *keyword, char *output, size_t outsz){
    DIR *d = opendir("server_files");
    if(!d){
        strncpy(output,  "[gabim] server_files nuk u gjet.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    struct dirent *entry;
    output[0] = '\0';
    while ((entry = readdir(d)) !=NULL){
        if (entry->d_name[0] == '.') continue;
        if(strst(entry->d_name, keyword)) {
            strncat(output, entry->d_name, outsz - strlen(output) - 2);
            strncat(output, "\n", outsz - strlen(output) - 1);
        }
    }

    closedir(d);
    if (output[0] == '\0') strncpy(output, "(nuk u gjeten rezultate)", outsz - 1);
    output[outsz - 1] = '\0';
}