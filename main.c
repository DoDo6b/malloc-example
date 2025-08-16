#include <stdlib.h>
#include "logger/logger.h"
#include "allocator/memory.h"

static void overWordCap(){
    log_string ("$bAllocating more than WORDCAP(%lu)$d\n", WORDCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    void* ptr = memalloc(MEMORYCAP-64);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    log_string ("We are almost out of memory (Word(s) used: %zu)\n", allocated.words);

    void* bigptr = memalloc(MEMORYCAP);
    log_string ("\n$yAllocating %d byte(s)$d\npointer returned: %p\n\n", MEMORYCAP, bigptr);
    assert(!bigptr);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    log_string ("$gCleanning$d\n");
    memfree(bigptr);
    memfree(ptr);
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$gTEST PASSED$d\n\n");
}

static void overChunkCap(){
    log_string ("$bAllocating more than CHUNKCAP(%d)$d\n", CHUNKCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    char* ptrs[CHUNKCAP+1];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated==CHUNKCAP);

    log_string ("\n$yAllocating another one$d\n");
    char* nextptr = (char*)memalloc(sizeof(char));
    log_string ("pointer returned: %p\n\n", nextptr);
    assert(!nextptr);
    assert(allocated.allocated==CHUNKCAP);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    log_string ("$gCleanning$d\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$gTEST PASSED$d\n\n");
}

static void fragmentationTest(){
    log_string ("$bFragmentation test$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    char* ptrs[CHUNKCAP];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    log_string ("\n$yDeallocating every second ptr$d\n");
    for(int i=0; i<CHUNKCAP; i+=2){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, FIRSNLAST, true);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated == CHUNKCAP/2);


    log_string ("$gCleanning$d\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$gTEST PASSED$d\n\n");
}

static void zeroSize(){
    log_string ("$bAllocating 0 bytes$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$yAllocating: memalloc(0)$d\n");
    void* ptr = memalloc(0);
    log_string ("returned: %p\n", ptr);
    
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);
}

static void freeRandPtr(){
    log_string ("$bFreeing a NULL and random pointer$d\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$yDeallocating NULL$d\n");
    memfree(NULL);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    void* ptr = malloc(64);
    log_string ("\n$yfreeing random pointer:$d %p\n", ptr);
    memfree(ptr);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    IF_DBG (regionVerify();)
    assert(allocated.allocated == 0 && freed.allocated == 1);

    log_string ("\n$gTEST PASSED$d\n\n");
}

#ifndef NDEBUG

static void corruption(){
    log_string ("$bCorruption test$d\n");
    IF_DBG (regionVerify();)

    log_string ("\n$yNULL ptr as a chunk*$d\n");
    log_string ("errCode: %llu\n", chunkVerify(NULL));

    log_string ("\n$yChunk out of bounds$d\n");
    Chunk* outofbounds = (Chunk*)malloc(sizeof(Chunk));
    outofbounds->signature = HEXSPEAK;
    outofbounds->ptr = Memory - 10;
    outofbounds->size = 5;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 10;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 20;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP - 10;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP + 1;
    log_string ("errCode: %llu\n", chunkVerify(outofbounds));
    free(outofbounds);

    log_string ("\n$yChunk::ptr pointing something else, or first 8 bytes of allocated memory is corrupted$d\n");
    void* ptr = memalloc(16);
    Chunk* corrupted = &allocated.chunks[0];
    log_string ("\n$yCorrupted ptr$d\n");
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr += 1;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr -= 1;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    log_string ("\n$yCorrupted first 8 bytes$d\n");
    *corrupted->ptr += 1;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    *corrupted->ptr -= 1;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    log_string ("\n$yChunk has a 0 size$d\n");
    ptr = memalloc(16);
    corrupted = &allocated.chunks[0];
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size=0;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size = 3;
    log_string ("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    IF_DBG (regionVerify();)
    log_string ("\n$gTEST PASSED$d\n\n");
}

static void clcorruption(){
    log_string ("$bCL Corruption test$d\n");
    IF_DBG (regionVerify();)

    log_string ("errCode: %llu\n", chunkListVerify(&allocated));

    log_string ("\n$yNULL ptr as a ChunkList*$d\n");
    log_string ("errCode: %llu\n", chunkListVerify(NULL));

    log_string ("\n$yAllocated more than allowed$d\n");
    allocated.allocated = CHUNKCAP * 2;
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));

    uintptr_t* a = (uintptr_t*)memalloc(16);
    uintptr_t* b = (uintptr_t*)memalloc(16);

    log_string ("\n$yChunks unsorted$d\n");
    Chunk buffer = allocated.chunks[0];
    allocated.chunks[0] = allocated.chunks[1];
    allocated.chunks[1] = buffer;
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.chunks[1] = allocated.chunks[0];
    allocated.chunks[0] = buffer;
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));

    log_string ("\n$yChunks are clipping$d\n");
    allocated.chunks[1].ptr = a; 
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    allocated.chunks[1].ptr = b - 1;
    log_string ("errCode: %llu\n", chunkListVerify(&allocated));

    memfree(a);
    memfree(b);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);

    IF_DBG (regionVerify();)
    log_string ("\n$gTEST PASSED$d\n\n");
}

#endif


int main(int argc, char** argv){
    IF_DBG
    (
    Memory[0] = (uintptr_t)Memory ^ HEXSPEAK;
    Memory[WORDCAP-1] = (uintptr_t)(Memory + WORDCAP) ^ HEXSPEAK;
    )

    const char* fname = (argc == 2) ? argv[1] : "stdout";
    log_start (fname);

    overWordCap();
    overChunkCap();
    fragmentationTest();
    zeroSize();
    freeRandPtr();

    #ifndef NDEBUG

    corruption();
    clcorruption();
    
    #endif

    log_string ("\n$gALL TESTS PASSED$d\n");

    return 0;
}