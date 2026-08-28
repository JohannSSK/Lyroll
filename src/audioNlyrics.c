#include <stdio.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>


// For curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
size_t callback(void* data, size_t size, size_t nmemb, void* buffer) {
    size_t total = size * nmemb;
    strncat((char*)buffer, (char*)data, total);
    return total;
}


void CheckLRCLIB(char* SongName, char* ArtistName) {

        // Handle spaces in Song Name
        int i = 0;
        while (SongName[i] != '\0') {
            if (SongName[i] == ' ') {
                SongName[i] = '_';
            }
              if (SongName[i] == '\n'){
                SongName[i] = '\0';
            }
            i++;
        }

        // Handle spaces in Artist name
        i = 0;
        while (ArtistName[i] != '\0') {
            if (ArtistName[i] == ' ') {
                ArtistName[i] = '_';
            }
            if (ArtistName[i] == '\n'){
                ArtistName[i] = '\0';
            }
            i++;
        }


        //Prepare the link

        char url[1024];
        snprintf(url, sizeof(url), "https://lrclib.net/api/get?artist_name=%s&track_name=%s", ArtistName,SongName);
        printf("%s",url);


        char* response_buffer = malloc(65536);
        memset(response_buffer, 0, 65536);


        CURL* curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);


        if (strstr(response_buffer, "artistName") == NULL) {
           strcpy(ArtistName, "");
           strcpy(SongName, "");

        }


        printf("%s\n", response_buffer);
        printf("Artist Name:%s",ArtistName);
        printf("Song Name:%s",SongName);

        free(response_buffer);
    }
}


void CheckYoutube(char* SongName, char* ArtistName) {

        char cmd[512];
        snprintf(cmd, sizeof(cmd),
            "yt-dlp \"ytsearch:%s %s\" --get-id --get-title --no-warnings 2>/dev/null",
            ArtistName, SongName);



        FILE* fp = popen(cmd, "r");
        if (!fp){
            printf("Error with opening pipe in CheckYoutube");
            exit(0);
        }

        char VideoId[64];
        char ExactTitle[256];

        if (fgets(ExactTitle, sizeof(ExactTitle), fp) == NULL) {
            pclose(fp);
            strcpy(ArtistName,"");
            strcpy(SongName, "");
        }
        if (fgets(VideoId, sizeof(VideoId), fp) == NULL) {
            pclose(fp);
            strcpy(ArtistName,"");
            strcpy(SongName, "");    }

        pclose(fp);

        if (strlen(VideoId) > 0 && strlen(ExactTitle) > 0) {
        printf("Youtube Video ID:%s\n", VideoId);
        printf("Youtube Exact Title:%s\n", ExactTitle);
        }
}


char* DownloadAudioMp3(char* SongName, char* ArtistName) {


        char cmd[1024];
        snprintf(cmd, sizeof(cmd),
        "yt-dlp \"ytsearch:%s %s\" --get-id --get-title --no-warnings 2>/dev/null",
        ArtistName, SongName);

        FILE* fp = popen(cmd, "r");
        if (!fp){
            printf("Error with opening pipe in CheckYoutube");
            exit(1);
        }
        char ExactTitle[256];
        char VideoId[64];

        char* Mp3AudioPath = malloc(300);

        strcpy(Mp3AudioPath, "assets/audio/");

        fgets(ExactTitle, sizeof(ExactTitle), fp);
        fgets(VideoId, sizeof(VideoId), fp);

        ExactTitle[strcspn(ExactTitle, "\n")] = 0;
        VideoId[strcspn(VideoId, "\n")] = 0;

        char FileName[1024];
        snprintf(FileName, 1024, "%s-%s.mp3", SongName,ArtistName);

        strncat(Mp3AudioPath, FileName, 300 - strlen(Mp3AudioPath) - 1);

        snprintf(cmd, sizeof(cmd),
        "yt-dlp -f bestaudio --extract-audio --audio-format mp3 -o \"%s\" \"https://www.youtube.com/watch?v=%s\"",
        Mp3AudioPath, VideoId);
        system(cmd);

        pclose(fp);
    return Mp3AudioPath;
}




char* GetLyrics(char* SongName, char* ArtistName) {


    //Prepare the link

    char url[1024];
    snprintf(url, sizeof(url), "https://lrclib.net/api/get?artist_name=%s&track_name=%s", ArtistName,SongName);
    printf("%s",url);


    char* response_buffer = malloc(65536);
    memset(response_buffer, 0, 65536);

    //curl

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response_buffer);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);



    char* JsonLyricsPath = malloc(300);

    char JsonLyricsName[256];
    snprintf(JsonLyricsName, 256, "%s-%s.json", SongName, ArtistName);
    strcpy(JsonLyricsPath, "assets/lyrics/");
    strcat(JsonLyricsPath, JsonLyricsName);


    // TODO: check if file exists before writing, safer and faster for use that wants to play a song multiple times


    FILE* file = fopen(JsonLyricsPath, "w");
    if (file == NULL) {
    printf("Failed to create lyrics file");
    exit(1);
    }

    fprintf(file, "%s", response_buffer);
    fclose(file);


    free(response_buffer);

    return JsonLyricsPath;
}
