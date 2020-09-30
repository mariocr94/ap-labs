#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include "logger.h"

#define RESET 0
#define BRIGHT 1
#define DIM 2
#define UNDERLINE 3
#define BLINK 4
#define REVERSE 7
#define HIDDEN 8

#define BLACK 30
#define RED 31
#define GREEN 32
#define YELLOW 33
#define BLUE 34
#define MAGENTA 35
#define CYAN 36
#define WHITE 37

#define STDOUT 0
#define SYSLOG 1

int logT = 0;


int initLogger(char *logType) {

    if(strcmp(logType, "stdout") == 0)
        logT = STDOUT;
    else if (strcmp(logType, "syslog") == 0)
        logT = SYSLOG;
    else{
        printf("Please use stdout or syslog.\n");
        return 0;
    }
    printf("Initializing Logger on: %s\n", logType);
    return 0;
}

int infof(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    if(logT == STDOUT) {
        int d;
        d = printWF("INFO: ", WHITE, format, arg);
        va_end(arg);
        return d;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_USER);
        syslog(LOG_INFO, "log");
        closelog();
        va_end(arg);
        return 0;
    }
    return 0;
}

int warnf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    if(logT == STDOUT) {
        int d;
        d = printWF("WARNING: ", YELLOW, format, arg);
        va_end(arg);
        return d;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_USER);
        syslog(LOG_WARNING, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
    return 0;
}

int errorf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    if(logT == STDOUT) {
        int d;
        d = printWF("ERROR: ", RED, format, arg);
        va_end(arg);
        return d;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_USER);
        syslog(LOG_ERR, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
    return 0;
}
int panicf(const char *format, ...) {
    va_list arg;
    va_start(arg, format);
    if(logT == STDOUT) {
        int d;
        d = printWF("PANIC: ", MAGENTA, format, arg);
        va_end(arg);
        return d;
    } else {
        openlog("syslog", LOG_NDELAY, LOG_USER);
        syslog(LOG_ALERT, format, arg);
        closelog();
        va_end(arg);
        return 0;
    }
    return 0;
}

void textcolor(int attr, int fg, int bg){
    char command[13];
    sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg, bg + 10);
    printf("%s\n", command);
}

int printWF(char *type, int color, const char *format, va_list arg){
    int d;
    textcolor(BRIGHT, color, HIDDEN);
    printf("%s", type);
    d = vfprintf(stdout, format, arg);
    textcolor(RESET, WHITE, HIDDEN);
    return d;
}
