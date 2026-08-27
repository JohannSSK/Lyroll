#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "functions.h"

int checkIfFilesAreValid(int argumentCount, char* lyricsFile, char* mp3File, char* pathToLyricsFile, char* pathToMp3File) {

    if (argumentCount < 3) {
        printf("Usage: %s <lyrics_file> <mp3_file>\n", PROGRAM_NAME);
        return 1;
    }

    //.mp3 handling
    chdir("assets/audio");
    if (access(mp3File, F_OK) != 0) {
        printf("Audio file does not exist\n");
        return 1;
    }

    if (strstr(mp3File, ".mp3")==NULL) {
        printf("Your second argument must be a valid .mp3 file");
        printf("Usage: %s <lyrics_file> <mp3_file>\n", PROGRAM_NAME);
        return 1;
    }
    char *fakepathToMp3File = realpath(mp3File, NULL);
    if (fakepathToMp3File == NULL) {
        perror("realpath failed");
        return 1;
    }
    strcpy(pathToMp3File, fakepathToMp3File);
    free(fakepathToMp3File);
    chdir("../..");

    //.lcr handling
    chdir("assets/lyrics");
    if (access(lyricsFile, F_OK) != 0) {
        printf("Lyrics file does not exist\n");
        return 1;
    }

    if (strstr(lyricsFile, ".lrc") == NULL) {
        printf("Your first argument must be a valid .lcr file");
        printf("Usage: %s <lyrics_file> <mp3_file>\n", PROGRAM_NAME);
        return 1;
    }

    char* fakepathToLyricsFile = realpath(lyricsFile, NULL);
    if (fakepathToLyricsFile == NULL) {
        perror("realpath failed");
        return 1;
    }
    strcpy(pathToLyricsFile, fakepathToLyricsFile);
    free(fakepathToLyricsFile);

    chdir("../..");
    return 0;
}

void stats(FILE* file, char* id, char* artist, char* album, char* title, char* length){

    // Get stats
    printDebug("Gettings stats...\n");
    char line[MAX_LINE_LEN];

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


void extractLyrics(FILE* file, char lyric[MAX_LINES][MAX_LINE_LEN], int* count, int* minutes, float* seconds){

    // Get the lyrics and store them, sscanf() not scanf()!!
    printDebug("Extracting lyrics...\n");
    char line[MAX_LINE_LEN];

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

        if (copy[i][0] == ' ' || copy[i][0] == '\0') {
            wordsInLine[i] = 0;
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
