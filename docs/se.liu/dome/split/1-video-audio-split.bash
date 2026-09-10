#!/bin/bash

if [ -z "$1" ]
then
    echo "Usage: $0 <file.mp4>"
    exit 1
fi

mkdir -p raw
ffmpeg -r 1 -i $1 -r 1 -map 0:a audio.wav -map 0:v raw/out_%09d.png
