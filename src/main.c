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
#include <signal.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <raylib.h>
#include <stdlib.h>
#include "const.h"
#include <string.h>
#include "functions.h"


int main(int argc,char** args){


    // Going back one directory, from src to the main
    // TODO REMOVE THAT. Make it independent of where it runs from, assuming constant main file
    chdir("..");

    if (argc != 3) {
        printf("Usage: %s <lyrics_file> <mp3_file>\n", args[0]);
        return 1;
    }

    chdir("assets/audio");
    //print current directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printDebug("Current directory: %s\n", cwd);

    char* mp3_file = args[2];
    char* absolute_path = realpath(mp3_file, NULL);
    if (absolute_path == NULL) {
        perror("realpath failed");
        return 1;
    }
    chdir("../..");

    chdir("assets/lyrics");
    FILE* file = fopen(args[1], "r");
    if (file == NULL) {
        perror("fopen failed");
        return 1;
    }

    chdir("../..");
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




    // Initialize milliseconds and lyric arrays
    long int milli[256];

    lyric[0][0] = ' ';

    long int difference[count];

    printDebug("Calculating millisecond timestamps...\n");

    lyricsMilli(milli, minutes, seconds, &count);

    printDebug("Millisecond timestamps calculated successfully!\n");

    for (int i = 0; i < count; i++) {
        printDebug("%ldms\n", milli[i]);
    }


    printDebug("Starting music player...\n");
    int pid = initMusic(absolute_path);
    printDebug("Music player started successfully!\n");






    // Starting raylib window

    MakeWindow(count,milli,lyric);



    printDebug("Lyrics played successfully!\n");

    printDebug("Cleaning up...\n");

    kill(pid, SIGTERM);
    free(absolute_path);

    printDebug("Exiting.\n");

    return 0;
}
