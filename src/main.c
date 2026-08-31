#include "../includes/parsing.h"
#include <stdio.h>

int main(void)
{
    char song[100];
    char artist[100];
    printf("Enter song: ");
    fgets(song, 100, stdin);
    song[strcspn(song, "\n")] = '\0';

    printf("Enter artist: ");
    fgets(artist, 100, stdin);
    artist[strcspn(artist, "\n")] = '\0';
    printf("you entered: song:%s artist:%s\n", song, artist);

    struct LyricData data = GetLRCLIB(song, artist);

    printf("\033[36m%-40s\033[0m \033[33m%-15s\033[0m \033[35m%-15s\033[0m\n", "Lyrics", "Start (ms)", "End (ms)");
    printf("\033[36m%-40s\033[0m \033[33m%-15s\033[0m \033[35m%-15s\033[0m\n", "------", "---------", "-------");

    for (int i = 0; i < data.count; i++) {
        printf("\033[32m%-40s\033[0m \033[33m%-15d\033[0m \033[35m%-15d\033[0m\n",
               data.lines[i],
               data.start_ms[i],
               data.end_ms[i]);
    }

    return 0;
}
