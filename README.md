# magic_mirror
Magic OOCSI Mirror for the course Technologies for Connectivity

# OOCSI
The name of this device on the OOCSI network is magic_mirror

It produces the following data

| channel      | key | type | range | description |
| ------------ | --- | ---- | ----- | ----------- |
| magic_mirror | distL | int | 0 ~ 400 | the distance measured by the left ultrasonic sensor in centimeters |
| magic_mirror | distR | int | 0 ~ 400 | the distance measured by the right ultrasonic sensor in centimeters |
| magic_mirror | bass | int[3] | 0 - x | Bass frequencies measured by the microphone* |
| magic_mirror | mid | int[13] | 0 - x | Mid frequencies measured by the microphone* |
| magic_mirror | treble | int[127] | 0 - x | Treble frequencies measured by the microphone* |

*first bass bin starts at 156.25 Hz, each next bin (element of the array) takes a step of 78.125 Hz up. Each bass, mid and treble are part of the same FFT frequency spectrum, there only ment as a rough guide to choose the right frequency band.

To understand how to interprit the FFT data I suggest these videos: 
https://www.youtube.com/watch?v=8YaeUYZ_Ex8
https://www.youtube.com/watch?v=spUNpyF58BY

Baseline readings of the bass frequencies are around 80 - 600, not very accurate, would not recommend to use these.
Baseline readings of the mid frequencies are around 10 - 200.
Baseline readings of the treble frequencies are around 0 - 100, these are the most accurate bands.
