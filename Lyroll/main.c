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


int main(int argc,char** args){

    int pid = fork();

    if (pid==0){

        execlp("mpg123", "mpg123", "-q", args[2], NULL);
    } else {





    // Start of LYRICS PRINTING

    // Extracting lyrics from file
    FILE* file = fopen(args[1], "r");

    // initialize variables
    char line[256];
    int count = 0;
    int minutes[256];
    char lyric[256][256];
    float seconds[256];

    char id[64];
    char artist[64];
    char title[64];
    char album[64];
    char length[64];

    // Extracting stats data from file
    fgets(line, sizeof(line), file);
    sscanf(line, "[id: %[^]]]", id);

    fgets(line, sizeof(line), file);
    sscanf(line, "[ar: %[^]]]", artist);

    fgets(line, sizeof(line), file);
    sscanf(line, "[al: %[^]]", album);

    fgets(line, sizeof(line), file);
    sscanf(line, "[ti: %[^]]", title);

    fgets(line, sizeof(line), file);
    sscanf(line, "[length: %[^]]", length);

    // Print stats data
    printf("id: %s\n", id);
    printf("artist: %s\n", artist);
    printf("title: %s\n", title);
    printf("album: %s\n", album);
    printf("length: %s\n", length);
    printf("\n");


    // Get the lyrics and store them
    while (fgets(line, sizeof(line), file)) {
        count++;
        sscanf(line, "[%d:%f]%[^\n]", &minutes[count], &seconds[count], lyric[count]);
    }


    // Initialize microseconds and lyric arrays
    long int micro[256];
    minutes[0] = 0;
    seconds[0] = 0;
    lyric[0][0] = ' ';
    long int difference;

    for (int i = 1; i <= count; i++) {
        micro[i] = (minutes[i] * 60 + seconds[i]) * 1000000;
    }




    // Play the lyrics
    for (int i = 1; i <= count; i++) {
        difference = micro[i] - micro[i-1];
        usleep(difference);
        printf("%s\n", lyric[i]);
    }


    fclose(file);

    }

    // End of LYRICS PRINTING

}
