#!/bin/bash

if [ -z "$1" ]
then
    echo "Usage: $0 <file.mp4>"
    exit 1
fi

mkdir -p raw
ffmpeg -i $1 -vf fps=30 -map 0:a audio.wav -map 0:v raw/out_%09d.png
