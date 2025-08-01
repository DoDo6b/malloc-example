#include "memory.h"

uintptr_t memory[WORDCAP];
size_t WordAlloc = 0;

ChunkList allocated={0};
ChunkList freed={
    .allocated=1,
    .chunks={
        [0]={.ptr=memory, .size=WORDCAP}
    },
};


void chunkPush(ChunkList* list, void* ptr, size_t size){
    assert(list->allocated<CHUNKCAP);
    assert(size>0 && ptr!=NULL);

    list->chunks[list->allocated].ptr = ptr;
    list->chunks[list->allocated].size = size;
    
    for(int i=list->allocated; i>0 && list->chunks[i].ptr < list->chunks[i-1].ptr; i--){
        const Chunk buffer = list->chunks[i];
        list->chunks[i] = list->chunks[i-1];
        list->chunks[i-1] = buffer;
    }

    list->allocated++;
}


int chunkFind(const ChunkList* list, const uintptr_t* ptr){
    int lp = 0;
    int rp = list->allocated - 1;
    while(lp<=rp){
        int mid = (lp + rp)/2;
        if(ptr==list->chunks[mid].ptr) return mid;
        if(ptr<list->chunks[mid].ptr) rp = mid - 1;
        else lp = mid + 1;
    }
    return -1;
}

size_t chunkFindPlace(const ChunkList* list, const uintptr_t* ptr){
    size_t lp = 0;
    size_t rp = list->allocated - 1;
    if(list->chunks[rp].ptr < ptr) return list->allocated;
    while(lp<rp){
        size_t mid = (lp + rp)/2;
        if(list->chunks[mid].ptr > ptr) rp = mid;
        else lp = mid+1;
    }
    return rp;
}


void chunkPushnMerge(ChunkList* list, uintptr_t* ptr, size_t size){
    assert(ptr!=NULL);
    size_t index = chunkFindPlace(list, ptr);

    bool rflag = list->chunks[index].ptr == ptr + size;
    if(rflag){
        list->chunks[index].ptr -= size;
        list->chunks[index].size += size;
    }
    
    bool lflag = index != 0 && list->chunks[index-1].ptr + list->chunks[index-1].size == ptr;
    if(lflag){
        if(rflag){
            list->chunks[index-1].size += list->chunks[index].size;
            chunkPop(list, index);
        }else list->chunks[index-1].size += size;
    }

    if(!rflag && !lflag) chunkPush(list, ptr, size);
}


void chunkPop(ChunkList* list, size_t index){
    assert(index < list->allocated);
    list->allocated--;
    for(size_t i=index; i<list->allocated; i++){
        list->chunks[i] = list->chunks[i+1];
    }
}


void chunkDump(const ChunkList* list, const char* name, ShowMode showMode){
    printf("%s Chunks (%zu):\n", name, list->allocated);

    switch(showMode){
    case HIDE:
        printf("  *Chunks are hidden*\n");
        break;

    case FIRSNLAST:
        for(size_t i=0; i<list->allocated && i<4; ++i){
            printf("  ptr: %p(%p), size: %zu word(s)\n",
                (void*)list->chunks[i].ptr,
                (void*)(list->chunks[i].ptr+list->chunks[i].size),
                list->chunks[i].size);
        }
        if(list->allocated>8) printf("  ...\n");
        for(size_t i=list->allocated-4; list->allocated>4 && i<list->allocated; ++i){
            printf("  ptr: %p(%p), size: %zu word(s)\n",
                (void*)list->chunks[i].ptr,
                (void*)(list->chunks[i].ptr+list->chunks[i].size),
                list->chunks[i].size);
        }
        break;

    case ALL:
        for(size_t i=0; i<list->allocated; ++i){
            printf("  ptr: %p(%p), size: %zu word(s)\n",
                (void*)list->chunks[i].ptr,
                (void*)(list->chunks[i].ptr+list->chunks[i].size),
                list->chunks[i].size);
        }
        break;

    default:
        fprintf(stderr, "SYNTAX ERROR: unkown mode\n");
        return;
    }
    
    printf("%lld chunks in total\n", list->allocated);
    printf("--------------------\n");
}


void* memalloc(size_t size){
    if(allocated.allocated < CHUNKCAP){
        const size_t words = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);
        if(words==0 || WordAlloc + words > WORDCAP) return NULL;

        for(size_t i=0; i<freed.allocated; i++){
            const Chunk ichunk = freed.chunks[i];

            if(ichunk.size>=words){

                size_t tail = ichunk.size-words;
                
                if(tail>0){
                    freed.chunks[i].ptr+=words;
                    freed.chunks[i].size=tail;
                }else chunkPop(&freed, i);

                chunkPush(&allocated, ichunk.ptr, words);

                WordAlloc+=words;
                return ichunk.ptr;
            }
        }
    }
    return NULL;
}

void memfree(void* ptr){
    int index = chunkFind(&allocated, ptr);
    if(index!=-1){
        const Chunk ichunk = allocated.chunks[index];
        chunkPop(&allocated, index);
        WordAlloc -= ichunk.size;
        chunkPushnMerge(&freed, ichunk.ptr, ichunk.size);
    }
}