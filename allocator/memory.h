#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../logger/logger.h"
#include "../useful/Macro.h"


#define MEMORYCAP   65536
#define WORDCAP     (MEMORYCAP/sizeof(uintptr_t))
#define CHUNKCAP    2048
#define CANARY      0X4655434B
#define HEXSPEAK    0XDEADDEDF


typedef struct
{
    IF_DBG(uintptr_t signature;)
    uintptr_t*  ptr;
    size_t      size;
}Chunk;

typedef struct
{
    IF_DBG(uintptr_t sign;)
    size_t  allocated;
    size_t  words;
    Chunk   chunks[CHUNKCAP];
    IF_DBG(uintptr_t taleSign;)
}ChunkList;

extern ChunkList    allocated;
extern ChunkList    freed;
extern uintptr_t    Memory[WORDCAP];

void*   memalloc    (size_t size);
void    memfree     (void* ptr);


void memDump                (const void* pointer, size_t words);
typedef enum{
    HIDE,
    FIRSNLAST,
    ALL,
}ShowMode;
void chunkDump_            (const char* callerFile, unsigned int callerLine,                  const Chunk* chunk,                       bool bytesDump);
void chunkListDump_        (const char* callerFile, unsigned int callerLine, const char* name, const ChunkList* list, ShowMode showMode, bool bytesDump);
#define chunkDump(...)              chunkDump_      (__FILE__, __LINE__, __VA_ARGS__)
#define chunkListDump(list, ...)    chunkListDump_  (__FILE__, __LINE__, #list, list, __VA_ARGS__)

#ifndef NDEBUG


#include "errCodes.h"

uint64_t chunkVerify_       (const char* callerFile, unsigned int callerLine, const Chunk* chunk);
uint64_t chunkListVerify_   (const char* callerFile, unsigned int callerLine, const ChunkList* list);
uint64_t regionVerify_      (const char* callerFile, unsigned int callerLine);
#define chunkVerify(chunk)          chunkVerify_        (__FILE__, __LINE__, chunk)
#define chunkListVerify(list)       chunkListVerify_    (__FILE__, __LINE__, list)
#define regionVerify()              regionVerify_       (__FILE__, __LINE__)


#endif

#endif