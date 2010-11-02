#!/bin/sh

gcc -Isrc --std=c99 /home/ker2x/mongo-c-driver/src/*.c -I /home/ker2x/mongo-c-driver/src/ -lcurl main.c -o main


