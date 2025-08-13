#!/usr/bin/env sh

# Move to the app directory
cd "$(dirname "$(readlink -f "$0")")" || exit

OUTPUT="crystaller"
SOURCES="src/main.c"
BIN="/usr/bin"

printf "Compiling $OUTPUT...\n"
# Compile the app
gcc $SOURCES -D_POSIX_C_SOURCE=200112L -std=c90 -pedantic-errors -Wall -Wextra -o $OUTPUT
if [ -e "$BIN/$OUTPUT" ]; then
    printf "File exists as '$BIN/$OUTPUT', deleting...\n"
    sudo rm "$BIN/$OUTPUT"
fi
if [ -e "$BIN/$OUTPUT" ]; then
    printf "Could not delete '$BIN/$OUTPUT'.\n"
else
    printf "Moving $OUTPUT to $BIN...\n"
    sudo mv $OUTPUT "$BIN/$OUTPUT"
fi
