#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <raylib.h>
#include <stdlib.h>
#include "const.h"


void initMusic(char* absolute_path){

    printDebug("Forking process...\n");
    int fork_id = fork();

    if (fork_id == 0) {
        printDebug("Executing audio player from child process...\n");
        printDebug("Audio player path: %s\n", AUDIO_PLAYER_PATH);
        printDebug("Audio player name: %s\n", AUDIO_PLAYER_NAME);
        printDebug("Quiet flag: %s\n", QUIET_FLAG);
        printDebug("Absolute path: %s\n", absolute_path);
        printDebug("Executing audio player...\n");
        execlp(AUDIO_PLAYER_NAME, AUDIO_PLAYER_PATH, QUIET_FLAG, absolute_path, NULL);
        printDebug("Executing audio player failed!\n");
        printDebug("Exiting...\n");
        exit(1);
    }
}



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



void lyricsMicro(long int* micro, int* minutes, float* seconds, int* count, long int* difference){

    // Transforming seconds and minutes into microseconds... Magic number 1000000 physics related :(
    minutes[0] = 0;
    seconds[0] = 0;
    for (int i = 1; i <= *count; i++) {
        micro[i] = (minutes[i] * 60 + seconds[i]) * 1000000;
    }

    for (int i = 1; i <= *count; i++) {
        difference[i] = micro[i] - micro[i-1];
    }
}



int main(int argc,char** args){


    // Going back one directory, from src to the main
    // TODO REMOVE THAT. Make it independent of where it runs from, assuming constant main file
    chdir("..");

    if (argc != 3) {
        printf("Usage: %s <lyrics_file> <mp3_file>\n", args[0]);
        return 1;
    }

    char* mp3_file = args[2];
    char* absolute_path = realpath(mp3_file, NULL);
    if (absolute_path == NULL) {
        perror("realpath failed");
        return 1;
    }

    FILE* file = fopen(args[1], "r");
    if (file == NULL) {
        perror("fopen failed");
        return 1;
    }

    // initialize variables
    char line[MAX_LINES];
    int count = 0;
    int minutes[MAX_LINES];
    char lyric[MAX_LINES][MAX_LINE_LEN];
    float seconds[MAX_LINES];

    char id[MAX_STR_LEN];
    char artist[MAX_STR_LEN];
    char title[MAX_STR_LEN];
    char album[MAX_STR_LEN];
    char length[MAX_STR_LEN];


    printDebug("Starting stats function...\n");
    stats(file, line, id, artist, album, title, length);
    printDebug("Stats function returned successfully!\n");

    printDebug("Starting extractLyrics function...\n");
    extractLyrics(file, lyric, line, &count, minutes, seconds);
    printDebug("extractLyrics function returned successfully!\n");

    fclose(file);




    // Initialize microseconds and lyric arrays
    long int micro[256];

    lyric[0][0] = ' ';

    long int difference[count];

    printDebug("Calculating millisecond differences...\n");

    lyricsMicro(micro, minutes, seconds, &count, difference);

    printDebug("Millisecond differences calculated successfully!\n");


    printDebug("Starting music player...\n");
    initMusic(absolute_path);
    printDebug("Music player started successfully!\n");


    // Play the lyrics
    printDebug("Playing lyrics...\n");

    for (int i = 1; i <= count; i++) {
        usleep(difference[i]);
        printf("%s\n", lyric[i]);
    }

    printDebug("Lyrics played successfully!\n");

    printDebug("Cleaning up...\n");

    free(absolute_path);

    printDebug("Exiting.\n");

    return 0;
}
