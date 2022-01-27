#!/bin/bash

if [ -z "$1" ]
then
    echo "Usage: $0 <config.xml>"
    exit 1
fi

mkdir -p split
gm-load --config $1
