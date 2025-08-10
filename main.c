#include <stdlib.h>
#include "logger/logger.h"
#include "allocator/memory.h"

void overWordCap(){
    alog ("$bAllocating more than WORDCAP(%lu)$d\n", WORDCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = memalloc(MEMORYCAP-64);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    alog ("We are almost out of memory (Word(s) used: %zu)\n", WordAlloc);

    void* bigptr = memalloc(MEMORYCAP);
    alog ("\n$yAllocating %d byte(s)$d\npointer returned: %p\n\n", MEMORYCAP, bigptr);
    assert(!bigptr);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    alog ("$gCleanning$d\n");
    memfree(bigptr);
    memfree(ptr);
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$gTEST PASSED$d\n\n");
}

void overChunkCap(){
    alog ("$bAllocating more than CHUNKCAP(%d)$d\n", CHUNKCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    char* ptrs[CHUNKCAP+1];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated==CHUNKCAP);

    alog ("\n$yAllocating another one$d\n");
    char* nextptr = (char*)memalloc(sizeof(char));
    alog ("pointer returned: %p\n\n", nextptr);
    assert(!nextptr);
    assert(allocated.allocated==CHUNKCAP);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    alog ("$gCleanning$d\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$gTEST PASSED$d\n\n");
}

void fragmentationTest(){
    alog ("$bFragmentation test$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    char* ptrs[CHUNKCAP];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    alog ("\n$yDeallocating every second ptr$d\n");
    for(int i=0; i<CHUNKCAP; i+=2){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, FIRSNLAST, true);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated == CHUNKCAP/2);


    alog ("$gCleanning$d\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$gTEST PASSED$d\n\n");
}

void zeroSize(){
    alog ("$bAllocating 0 bytes$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$yAllocating: memalloc(0)$d\n");
    void* ptr = memalloc(0);
    alog ("returned: %p\n", ptr);
    
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);
}

void freeRandPtr(){
    alog ("$bFreeing a NULL and random pointer$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$yDeallocating NULL$d\n");
    memfree(NULL);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = malloc(64);
    alog ("\n$yfreeing random pointer:$d %p\n", ptr);
    memfree(ptr);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    alog ("\n$gTEST PASSED$d\n\n");
}

#ifndef NDEBUG

void corruption(){
    alog ("$bCorruption test$d\n");

    alog ("\n$yNULL ptr as a chunk*$d\n");
    alog ("errCode: %llu\n", chunkVerify(NULL));

    alog ("\n$yChunk out of bounds$d\n");
    Chunk* outofbounds = (Chunk*)malloc(sizeof(Chunk));
    outofbounds->signature = HEXSPEAK;
    outofbounds->ptr = Memory - 10;
    outofbounds->size = 5;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 10;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 20;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP - 10;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP + 1;
    alog ("errCode: %llu\n", chunkVerify(outofbounds));
    free(outofbounds);

    alog ("\n$yChunk::ptr pointing something else, or first 8 bytes of allocated memory is corrupted$d\n");
    void* ptr = memalloc(16);
    Chunk* corrupted = &allocated.chunks[0];
    alog ("\n$yCorrupted ptr$d\n");
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr += 1;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr -= 1;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    alog ("\n$yCorrupted first 8 bytes$d\n");
    *corrupted->ptr += 1;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    *corrupted->ptr -= 1;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    alog ("\n$yChunk has a 0 size$d\n");
    ptr = memalloc(16);
    corrupted = &allocated.chunks[0];
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size=0;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size = 3;
    alog ("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    alog ("\n$gTEST PASSED$d\n\n");
}

void clcorruption(){
    alog ("$bCL Corruption test$d\n");
    alog ("errCode: %llu\n", chunkListVerify(&allocated));

    alog ("\n$yNULL ptr as a ChunkList*$d\n");
    alog ("errCode: %llu\n", chunkListVerify(NULL));

    alog ("\n$yAllocated more than allowed$d\n");
    allocated.allocated = CHUNKCAP * 2;
    alog ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    alog ("errCode: %llu\n", chunkListVerify(&allocated));

    void* a = memalloc(16);
    void* b = memalloc(16);

    alog ("\n$yChunks unsorted$d\n");
    Chunk buffer = allocated.chunks[0];
    allocated.chunks[0] = allocated.chunks[1];
    allocated.chunks[1] = buffer;
    alog ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.chunks[1] = allocated.chunks[0];
    allocated.chunks[0] = buffer;
    alog ("errCode: %llu\n", chunkListVerify(&allocated));

    alog ("\n$yChunks are clipping$d\n");
    allocated.chunks[1].ptr = a; 
    alog ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    allocated.chunks[1].ptr = (uintptr_t*)b - 1;
    alog ("errCode: %llu\n", chunkListVerify(&allocated));

    memfree(a);
    memfree(b);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);

    alog ("\n$gTEST PASSED$d\n\n");
}

#endif


int main(int argc, char** argv){
    #ifndef NDEBUG

    Memory[0] = (uintptr_t)Memory ^ HEXSPEAK;
    Memory[WORDCAP-1] = (uintptr_t)(Memory + WORDCAP) ^ HEXSPEAK;

    #endif

    char* fname = (argc == 2) ? argv[1] : "stdout";
    ls_start(fname);

    overWordCap();
    overChunkCap();
    fragmentationTest();
    zeroSize();
    freeRandPtr();

    #ifndef NDEBUG

    corruption();
    clcorruption();
    
    #endif

    alog ("\n$gALL TESTS PASSED$d\n");

    return 0;
}