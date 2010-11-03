#!/bin/sh

gcc -Isrc --std=c99 ./mongo-c-driver/src/*.c -I ./mongo-c-driver/src/ -lcurl main.c -o main


