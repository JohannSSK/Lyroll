ALL FUNCTIONS IN SONGGUI AND WHAT THEY DO:


DownloadAudioMp3 should take the mp3 Songname And artistname, search for it, download it and return it's path as a string. If it doesn't find anything it shall return NULL, which we immediately after check for and stop the execution if it's wrong, so it should do it without worry.


DownloadLyrics does the same thing, takes song and artist, returns path of the lcr file, and if it doesn't exist it should return NULL which we check for immediately.

TotalLines obviously holds the total lines in the lyrics. Count lines shall return the number  of lines. It receives the path, and it shall find the lyrics and count the \n characters in it, and return them +1.

LyricsLines separates the lines themselves, it returns an array with the pointers to these strings. It also gets the total lines, so the array must have totallines-1 objects. The rest shall be filled with NULL.

LineResetTimeStamps is in ms, and it's when lines change. It's the timestamps included in the lcr file.
So the function that handles it it SeparateTimeStamps, and it takes the path for the lyrics, should create an array of MAX_LINES and put the timestamps in ms one by one. The rest shall be filled with 0's

TotalTime should return the song duration from the file, it's written there, so will just make it into ms. 

LineTimeLengths should return how long each line is in ms. It gets the LineResetTimeStamps, so it shall just do simple subtraction for each two. Last and First lines are empty, so they should last quite long. There will be a check right after it, where it will calculate the sum of the array it returns back, and if it's not equal the total Time, we exit with error regarding LineTimeLengths

WordsPerLine is how many words are in each line, the function is CalculateWordsPerLine and the loop happens outside the function, so it gives it one string pointer, and it should return how many words in it. Simple enough. If no words in string, it shall return 0. This should also handle the situation where the whole string is just a single space, so it disregards that space and returns 0 also.

WordsOfEachLine is an array that will contain each word separately, the function that's responsible is SeparateWords which takes lyricspath, where the lyrics are located, and the number of the current line we are working on, and the word we are working on with, and it shall find it and return it's pointer. shall not worry about empty strings, we handle them using NULL;

WordTimeValues will be filled by CalculateWordTimeValues which gives each word a rating in ms, just a speculation for how long it's going to be sang for. Gives estimates for vowels etc and returns the number it thinks. It of course takes in a single pointer to a string, and returns a good ms value. shall not worry about empty strings. Completely free to give whatever values it wants, absolutely no restraints.

ShrinkWordTimeValues is called when we notice that WordTimeValues gave longer times that the lyric should stay anyway, what this function shall do is remove one ms from each word one by one, until the LineTimeLength is equal to the sum of all WordTimeValues. It takes in a pointer to a row in a 2d array WordTimeValues, an int that represents the words in that line so it doesn't overshoot, and LineTimeLengths which is your criteria for how long it should stay. Just an int. So make sure to sum the WordTimeValues array in each loop. Also, make sure you don't have infinite loops, the shrinkage shall be 1 ms at a time. After that, we are going to check if the sums are indeed, equal to the times of each line. If WordsPerLine is 0... it shouldn't be possible and you should exit with error telling us what happened.

FillLineGap, it's job is completely the opposite of ShrinkWordTimeValues, and it's probably going to be called a lot more often. When WordTimeValues has undershot, then you're called to fill in the gaps, which will be filled 100% on the last word. It takes in a pointer to a row of a 2d array which contains each word time, Words per line which tells you how many words are in the whole row, and linetimelengths which is how long the line should stay for. so you'll find the difference between WordTimeValues and LineTimeLengths and you'll increase the last words time by it. There will be a check after you return. 

FindCurrentLineValue shall return the Line number we are currently at, given the current time in ms, and lineresettimestamps as a whole array and total lines so we don't overshoot. You'll calculate which line we should be at right now.

FindCurrentWordValue is the same thing, you'll get the current time, an array with all the words, and a total words so you don't overshoot. Total words is human numeral, so 0 means literally 0. It won't fit perfectly in your array as a value, but as a reference to where to stop. You'll calculate which word we should be at and return it's line, in computer numerals. meaning 0 is the first line.

FlushLineBuffer. Your job is minimal, you take the buffer string, and make it into the second argument. Basic copy string. Don't forget to add a space at the end of the word.

UpdateBufferWords, you'll take the buffer, and a string after it, and you have to append that string in the end of the buffer. Don't forget to add a space as well at the end of the word.

PrintBuffer will just print the words, you shall get the buffer, resx and resy and do your calculations on how to fit this buffer in real time, and where to print it. You're also given a font.

Drawprogressbar, you'll take the current time and the totaltime, and make the progress time using rectangles.
