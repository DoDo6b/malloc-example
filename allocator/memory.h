#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../logger/logger.h"
#include "../headers/Macro.h"


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
    Chunk   chunks[CHUNKCAP];
    IF_DBG(uintptr_t taleSign;)
}ChunkList;

extern ChunkList    allocated;
extern ChunkList    freed;
extern size_t       WordAlloc;
extern uintptr_t    Memory[WORDCAP];

void*   memalloc    (size_t size);
void    memfree     (void* ptr);

void    chunkPush       (ChunkList* list, void* ptr, size_t size);
void    chunkPushnMerge (ChunkList* list, uintptr_t* ptr, size_t size);
void    chunkPop        (ChunkList* list, size_t index);
int     chunkFind       (const ChunkList* list, const uintptr_t* ptr);
size_t  chunkFindPlace  (const ChunkList* list, const uintptr_t* ptr);

void memDump        (const void* pointer, size_t words);
typedef enum{
    HIDE,
    FIRSNLAST,
    ALL,
}ShowMode;
void cDump          (const char* fileCalledFrom, unsigned int lineCalledFrom,                   const Chunk* chunk,                       bool bytesDump);
void clDump         (const char* fileCalledFrom, unsigned int lineCalledFrom, const char* name, const ChunkList* list, ShowMode showMode, bool bytesDump);
#define chunkDump(...)              cDump(__FILE__, __LINE__, __VA_ARGS__)
#define chunkListDump(list, ...)    clDump(__FILE__, __LINE__, #list, list, __VA_ARGS__)

#ifndef NDEBUG


#include "errCodes.h"

uint64_t cVerify    (const char* fileCalledFrom, unsigned int lineCalledFrom, const Chunk* chunk);
uint64_t clVerify   (const char* fileCalledFrom, unsigned int lineCalledFrom, const ChunkList* list);
#define chunkVerify(chunk)            cVerify(__FILE__, __LINE__, chunk)
#define chunkListVerify(list)        clVerify(__FILE__, __LINE__, list)

#endif

#endif