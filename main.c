#include "memory.h"
#include <stdlib.h>


void overWordCap(){
    printf("\n" COLOR_BLUE "Allocating more than WORDCAP(%lu)\n" STYLE_RESET, WORDCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = memalloc(MEMORYCAP-64);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    printf("We are almost out of memory (Word(s) used: %zu)\n", WordAlloc);

    void* bigptr = memalloc(MEMORYCAP);
    printf(COLOR_YELLOW "Allocating %d byte(s)" STYLE_RESET "\npointer returned: %p\n", MEMORYCAP, bigptr);
    assert(!bigptr);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf("Cleanning\n");
    memfree(bigptr);
    memfree(ptr);
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

void overChunkCap(){
    printf("\n" COLOR_BLUE "Allocating more than CHUNKCAP(%d)\n" STYLE_RESET, CHUNKCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    char* ptrs[CHUNKCAP+1];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    printf(COLOR_YELLOW "Allocated: %zu\n" STYLE_RESET, allocated.allocated);
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated==CHUNKCAP);

    printf(COLOR_YELLOW "Allocating another one\n\n" STYLE_RESET);
    char* nextptr = (char*)memalloc(sizeof(char));
    printf("pointer returned: %p\n", nextptr);
    assert(!nextptr);
    assert(allocated.allocated==CHUNKCAP);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf("Cleanning\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

void fragmentationTest(){
    printf("\n" COLOR_BLUE "Fragmentation test\n" STYLE_RESET);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    char* ptrs[CHUNKCAP];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf(COLOR_YELLOW "Deallocating every second ptr\n" STYLE_RESET);
    for(int i=0; i<CHUNKCAP; i+=2){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, FIRSNLAST, true);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated == CHUNKCAP/2);


    printf("Cleanning\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

void zeroSize(){
    printf("\n" COLOR_BLUE "Allocating 0 bytes\n" STYLE_RESET);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf(COLOR_YELLOW "Allocating: memalloc(0)\n" STYLE_RESET);
    void* ptr = memalloc(0);
    printf("returned: %p\n", ptr);
    
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);
}

void freeRandPtr(){
    printf("\n" COLOR_BLUE "Freeing a NULL and random pointer\n" STYLE_RESET);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf(COLOR_YELLOW "Deallocating NULL\n" STYLE_RESET);
    memfree(NULL);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = malloc(64);
    printf(COLOR_YELLOW "freeing random pointer:" STYLE_RESET " %p\n", ptr);
    memfree(ptr);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

#ifndef NDEBUG

void corruption(){
    printf(COLOR_BLUE "Corruption test\n" STYLE_RESET);

    printf(COLOR_YELLOW "NULL ptr as a chunk*\n" STYLE_RESET);
    printf("errCode: %llu\n", chunkVerify(NULL));

    printf(COLOR_YELLOW "Chunk out of bounds\n" STYLE_RESET);
    Chunk* outofbounds = (Chunk*)malloc(sizeof(Chunk));
    outofbounds->signature = HEXSPEAK;
    outofbounds->ptr = Memory - 10;
    outofbounds->size = 5;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 10;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->size = 20;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP - 10;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    outofbounds->ptr = Memory + WORDCAP + 1;
    printf("errCode: %llu\n", chunkVerify(outofbounds));
    free(outofbounds);

    printf(COLOR_YELLOW "Chunk::ptr pointing something else, or first 8 bytes of allocated memory is corrupted\n" STYLE_RESET);
    void* ptr = memalloc(16);
    Chunk* corrupted = &allocated.chunks[0];
    printf(COLOR_YELLOW "Corrupted ptr\n" STYLE_RESET);
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr += 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr -= 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    printf(COLOR_YELLOW "Corrupted first 8 bytes\n" STYLE_RESET);
    *corrupted->ptr += 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    *corrupted->ptr -= 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    printf(COLOR_YELLOW "Chunk has a 0 size\n" STYLE_RESET);
    ptr = memalloc(16);
    corrupted = &allocated.chunks[0];
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size=0;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size = 3;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

void clcorruption(){
    printf(COLOR_BLUE "CL Corruption test\n" STYLE_RESET);
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    printf(COLOR_YELLOW "NULL ptr as a ChunkList*\n" STYLE_RESET);
    printf("errCode: %llu\n", chunkListVerify(NULL));

    printf(COLOR_YELLOW "Allocated more than allowed\n" STYLE_RESET);
    allocated.allocated = CHUNKCAP * 2;
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    void* a = memalloc(16);
    void* b = memalloc(16);

    printf(COLOR_YELLOW "Chunks unsorted\n" STYLE_RESET);
    Chunk buffer = allocated.chunks[0];
    allocated.chunks[0] = allocated.chunks[1];
    allocated.chunks[1] = buffer;
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.chunks[1] = allocated.chunks[0];
    allocated.chunks[0] = buffer;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    printf(COLOR_YELLOW "Chunks are clipping\n" STYLE_RESET);
    allocated.chunks[1].ptr = a; 
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    allocated.chunks[1].ptr = (uintptr_t*)b - 1;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    memfree(a);
    memfree(b);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);

    printf("\n" COLOR_GREEN "TEST PASSED\n" STYLE_RESET);
}

#endif


int main(){
    #ifndef NDEBUG

    Memory[0] = (uintptr_t)Memory ^ HEXSPEAK;
    Memory[WORDCAP-1] = (uintptr_t)(Memory + WORDCAP) ^ HEXSPEAK;

    #endif

    overWordCap();
    overChunkCap();
    fragmentationTest();
    zeroSize();
    freeRandPtr();

    #ifndef NDEBUG

    corruption();
    clcorruption();
    
    #endif

    printf("\n" COLOR_GREEN "ALL TESTS PASSED\n" STYLE_RESET);

    return 0;
}