#include "memory.h"

uintptr_t memory[WORDCAP];

ChunkList allocated={0};
ChunkList freed={
    .allocated=1,
    .chunks={
        [0]={.ptr=memory, .size=WORDCAP}
    },
};


void chunkPush(ChunkList* list, void* ptr, size_t size){
    if(list->allocated>=CHUNKCAP){
        fprintf(stderr, "CHUNKCAP has been reached\n");
        return;
    }
    list->chunks[list->allocated].ptr = ptr;
    list->chunks[list->allocated].size = size;
    list->allocated++;
    
    for(int i=list->allocated-1; i>0&&list->chunks[i].ptr<list->chunks[i-1].ptr; i--){
        const Chunk buffer = list->chunks[i];
        list->chunks[i] = list->chunks[i-1];
        list->chunks[i-1] = buffer;
    }
}


int chunkBinSearch(const ChunkList* list, const void* ptr, int lp, int rp){
    int mid = (lp+rp)/2;
    if(lp<=rp){
        if(list->chunks[mid].ptr==ptr) return mid;
        else if(list->chunks[mid].ptr>ptr) return chunkBinSearch(list, ptr, lp, mid);
        else return chunkBinSearch(list, ptr, mid+1, rp);
    }
    return -1;
}

int chunkFind(const ChunkList* list, const void* ptr){
    return chunkBinSearch(list, ptr, 0, list->allocated-1);
}


void chunkPushnMerge(ChunkList* list, void* ptr, size_t size){
    int index = chunkFind(list, ptr+size);
    if(index!=-1){
        list->chunks[index].ptr -= size;

        list->chunks[index].size += size;
    } else chunkPush(list, ptr, size);
}

void chunkPop(ChunkList* list, size_t index){
    assert(index < list->allocated);
    for(size_t i=index; i<list->allocated-1; i++){
        list->chunks[i] = list->chunks[i+1];
    }
    list->allocated--;
}


void chunkDump(const ChunkList* list, const char* name){
    printf("%s Chunks (%zu):\n", name, list->allocated);
    for(size_t i = 0; i < list->allocated; ++i){
        printf("  start: %p, size: %zu word(s)\n",
               (void*) list->chunks[i].ptr,
               list->chunks[i].size);
    }
    printf("%lld chunks in total\n", list->allocated);
    printf("--------------------\n");
}


void* memalloc(size_t size){
    const size_t words = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
    if(words==0) return NULL;

    for(size_t i=0; i<freed.allocated; i++){
        const Chunk ichunk = freed.chunks[i];

        if(ichunk.size>=words){

            chunkPop(&freed, i);
            chunkPush(&allocated, ichunk.ptr, words);

            size_t tail = ichunk.size-words;
            if(tail>0){
                chunkPush(&freed, ichunk.ptr+words, tail);
            }
            return ichunk.ptr;
        }
    }
    return NULL;
}

void memfree(void* ptr){
    int index = chunkFind(&allocated, ptr);
    if(index!=-1){
        const Chunk ichunk = allocated.chunks[index];
        chunkPop(&allocated, index);
        chunkPushnMerge(&freed, ichunk.ptr, ichunk.size);
    }
}