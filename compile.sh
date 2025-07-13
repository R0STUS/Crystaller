#!/usr/bin/env sh

# Move to the app directory
cd "$(dirname "$(readlink -f "$0")")" || exit

OUTPUT="crystaller"
SOURCES="src/main.c"

# Compile the app
exec gcc $SOURCES -D_POSIX_C_SOURCE=200112L -std=c90 -pedantic-errors -Wall -Wextra -o $OUTPUT "$@"
