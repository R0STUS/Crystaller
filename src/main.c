#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>

#define ROOT 1
#define _POSIX_C_SOURCE 200112L

int isRunningAsRoot() {
    return getuid() == 0;
}

typedef struct {
    char* name;
    void (*execute) (char* ptr, int size);
} Setting;

typedef struct {
    int pid;
    char* name;
    long mem;
} Proc;

/* BUILD VERSION */
char* build = "#2.5.main";
/* $ = Preview; # = Release
   after '.' is name of the branch */

Proc* procs;
int maxMem = 2048;
float sleepTime = 1;
int sleepBeforeTime = 2;
char** ignoringProcs = NULL;
int ignProcsSize = 0;
int* ignProcsSizes = NULL;
char* logs = NULL;
int procsSize;
char* settingsFile = ".config/crystaller/settings.properties";

void changeMaxMem(char* ptr, int size) {
    int i;
    maxMem = 0;
    for (i = 0; i < size; i++) {
        if (isdigit(ptr[i]))
            maxMem = (maxMem * 10) + ptr[i] - '0';
    }
    if (maxMem == 0) {
        maxMem = 2048;
    }
    printf("maxMem=%d\n", maxMem);
}

void changeSleepTime(char* ptr, int size) {
    char* endptr;
    float value = strtof(ptr, &endptr);
    if (ptr != endptr)
        sleepTime = value;
    printf("sleepTime=%f\n", sleepTime);
}

void changeSleepBeforeTime(char* ptr, int size) {
    int i;
    sleepBeforeTime = 0;
    for (i = 0; i < size; i++) {
        if (isdigit(ptr[i]))
            sleepBeforeTime = (sleepTime * 10) + ptr[i] - '0';
    }
    printf("sleepBeforeTime=%d\n", sleepBeforeTime);
}

void changeIgnoring(char* ptr, int size) {
    ignProcsSize++;
    ignProcsSizes = realloc(ignProcsSizes, ignProcsSize * (int)sizeof(int));
    ignoringProcs = realloc(ignoringProcs, ignProcsSize * (int)sizeof(char*));
    if (ignProcsSizes == NULL || ignoringProcs == NULL) {
        ignProcsSize--;
        return;
    }
    ignProcsSizes[ignProcsSize - 1] = size + 1;
    ignoringProcs[ignProcsSize - 1] = malloc(size + 1);
    if (ignoringProcs == NULL) {
        ignProcsSize--;
        return;
    }
    strcpy(ignoringProcs[ignProcsSize - 1], ptr);
    ignoringProcs[ignProcsSize - 1][size] = '\0';
    printf("Added ignoring process: %s\n", ignoringProcs[ignProcsSize - 1]);
}

Setting settings[] = {
    {"maxMem", changeMaxMem},
    {"ignoreName", changeIgnoring},
    {"sleepTime", changeSleepTime},
    {NULL, NULL}
};

void handleSigint(int sig) {
    int i;
    printf("\nSignal %d received! Clearing memory...\n", sig);
    if (ignProcsSize == 0)
        free(ignoringProcs);
    else {
        for (i = 0; i < ignProcsSize; i++) {
            free(ignoringProcs[i]);
        }
    }
    free(ignProcsSizes);
    if (logs)
        free(logs);
    if (procsSize > 0) {
        for (i = 0; i < procsSize; i++) {
            free(procs[i].name);
        }
        free(procs);
    }
    exit(sig);
}

char* format_string(const char *fmt, ...) {
    va_list args;
    int size;
    char* buffer;
    va_start(args, fmt);
    size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (size < 0) return NULL;
    buffer = malloc(size + 1);
    if (!buffer) return NULL;
    va_start(args, fmt);
    vsnprintf(buffer, size + 1, fmt, args);
    va_end(args);
    return buffer;
}

int removeNs(char** dest, int* size) {
    int i;
    for (i = 0; i < *size; i++) {
        if ((*dest)[i] == '\n') {
            (*dest)[i] = '\0';
            *size = i + 1;
            *dest = realloc(*dest, *size);
            if (*dest == NULL) {
                fprintf(stderr, "Cannot allocate memory!\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}

int checkConfig() {
    FILE* file;
    char buffer[1024];
    char* phr[2];
    int phrSize[2] = {0, 0};
    int i;
    int epos;
    phr[0] = malloc(1);
    phr[1] = malloc(1);
    if (phr[0] == NULL || phr[1] == NULL) {
        fprintf(stderr, "Failed to allocate memory!\n");
        return -1;
    }
    settingsFile = format_string("%s/%s", getenv("HOME"), settingsFile);
    file = fopen(settingsFile, "r");
    if (file == NULL) {
        fprintf(stderr, "Failed to open '%s' file: %s\n", settingsFile, strerror(errno));
        return 1;
    }
    free(settingsFile);
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        epos = -1;
        for (i = 0; i < (int)strlen(buffer); i++) {
            if (buffer[i] == '=') {
                epos = i;
                break;
            }
            if (buffer[i] == '\n')
                continue;
        }
        if (epos == -1)
            continue;
        phrSize[0] = epos;
        phr[0] = malloc(phrSize[0]);
        snprintf(phr[0], (size_t)phrSize[0] + 1, "%s", buffer);
        phrSize[1] = (int)strlen(buffer) - epos;
        snprintf(phr[1], (size_t)phrSize[1], "%s", buffer + epos + 1);
        if (removeNs(&phr[1], &phrSize[1]) != 0)
            return -1;
        for (i = 0; settings[i].name != NULL; i++) {
            if (strcmp(phr[0], settings[i].name) == 0) {
                settings[i].execute(phr[1], phrSize[1]);
            }
        }
    }
    phrSize[0] = 0;
    phrSize[1] = 0;
    free(phr[0]);
    free(phr[1]);
    return 0;
}

Proc* getProcs(int* procSize) {
    FILE* file;
    char buffer[256];
    Proc* procs = malloc(1 * sizeof(Proc));
    int i;
    int p;
    char* tmp;
    int procsSize = 0;
    int poss[2] = {-1, -1};
    size_t len;
    char* memwatch;
    char* appdir = getenv("APPDIR");
    if (appdir) {
        memwatch = format_string("%s/usr/bin/memwatch --no-header", appdir);
    }
    else
        memwatch = format_string("memwatch --no-header");
    file = popen(memwatch, "r");
    free(memwatch);
    if (file == NULL) {
        perror("Failed to open the command");
        return NULL;
    }
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        procsSize++;
        procs = realloc(procs, procsSize * sizeof(Proc));
        p = 0;
        for (i = 0; i < (int)strlen(buffer); i++) {
            if (buffer[i] == '\t') {
                poss[p] = i;
                p++;
                if (p > 1)
                    break;
            }
        }
        if (poss[0] == -1 || poss[1] == -1)
            continue;
        tmp = malloc(poss[0] + 1);
        snprintf(tmp, poss[0] + 1, "%s", buffer);
        procs[procsSize - 1].pid = atoi(tmp);
        free(tmp);
        len = poss[1] - poss[0];
        procs[procsSize - 1].name = malloc(len);
        snprintf(procs[procsSize - 1].name, len, "%s", buffer + poss[0] + 1);
        tmp = malloc(strlen(buffer + poss[1] + 1));
        snprintf(tmp, strlen(buffer + poss[1] + 1) + 1, "%s", buffer + poss[1] + 1);
        procs[procsSize - 1].mem = atol(tmp);
        free(tmp);
        procs[procsSize - 1].mem /= 1024;
    }
    pclose(file);
    *procSize = procsSize;
    return procs;
}

void sleepf(float seconds) {
    struct timespec req;
    req.tv_sec = (time_t)seconds;
    req.tv_nsec = (long)((seconds - req.tv_sec) * 1e9);
    nanosleep(&req, NULL);
}

int main() {
    int i; int j; int isIgnoring;
    char* cmd;
    int confcode;
    long cycle = 0;
    signal(SIGINT, handleSigint);
    if (isRunningAsRoot()) {
        fprintf(stderr, "Do NOT run this as root!\n");
        return ROOT;
    }
    confcode = checkConfig();
    if (confcode != 0) {
        fprintf(stderr, "Cannot load config. Using defaults.\n");
    }
    logs = malloc(3);
    sprintf(logs, " \b");
    sleep(sleepBeforeTime);
    while (1) {
        cycle++;
        printf("\x1B[H\x1B[2J\x1B[3J");
        printf(" BUILD: %s\n", build);
        printf(" Logs: {\n%s }\n Crystaller is working [%ld]\n", logs, cycle);
        if (procsSize > 0) {
            for (i = 0; i < procsSize; i++) {
                free(procs[i].name);
            }
            free(procs);
            procsSize = 0;
        }
        procs = getProcs(&procsSize);
        for (i = 0; i < procsSize; i++) {
            isIgnoring = 0;
            for (j = 0; j < ignProcsSize; j++) {
                if (ignoringProcs[j] == NULL) {
                    break;
                }
                else if (procs[i].name == NULL) {
                    break;
                }
                if (strcmp(ignoringProcs[j], procs[i].name) == 0) {
                    isIgnoring = 1;
                }
            }
            if (procs[i].mem >= maxMem && isIgnoring != 1) {
                logs = format_string("%s  Killed: PID:%d - %s - %ldMb\n", logs, procs[i].pid, procs[i].name, procs[i].mem);
                if (logs == NULL) {
                    fprintf(stderr, "Error, while formatting logs.\n");
                    logs = malloc(3);
                    sprintf(logs, " \b");
                }
                cmd = format_string("kill -9 %d", procs[i].pid);
                system(cmd);
                free(cmd);
            }
        }
        sleepf(sleepTime);
    }
}
