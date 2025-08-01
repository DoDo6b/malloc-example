#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define MEMORYCAP 65536
#define WORDCAP (MEMORYCAP/sizeof(uintptr_t))
#define CHUNKCAP 4096

extern size_t WordAlloc;

typedef struct{
    uintptr_t* ptr;
    size_t size;
}Chunk;

typedef struct{
    size_t allocated;
    Chunk chunks[CHUNKCAP];
}ChunkList;

extern uintptr_t memory[WORDCAP];

void* memalloc(size_t size);
void memfree(void* ptr);

extern ChunkList allocated;
extern ChunkList freed;

void chunkPush(ChunkList* list, void* ptr, size_t size);
int chunkFind(const ChunkList* list, const uintptr_t* ptr);
size_t chunkFindPlace(const ChunkList* list, const uintptr_t* ptr);
void chunkPushnMerge(ChunkList* list, uintptr_t* ptr, size_t size);
void chunkPop(ChunkList* list, size_t index);

typedef enum{
    HIDE,
    FIRSNLAST,
    ALL,
}ShowMode;
void chunkDump(const ChunkList* list, const char* name, ShowMode showMode);

#endif