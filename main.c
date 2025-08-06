#include "memory.h"
#include <stdlib.h>


void overWordCap(){
    printf(COLOR_BLUE "Allocating more than WORDCAP(%lu)" STYLE_RESET "\n", WORDCAP);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = memalloc(MEMORYCAP-64);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);
    printf("We are almost out of memory (Word(s) used: %zu)\n", WordAlloc);

    void* bigptr = memalloc(MEMORYCAP);
    printf("\n" COLOR_YELLOW "Allocating %d byte(s)" STYLE_RESET "\npointer returned: %p\n\n", MEMORYCAP, bigptr);
    assert(!bigptr);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf(COLOR_GREEN "Cleanning" STYLE_RESET "\n");
    memfree(bigptr);
    memfree(ptr);
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
}

void overChunkCap(){
    printf(COLOR_BLUE "Allocating more than CHUNKCAP(%d)" STYLE_RESET "\n", CHUNKCAP);

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

    printf("\n" COLOR_YELLOW "Allocating another one" STYLE_RESET "\n");
    char* nextptr = (char*)memalloc(sizeof(char));
    printf("pointer returned: %p\n\n", nextptr);
    assert(!nextptr);
    assert(allocated.allocated==CHUNKCAP);

    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf(COLOR_GREEN "Cleanning" STYLE_RESET "\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
}

void fragmentationTest(){
    printf(COLOR_BLUE "Fragmentation test" STYLE_RESET "\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    char* ptrs[CHUNKCAP];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkListDump(&allocated, FIRSNLAST, false);
    chunkListDump(&freed, FIRSNLAST, false);

    printf("\n" COLOR_YELLOW "Deallocating every second ptr" STYLE_RESET "\n");
    for(int i=0; i<CHUNKCAP; i+=2){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, FIRSNLAST, true);
    chunkListDump(&freed, FIRSNLAST, false);
    assert(allocated.allocated == CHUNKCAP/2);


    printf(COLOR_GREEN "Cleanning" STYLE_RESET "\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
}

void zeroSize(){
    printf(COLOR_BLUE "Allocating 0 bytes" STYLE_RESET "\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_YELLOW "Allocating: memalloc(0)" STYLE_RESET "\n");
    void* ptr = memalloc(0);
    printf("returned: %p\n", ptr);
    
    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);
}

void freeRandPtr(){
    printf(COLOR_BLUE "Freeing a NULL and random pointer" STYLE_RESET "\n");

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_YELLOW "Deallocating NULL" STYLE_RESET "\n");
    memfree(NULL);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    void* ptr = malloc(64);
    printf("\n" COLOR_YELLOW "freeing random pointer:" STYLE_RESET " %p\n", ptr);
    memfree(ptr);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);
    assert(freed.allocated==1 && WordAlloc==0 && allocated.allocated==0);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
}

#ifndef NDEBUG

void corruption(){
    printf(COLOR_BLUE "Corruption test" STYLE_RESET "\n");

    printf("\n" COLOR_YELLOW "NULL ptr as a chunk*" STYLE_RESET "\n");
    printf("errCode: %llu\n", chunkVerify(NULL));

    printf("\n" COLOR_YELLOW "Chunk out of bounds" STYLE_RESET "\n");
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

    printf("\n" COLOR_YELLOW "Chunk::ptr pointing something else, or first 8 bytes of allocated memory is corrupted" STYLE_RESET "\n");
    void* ptr = memalloc(16);
    Chunk* corrupted = &allocated.chunks[0];
    printf("\n" COLOR_YELLOW "Corrupted ptr" STYLE_RESET "\n");
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr += 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->ptr -= 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    printf("\n" COLOR_YELLOW "Corrupted first 8 bytes" STYLE_RESET "\n");
    *corrupted->ptr += 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    *corrupted->ptr -= 1;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    printf("\n" COLOR_YELLOW "Chunk has a 0 size" STYLE_RESET "\n");
    ptr = memalloc(16);
    corrupted = &allocated.chunks[0];
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size=0;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    corrupted->size = 3;
    printf("errCode: %llu\n", chunkVerify(corrupted));
    memfree(ptr);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
}

void clcorruption(){
    printf(COLOR_BLUE "CL Corruption test" STYLE_RESET "\n");
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    printf("\n" COLOR_YELLOW "NULL ptr as a ChunkList*" STYLE_RESET "\n");
    printf("errCode: %llu\n", chunkListVerify(NULL));

    printf("\n" COLOR_YELLOW "Allocated more than allowed" STYLE_RESET "\n");
    allocated.allocated = CHUNKCAP * 2;
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    void* a = memalloc(16);
    void* b = memalloc(16);

    printf("\n" COLOR_YELLOW "Chunks unsorted" STYLE_RESET "\n");
    Chunk buffer = allocated.chunks[0];
    allocated.chunks[0] = allocated.chunks[1];
    allocated.chunks[1] = buffer;
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.chunks[1] = allocated.chunks[0];
    allocated.chunks[0] = buffer;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    printf("\n" COLOR_YELLOW "Chunks are clipping" STYLE_RESET "\n");
    allocated.chunks[1].ptr = a; 
    printf("errCode: %llu\n", chunkListVerify(&allocated));
    allocated.allocated = 0;
    allocated.chunks[1].ptr = (uintptr_t*)b - 1;
    printf("errCode: %llu\n", chunkListVerify(&allocated));

    memfree(a);
    memfree(b);

    chunkListDump(&allocated, ALL, true);
    chunkListDump(&freed, ALL, false);

    printf("\n" COLOR_GREEN "TEST PASSED" STYLE_RESET "\n\n");
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

    printf("\n" COLOR_GREEN "ALL TESTS PASSED" STYLE_RESET "\n");

    return 0;
}