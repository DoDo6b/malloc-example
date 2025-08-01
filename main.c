#include "memory.h"
#include <stdlib.h>

void overWordCap(){
    printf("\nOver than WORDCAP(%lld) test\n\n", WORDCAP);

    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);

    char* ptrs[CHUNKCAP-1];
    for(int i=0; i<CHUNKCAP-1; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    printf("Word(s) used: %lld\n", WordAlloc);
    void* bigptr = memalloc(MEMORYCAP);
    printf("Allocating %d byte(s), pointer returned: %p\n", MEMORYCAP, bigptr);

    printf("Cleanning\n");
    memfree(bigptr);
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
}

void overChunkCap(){
    printf("\nOver than CHUNKCAP(%d) test\n\n", CHUNKCAP);

    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);

    char* ptrs[CHUNKCAP+1];
    for(int i=0; i<CHUNKCAP+1; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    printf("ptrs[%d]: %p; ptrs[%d]: %p\n", CHUNKCAP-1, ptrs[CHUNKCAP-1], CHUNKCAP, ptrs[CHUNKCAP]);
    chunkDump(&allocated, "Allocated", FIRSNLAST);
    chunkDump(&freed, "Freed", FIRSNLAST);

    printf("Cleanning\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
}

void fragmentationTest(){
    printf("\nFragmentation test\n\n");

    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);

    char* ptrs[CHUNKCAP];
    for(int i=0; i<CHUNKCAP; i++){
        ptrs[i] = (char*)memalloc(sizeof(char));
    }
    chunkDump(&allocated, "Allocated", FIRSNLAST);
    chunkDump(&freed, "Freed", FIRSNLAST);

    printf("Deallocating every second ptr\n");
    for(int i=0; i<CHUNKCAP; i+=2){
        memfree(ptrs[i]);
    }
    chunkDump(&allocated, "Allocated", FIRSNLAST);
    chunkDump(&freed, "Freed", FIRSNLAST);

    printf("Cleanning\n");
    for(int i=0; i<CHUNKCAP; i++){
        memfree(ptrs[i]);
    }
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
}

void zeroSize(){
    printf("\nAllocating 0 bytes\n\n");
    void* ptr = memalloc(0);
    printf("returned: %p\n", ptr);
}

void freeRandPtr(){
    printf("\nPointer out of region\n\n");
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
    printf("Deallocating NULL\n");
    memfree(NULL);
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
    void* ptr = malloc(64);
    printf("freeing random pointer out of region: %p\n", ptr);
    memfree(ptr);
    chunkDump(&allocated, "Allocated", ALL);
    chunkDump(&freed, "Freed", ALL);
}

int main(){
    overWordCap();
    overChunkCap();
    fragmentationTest();
    zeroSize();
    freeRandPtr();
    return 0;
}