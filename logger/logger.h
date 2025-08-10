#ifndef LOGGER_H
#define LOGGER_H


#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>

#include "../headers/EscSeq.h"
#include "../headers/htmlTags.h"
#define SWITCH(esc, html) stream == stdout || stream == stderr ? esc : html

#define SPECIFICATOR '$'
#define SPEC_CLOSE '/'

#define SPEC_RST '0'
#define SPEC_BLD '#'
#define SPEC_ITA '*'

#define SPEC_BLK 'x'
#define SPEC_RED 'r'
#define SPEC_GRN 'g'
#define SPEC_YLW 'y'
#define SPEC_BLU 'b'
#define SPEC_MGN 'm'
#define SPEC_CYN 'c'
#define SPEC_WHT 'w'
#define SPEC_DFT 'd'

extern char _logFile[NAME_MAX];
#define ls_start(fname) strncpy(_logFile, fname, sizeof(_logFile) - 1)

int _flog (const char* fileCalledFrom, unsigned int lineCalledFrom, const char* fname, const char* format, ...);
#define flog(fname,  format, ...)   _flog(__FILE__, __LINE__, fname,       format, ##__VA_ARGS__);
#define alog(        format, ...)   _flog(__FILE__, __LINE__, _logFile,      format, ##__VA_ARGS__);
#define slog(        format, ...)   _flog(__FILE__, __LINE__, "stdout",     format, ##__VA_ARGS__);


#endif