#!/usr/bin/bash

gcc src/main.c -D_POSIX_C_SOURCE=200112L -std=c90 -pedantic-errors -Wall -Wextra -static -o crystaller.AppDir/usr/bin/crystaller
