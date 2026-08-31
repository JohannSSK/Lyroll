
#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>


// Generic... it's always the same don't worry about it
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total = size * nmemb;
    char **response = (char**)userp;
    size_t old_len = strlen(*response);
    char *new = realloc(*response, old_len + total + 1);
    if (!new) return 0;
    *response = new;
    memcpy(*response + old_len, contents, total);
    (*response)[old_len + total] = '\0';
    return total;
}


// Works with recursion, not the best way to do it,
// but it works and it's pretty simple
char* FindId(char* buffer,char* id_start, int limit){

    char* syncedLyrics;
    id_start = strstr(buffer, "\"id\":");


    if (id_start) {
        id_start += 5;
        syncedLyrics = strstr(buffer, "\"syncedLyrics\":");
        syncedLyrics += 15;


    }


    if (strncmp(syncedLyrics,"null", 4)==0) {

        buffer += 150;

        buffer = strstr(buffer, "\"id\":");
        FindId(buffer,id_start, limit +1);


    } else{
        return id_start;
    }
    if (limit >20){
        printf("No synced lyrics found");
        return NULL;
    }
}

// This is the main function that calls the other functions here. Will be called by main
// It takes the songname and artist name and returns the id of the lyrics so we can curl the good ones later...
// this whole thing is to just find a lyric file that surely contains syncedlyrics, otherwise we exit.
// I did notice that NOT having synced lyrics is VERY rare, even in songs that aren't that known at all...
// It still deserves to be there for protection
//curl_global_init(CURL_GLOBAL_DEFAULT);
// has to have been called before this, in main
char* GetLRCLIB(char* SongName, char* ArtistName){

   char buffer[500] = {0};
    int j = 0;
    for (int i = 0; SongName[i] && j < sizeof(buffer) - 1; i++) {
        if (SongName[i] == ' ') {
            buffer[j++] = '%';
            buffer[j++] = '2';
            buffer[j++] = '0';
        } else {
            buffer[j++] = SongName[i];
        }
    }
    buffer[j] = '\0';
    strcpy(SongName, buffer);


    strcpy(buffer, "");
    j = 0;
    for (int i = 0; ArtistName[i] && j < sizeof(buffer) - 1; i++) {
        if (ArtistName[i] == ' ') {
            buffer[j++] = '%';
            buffer[j++] = '2';
            buffer[j++] = '0';
        } else {
            buffer[j++] = ArtistName[i];
        }
    }
    buffer[j] = '\0';
    strcpy(ArtistName, buffer);

    char url[500] = "";
    snprintf(url, sizeof(url), "https://lrclib.net/api/search?q=%s+%s", SongName, ArtistName);


    CURL* curl = curl_easy_init();
    char* response = malloc(1);
    response[0] = '\0';
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();


    char* id_start;
    char* id = FindId(response,id_start,0);
    if (id == NULL){
        printf("error, didn't find synced lyrics");
        exit(1);
    }

    char* finalid = malloc(10);

    strncpy(finalid, id, 8);
    free(response);
    return finalid;
}

//receive like char* id = GetLRCLIB(SongName, ArtistName);
// free id when done
