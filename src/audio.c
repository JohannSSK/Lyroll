#include <unistd.h>
#include "const.h"
#include "functions.h"



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
