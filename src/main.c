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


int initMusic(char* absolute_path){

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
    return fork_id;
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



void lyricsMilli(long int* milli, int* minutes, float* seconds, int* count){

    // Transforming seconds and minutes into milliseconds... Magic number 1000000 physics related :(
    minutes[0] = 0;
    seconds[0] = 0;
    for (int i = 1; i <= *count; i++) {
        milli[i] = (minutes[i] * 60 + seconds[i]) * 1000;
    }

}


int findCurrentLine(long int elapsed, int count, long int* milli) {

    // Goes through all the lines from last to first, and returns the line which the elapsed time has already surpassed
    // Making it return the exact line we should be at right now
    int i = count;
    while (elapsed < milli[i] && i>0){
        i--;
    }
    return i;
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

void MakeWindow(int count, long int* milli, char lyric[MAX_LINES][MAX_LINE_LEN]) {
    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, PROGRAM_NAME);
    //MaximizeWindow();
    SetTargetFPS(FPS);;

    if (access("assets/fonts/roboto.ttf", F_OK) == 0) {
        printDebug("Font file exists\n");
    } else {
        printDebug("Font file NOT found\n");
    }

    Font font = LoadFontEx("assets/fonts/roboto.ttf", 128, NULL, 0);


    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    // Times = milli
    // Strings = lyric[count]

    double startTime = GetTime(); // Starting time of the window
    int current = 0; // Current line we are at
    int length[MAX_LINES]; // length of each line in milliseconds
    int separation[MAX_LINES]; // Determines how long each words will appear for.

    // Calculate length for each line
    for (int i = 0; i < count-1; i++) {
        length[i] = milli[i+1] - milli[i];
        printDebug("milli[%d]:%lds\n", i, milli[i]);
        printDebug("length[%d]:%dms\n", i, length[i]);
    }

    //Calculate how many words are in each line

    char* words[MAX_LINES][32];
    int wordsInLine[MAX_LINES];
    FunctionWordsInLine(words, wordsInLine, count, lyric);



    for (int i = 0; i < count; i++) {
        for (int j = 0; j < wordsInLine[i]; j++) {
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            printDebug("lyric[%d]:%s\n", i, lyric[i]);
            printDebug("words[%d][%d]:%s\n", i, j, words[i][j]);
        }
    }


    printDebug("Calculating separations...");





    // Calculate separations
    for (int i = 0; i < count - 1; i++) {
        if (wordsInLine[i] == 0) {
            wordsInLine[i] = 1;
            continue;
        }
        separation[i] = length[i] / wordsInLine[i];
        printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
        printDebug("separation[%d]:%dms\n", i, separation[i]);
    }

    // Calculate total words
    int totalWords = 0;
    for (int i = 0; i < count - 1; i++) {
        totalWords += wordsInLine[i];
    }
    printDebug("totalWords:%d\n", totalWords);


    // make an update milli with new values
    int time[totalWords];
    memset(time, 0, sizeof(time));
    int i = 1;
    int total = 1;
    while (i < count){
        for (int j = 1; j <= wordsInLine[i]; j++) {
            if (separation[i] == 0) {
                separation[i] = length[i];
            }
            printDebug("separation[%d]:%d, j:%d\n", i,separation[i], j);
            printDebug("wordsInLine[%d]:%d\n", i, wordsInLine[i]);
            time[total] = time[total-1] + separation[i];
            printDebug("time[%d]:%dms\n", total, time[total]);
            total++;
        }
        i++;
    }





    char buffer[MAX_LINE_LEN] = "";
    int index = 0;
    int previous = 0;


    while (!WindowShouldClose()){
        double elapsed = (GetTime() - startTime) * 1000;
        printDebug("current time:%dms \n", (int)elapsed);

        current = findCurrentLine(elapsed, count, milli);

        // timer for buffer reset using milli
        if (current != previous) {
            strcpy(buffer, "");
            previous = current;
            printDebug("buffer reset\n");
            index = 0;
        }

        //timer for buffer update
        if (index < wordsInLine[current]){
            int globalIndex =0;
            for (int k = 0; k < current; k++) {
                globalIndex += wordsInLine[k];
            }
            globalIndex += index;


            if (elapsed > time[globalIndex] ) {
                if (index > 0) strcat(buffer, " ");
                strcat(buffer, words[current][index]);
                printDebug("buffer updated: %s\n", buffer);
                index ++;
            }
        }






        printDebug("buffer:%s\n", buffer);

        BeginDrawing();
        ClearBackground(RAYWHITE);






            DrawTextEx(font, buffer, (Vector2){100, 400}, 120, 2, BLACK);
           //
        EndDrawing();


    }
    CloseWindow();


}



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
