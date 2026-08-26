#include <stdio.h>
#include <string.h>
#include "functions.h"

void stats(FILE* file,char* line, char* id, char* artist, char* album, char* title, char* length){

    // Get stats
    printDebug("Gettings stats...\n");

    fgets(line, (MAX_LINE_LEN), file);
    sscanf(line, "[id: %[^]]]", id);

    fgets(line, (MAX_LINE_LEN), file);
    sscanf(line, "[ar: %[^]]]", artist);

    fgets(line, (MAX_LINE_LEN), file);
    sscanf(line, "[al: %[^]]]", album);

    fgets(line, (MAX_LINE_LEN), file);
    sscanf(line, "[ti: %[^]]]", title);

    fgets(line, (MAX_LINE_LEN), file);
    sscanf(line, "[length: %[^]]]", length);

    printDebug("Getting stats was successful!\n");


    // Print stats data
    printf("id: %s\n", id);
    printf("artist: %s\n", artist);
    printf("title: %s\n", title);
    printf("album: %s\n", album);
    printf("length: %s\n", length);
    printf("\n");
}


void extractLyrics(FILE* file, char lyric[MAX_LINES][MAX_LINE_LEN], char* line, int* count, int* minutes, float* seconds){

    // Get the lyrics and store them, sscanf() not scanf()!!
    printDebug("Extracting lyrics...\n");

    while (fgets(line, (MAX_LINE_LEN), file)) {
        (*count)++;
        sscanf(line, "[%d:%f]%[^\n]", &minutes[*count], &seconds[*count], lyric[*count]);
    }

    printDebug("Lyrics extracted successfully!\n");
    printDebug("Lyrics count: %d\n", *count);
    printDebug("Lyrics: \n");
    for (int i = 1; i <= *count; i++) {
        printDebug("%s\n", lyric[i]);
    }

}


void lyricsMilli(long int* milli, int* minutes, float* seconds, int* count){

    // Transforming seconds and minutes into milliseconds... Magic number 1000000 physics related :(
    minutes[0] = 0;
    seconds[0] = 0;
    for (int i = 1; i <= *count; i++) {
        milli[i] = (minutes[i] * 60 + seconds[i]) * 1000;
    }

}


void FunctionWordsInLine(char* words[MAX_LINES][32], int wordsInLine[MAX_LINES], int count, char lyric[MAX_LINES][MAX_LINE_LEN]) {

    // Makes a copy of the whole lyrics table, and uses strtok_r to analyze it
    // For some reason strtok crashes the program??
    // if line starts with a space -> meaning it will only be a space
    // consider it one word. 0 words would create problems later
    // then, initialize variable for words, make the token to search for spaces
    // and each time you find a space, increase the words in line variable
    // Then add that token as a word into our new table, which is 2d for words and lines
    // initialize token again and continue

    char copy[MAX_LINES][MAX_LINE_LEN];
    for (int i = 0; i < count; i++) {
        strcpy(copy[i], lyric[i]);

        if (copy[i][0] == ' ') {
            wordsInLine[i] = 1;
            continue;
        }

        wordsInLine[i] = 0;
        char* saveptr;
        char* token = strtok_r(copy[i], " ", &saveptr);
        while (token != NULL && wordsInLine[i] < 32) {
            words[i][wordsInLine[i]] = token;
            wordsInLine[i]++;
            token = strtok_r(NULL, " ", &saveptr);
        }
    }
}
