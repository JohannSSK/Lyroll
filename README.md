# Lyroll - Synchronized Lyrics Player

A C program that plays MP3 audio with synchronized lyrics display. Parses LRC files and plays the corresponding MP3 file with accurate timing.

## Features

- Parses LRC (lyrics) files with timestamps
- Plays MP3 audio using an external player (mpg123)
- Displays lyrics in sync with audio
- Debug mode for development

## Requirements

- Linux environment
- GCC compiler
- **mpg123** audio player

- ## STILL UNDER DEVELOPMENT

- Will include RayLib support for the lyrics
- Currently takes .mp3 and .lcr files as arguments, only accepted from ?/Lyroll directory itself.


- ## Usage:
- ./main <lyrics_file.lcr> <mp3_file.mp3>
