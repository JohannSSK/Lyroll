#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../libs/cJSON/cJSON.h"
#include <unistd.h>

char* SeparateWords(char** LyricsLines, int lineIndex, int wordIndex) {
    char* line = LyricsLines[lineIndex];

    // Skip timestamp
    char* start = line;
    if (line[0] == '[') {
        char* end = strchr(line, ']');
        if (end) start = end + 1;
    }
    while (*start == ' ') start++;
    if (*start == '\0') {
        printf("ERROR: Line %d is empty in SeparateWords\n", lineIndex);
        exit(1);
    }

    // Copy to mutable buffer
    char buffer[512];
    strcpy(buffer, start);

    // Split by spaces
    int count = 0;
    char* token = strtok(buffer, " ");
    while (token != NULL) {
        if (count == wordIndex) {
            char* result = malloc(strlen(token) + 1);
            if (!result) {
                printf("ERROR: malloc failed in SeparateWords for line %d, word %d\n", lineIndex, wordIndex);
                exit(1);
            }
            strcpy(result, token);
            return result;
        }
        count++;
        token = strtok(NULL, " ");
    }

    printf("ERROR: Word %d not found in line %d in SeparateWords\n", wordIndex, lineIndex);
    exit(1);
}

void FlushLineBuffer(char* Buffer, char* String) {
    if (String == NULL) {
        Buffer[0] = '\0';
        return;
    }

    strcpy(Buffer, String);
    strcat(Buffer, " ");
}

int CalculateWordsPerLineHuman(char* line) {
    char* start = line;
    if (line[0] == '[') {
        start = strchr(line, ']');
        if (start) {
            start++;
        } else {
            start = line;
        }
    }

    if (start == NULL || start[0] == '\0' || start[0] == ' ') return 0;

    int words = 1;
    for (int i = 0; start[i] != '\0'; i++) {
        if (start[i] == ' ') words++;
    }
    return words;
}

long int GetSongDuration(char* LyricsPath) {
    FILE* file = fopen(LyricsPath, "r");
    if (!file) {
        printf("Failed to open file by function GetSongDuration: %s\n", LyricsPath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);

    cJSON* json = cJSON_Parse(content);
    free(content);
    if (!json) {
        printf("Failed to parse JSON in GetSongDuration\n");
        return 0;
    }

    cJSON* durationObj = cJSON_GetObjectItem(json, "duration");
    if (!durationObj) {
        printf("No duration field in JSON\n");
        cJSON_Delete(json);
        return 0;
    }

    double durationSeconds = cJSON_GetNumberValue(durationObj);
    cJSON_Delete(json);

    return (long int)(durationSeconds * 1000);
}

int CountLines(char* LyricsPath){
    FILE* file = fopen(LyricsPath, "r");
    if (!file){
        printf("Failed to open file by function CountLines: %s\n", LyricsPath);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, 1, size, file);
    content[size] = '\0';
    fclose(file);

    cJSON* json = cJSON_Parse(content);
    free(content);
    if (!json){
        printf("Failed to parse JSON: %s by function CountLines\n", cJSON_GetErrorPtr());
        return 0;
    }

    cJSON* synced = cJSON_GetObjectItem(json, "syncedLyrics");
    char* lyrics = cJSON_GetStringValue(synced);

    if (!lyrics){
        printf("Failed to get lyrics: %s by function CountLines\n", cJSON_GetErrorPtr());
        cJSON_Delete(json);
        return 0;
    }

    int lines = 0;
    for (int i = 0; i < strlen(lyrics); i++){
        if (lyrics[i] == '\n'){
            lines++;
        }
    }
    cJSON_Delete(json);
    return lines + 1;
}

char** SeparateLyricsLines(char* LyricsPath, int TotalLinesHuman){
    FILE* file = fopen(LyricsPath, "r");
    if (!file){
        printf("Failed to open file by function CountLines: %s\n", LyricsPath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = malloc(size + 1);
    fread(content, sizeof(char), size, file);
    content[size] = '\0';
    fclose(file);

    cJSON* json = cJSON_Parse(content);
    free(content);
    if (!json){
        printf("Failed to parse JSON: %s by function SeparateLyricsLines\n", cJSON_GetErrorPtr());
        return NULL;
    }

    cJSON* synced = cJSON_GetObjectItem(json, "syncedLyrics");
    char* lyrics = cJSON_GetStringValue(synced);

    if (!lyrics){
        printf("Failed to get lyrics: %s by function SeparateLyricsLines\n", cJSON_GetErrorPtr());
        cJSON_Delete(json);
        return NULL;
    }

    char** lines = malloc((TotalLinesHuman + 1) * sizeof(char*));
    if (!lines) {
        cJSON_Delete(json);
        return NULL;
    }

    char* buffer = malloc(strlen(lyrics) + 1);
    if (!buffer) {
        free(lines);
        cJSON_Delete(json);
        return NULL;
    }
    strcpy(buffer, lyrics);

    int index = 0;
    char* token = strtok(buffer, "\n");
    while (token != NULL && index < TotalLinesHuman){
        lines[index] = malloc(strlen(token) + 1);
        if (!lines[index]) {
            // Clean up already allocated lines
            for (int j = 0; j < index; j++) {
                free(lines[j]);
            }
            free(lines);
            free(buffer);
            cJSON_Delete(json);
            return NULL;
        }
        strcpy(lines[index], token);
        token = strtok(NULL, "\n");
        index++;
    }
    lines[index] = NULL;

    free(buffer);
    cJSON_Delete(json);
    return lines;
}
