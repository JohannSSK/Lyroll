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
#include <string.h>
#include "const.h"
#include "functions.h"


int main(int argc,char** args){


    // Going back one directory, from src to the main
    chdir("..");


    //print current directory
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printDebug("Current directory: %s\n", cwd);

    // calling the function to check if all files are valid
    int shouldProgramexit;
    char pathToLyricsFile[MAX_STR_LEN];
    char pathToMp3File[MAX_STR_LEN];

    shouldProgramexit = checkIfFilesAreValid(argc, args[1], args[2], pathToLyricsFile, pathToMp3File);
    if (shouldProgramexit) {
        return 1;
    }

    FILE* lyricsFile = fopen(pathToLyricsFile, "r");


    // initialize variables
    int count = 0; // Amount of lines in LyricsFile
    int minutes[MAX_LINES];
    char lyric[MAX_LINES][MAX_LINE_LEN];
    float seconds[MAX_LINES];

    char id[MAX_STR_LEN];
    char artist[MAX_STR_LEN];
    char title[MAX_STR_LEN];
    char album[MAX_STR_LEN];
    char length[MAX_STR_LEN];


    printDebug("Starting stats function...\n");
    stats(lyricsFile, id, artist, album, title, length);
    printDebug("Stats function returned successfully!\n");

    printDebug("Starting extractLyrics function...\n");
    extractLyrics(lyricsFile, lyric, &count, minutes, seconds);
    printDebug("extractLyrics function returned successfully!\n");

    fclose(lyricsFile);




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
    int pid = initMusic(pathToMp3File);
    printDebug("Music player started successfully!\n");





    // Specifies font
    char* font = NULL;
    for (int i = 3; i < argc; i++) {
        if (strcmp(args[i], "--font") == 0) {
            font = args[i + 1];
            break;
        }
    }

    if (font == NULL) {
        font = "roboto.ttf";
    }


    // Starting raylib window


    int realTime = 1;
    for (int i = 3; i < argc; i++) {
        if (strcmp(args[i], "--real-time-off") == 0) {
            realTime = 0;
        }
    }

    if (realTime) {
        MakeWindowRealTime(count, milli, lyric, font);
    } else {
        MakeWindowStatic(count, milli, lyric, font);
    }



    printDebug("Lyrics played successfully!\n");

    printDebug("Cleaning up...\n");

    kill(pid, SIGTERM);

    printDebug("Exiting.\n");

    return 0;
}
