#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define SI 2
#define ROOT 1

int isRunningAsRoot() {
    return getuid() == 0;
}

void handleSigint(int sig) {
    printf("\nSIGINT received! Saving logs...\n");
    exit(SI);
}

int main() {
    char* build;
    int max_mem = 2048;
    signal(SIGINT, handleSigint);
    if (isRunningAsRoot()) {
        printf("Do NOT run this as root\n");
        return ROOT;
    }
    printf("\e[H\e[2J");
    /* BUILD VERSION */
    build = "$5.refactor_to_c90";
    /* $ = Preview; # = Release
       after '.' is name of the branch */
    printf(" BUILD: %s\n", build);
    return 0;
}
