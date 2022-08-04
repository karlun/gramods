#!/bin/bash

# ls -v | cat -n | while read n f; do mv -n "$f" "$n.ext"; done

rm -rf cluster/node0 cluster/node1 cluster/node2 cluster/node3 cluster/node4 cluster/node5 cluster/node6
mkdir -p cluster/node0 cluster/node1 cluster/node2 cluster/node3 cluster/node4 cluster/node5 cluster/node6

FFMPEG_SETTINGS="-c:v libx264 -r 30 -pix_fmt yuv420p"
ffmpeg -i split/split0_%09d.png $FFMPEG_SETTINGS cluster/node0/film.mp4
ffmpeg -i split/split1_%09d.png $FFMPEG_SETTINGS cluster/node1/film.mp4
ffmpeg -i split/split2_%09d.png $FFMPEG_SETTINGS cluster/node2/film.mp4
ffmpeg -i split/split3_%09d.png $FFMPEG_SETTINGS cluster/node3/film.mp4
ffmpeg -i split/split4_%09d.png $FFMPEG_SETTINGS cluster/node4/film.mp4
ffmpeg -i split/split5_%09d.png $FFMPEG_SETTINGS cluster/node5/film.mp4
ffmpeg -i split/split6_%09d.png $FFMPEG_SETTINGS cluster/node6/film.mp4
