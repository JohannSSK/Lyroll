#ifndef AUDIO_NLYRICS_H
#define AUDIO_NLYRICS_H

#include <stddef.h>
#include <stdio.h>

// Callback for curl
size_t callback(void* data, size_t size, size_t nmemb, void* buffer);

// Audio playback
void PlayAudio(char* mp3Path);

// Lyrics checking
int CheckLRCLIB(char* SongName, char* ArtistName, char* exactArtist, size_t artistBufSize, char* exactTrack, size_t trackBufSize);

// YouTube checking
void CheckYoutube(char* SongName, char* ArtistName);

// Downloads
char* DownloadAudioMp3(char* SongName, char* ArtistName);
char* GetLyrics(char* exactArtist, char* exactTrack);

#endif
