#include <stdio.h>
#include <stdlib.h>
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
    else strncpy(output, "[gabim] nuk u fshi dot file.", outsz - 1);
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
        if(strstr(entry->d_name, keyword)) {
            strncat(output, entry->d_name, outsz - strlen(output) - 2);
            strncat(output, "\n", outsz - strlen(output) - 1);
        }
    }

    closedir(d);
    if (output[0] == '\0') strncpy(output, "(nuk u gjeten rezultate)", outsz - 1);
    output[outsz - 1] = '\0';
}

void cmd_info(const char *filename, char *output, size_t outsz){
    char path[300];
    snprintf(path, sizeof(path), "server_files/%s", filename);
    struct stat st;
    if (stat(path, &st) !=0){
        strncpy(output,"[gabim] file nuk u gjet.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    char mtime_buf[64], ctime_buf[64];
    strftime(mtime_buf, sizeof(mtime_buf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
    strftime(ctime_buf, sizeof(ctime_buf), "%Y-%m-%d %H:%M:%S", localtime(&st.st_ctime));
    snprintf(output,outsz,
    "emri:                %s\n"
    "madhesia:            %ld bytes\n"
    "modifikuar:          %s\n"
    "metadata ndryshuar:  %s",
    filename, (long)st.st_size, mtime_buf, ctime_buf);
}

void cmd_upload(const char *filename, const char *content, char *output, size_t outsz){
    mkdir("server_files", 0755);
    char path[300];
    snprintf(path, sizeof(path), "server_files/%s", filename);
    FILE *f = fopen(path, "w");
    if(!f){
        strncpy(output,"[gabum] nuk u ruajt dot file.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    fprintf(f, "%s", content);
    fclose(f);
    strncpy(output, "[ok] file u ngarkua me sukses.", outsz - 1);
    output[outsz - 1] = '\0';
}

void cmd_download(const char *filename, char *output, size_t outsz){
    char path[300];
    snprintf(path, sizeof(path), "server_files/%s", filename);
    FILE *f = fopen(path, "r");
    if (!f){
        strncpy(output, "[gabim] file nuk u gjet.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    size_t capacity = BUFFER_SIZE - 300;
    char *content = malloc(capacity);
    if (!content) {
        fclose(f);
        strncpy(output, "[gabim] nuk ka memorie te mjaftueshme.", outsz - 1);
        output[outsz - 1] = '\0';
        return;
    }

    size_t n = fread(content, 1, capacity - 1, f);
    content[n] = '\0';
    fclose(f);
    snprintf(output, outsz, "FILE_DATA:%s:%s", filename,content);
    free(content);
    
}