#include "memory.h"

int main(){
    chunkDump(&allocated, "Allocated");
    chunkDump(&freed, "Freed");
    char* ptr[1023];
    for(int i=0; i<CHUNKCAP; i++){
        ptr[i] = (char*)memalloc(sizeof(char));
    }
    chunkDump(&allocated, "Allocated");
    chunkDump(&freed, "Freed");
    return 0;
}