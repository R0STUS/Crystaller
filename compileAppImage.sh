#!/bin/usr/bash

mkdir -p Crystaller.AppDir/usr/bin/crystaller

gcc src/appImage_main.c -D_POSIX_C_SOURCE=200112L -std=c90 -pedantic-errors -Wall -Wextra -static -o Crystaller.AppDir/usr/bin/crystaller
