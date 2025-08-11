#include "memory.h"

uintptr_t Memory[WORDCAP];

ChunkList allocated={
    IF_DBG(.sign = CANARY,)
    .allocated=0,
    .words=0,
    .chunks={0},
    IF_DBG(.taleSign = CANARY,)
};
ChunkList freed={
    IF_DBG(.sign = CANARY,)
    .allocated=1,
    .words=WORDCAP,
    .chunks={
        [0]={IF_DBG(.signature=HEXSPEAK,) .ptr=Memory, .size=WORDCAP}
    },
    IF_DBG(.taleSign = CANARY,)
};


void chunkPush (ChunkList* list, void* ptr, size_t size)
{
    assert (list->allocated < CHUNKCAP);
    assert (size > 0 && ptr != NULL);

    IF_DBG(list->chunks[list->allocated].signature = HEXSPEAK;)
    list->chunks[list->allocated].ptr =     ptr;
    list->chunks[list->allocated].size =    size;
    
    for (size_t i = list->allocated; i > 0 && list->chunks[i].ptr < list->chunks[i-1].ptr; i--)
    {
        const Chunk buffer  = list->chunks[i];
        list->chunks[i]     = list->chunks[i-1];
        list->chunks[i-1]   = buffer;
    }

    list->allocated++;
}


int chunkFind (const ChunkList* list, const uintptr_t* ptr)
{
    int lp = 0;
    int rp = (int)list->allocated - 1;
    while (lp <= rp)
    {
        int mid = (lp + rp) / 2;
        if (ptr == list->chunks[mid].ptr)   return mid;
        if (ptr  < list->chunks[mid].ptr)   rp = mid - 1;
        else                                lp = mid + 1;
    }
    return -1;
}

size_t chunkFindPlace (const ChunkList* list, const uintptr_t* ptr)
{
    size_t lp = 0;
    size_t rp = list->allocated - 1;
    if (list->chunks[rp].ptr < ptr) return list->allocated;
    while (lp < rp)
    {
        size_t mid = (lp + rp) / 2;
        if (list->chunks[mid].ptr > ptr)    rp = mid;
        else                                lp = mid+1;
    }
    return rp;
}


void chunkPushnMerge (ChunkList* list, uintptr_t* ptr, size_t size)
{
    assert (ptr != NULL);
    size_t index = chunkFindPlace(list, ptr);

    bool rflag = index<list->allocated && list->chunks[index].ptr == ptr + size;
    if (rflag)
    {
        list->chunks[index].ptr -= size;
        list->chunks[index].size += size;
    }
    
    bool lflag = index != 0 && list->chunks[index-1].ptr + list->chunks[index-1].size == ptr;
    if (lflag)
    {
        if (rflag)
        {
            list->chunks[index-1].size += list->chunks[index].size;
            chunkPop (list, index);
        }
        else list->chunks[index-1].size += size;
    }

    if (!rflag && !lflag) chunkPush (list, ptr, size);
}


void chunkPop (ChunkList* list, size_t index)
{
    assert (index < list->allocated);
    list->allocated--;
    for (size_t i = index; i < list->allocated; i++)
    {
        list->chunks[i] = list->chunks[i+1];
    }
}


void memDump (const void* pointer, size_t words)
{
    const unsigned char* ptr        =   (const unsigned char*)pointer;
    size_t               byteSize   =   words * sizeof(uintptr_t);

    alog ("  Memory dump of %p(%zu byte(s))\n", pointer, byteSize);
    alog ("  {\n    ");
    
    alog ("$x");
    for (size_t i = 0; i < byteSize; i++)
    {
        alog ("%02zx ", i);
    }
    alog ("$d\n    $c");

    for (size_t i=0; i < byteSize; i++)
    {
        alog ("%02x ", *(ptr+i));
    }
        
    alog ("$d\n  }\n");
}

void _chunkDump (const char* fileCalledFrom, unsigned int lineCalledFrom, const Chunk* chunk, bool bytesDump)
{
    #ifndef NDEBUG
    
    if (chunkVerify(chunk) != 0)
    {
        alog
        (
            "%s:%d: chunkDump: $#$rverification error:$d Chunk failed verification$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        abort();
    }

    #else

    if (chunk == NULL)
    {
        alog ("%s:%d: chunkDump: $#$rverification error:$d received a NULL$/#\n", fileCalledFrom, lineCalledFrom);
        abort();
    }

    #endif


    alog
    (
        "  ptr: %p(%p), size: %zu word(s)\n",
        (void*) chunk->ptr,
        (void*) (chunk->ptr+chunk->size),
        chunk->size
    );

    if (bytesDump)
    {
        memDump (chunk->ptr, chunk->size);
    }
}

void _chunkListDump (const char* fileCalledFrom, unsigned int lineCalledFrom, const char* name, const ChunkList* list, ShowMode showMode, bool bytesDump){
    #ifndef NDEBUG

    if (chunkListVerify(list) != 0)
    {
        alog
        (
            "%s:%d: chunkListDump: $#$rverification error:$d ChunkList failed verification$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        abort();
    }

    #else

    if (list == NULL)
    {
        alog ("%s:%d: chunkListDump: $#$rverification error:$d received a NULL$/#\n", fileCalledFrom, lineCalledFrom);
        abort();
    }

    #endif

    alog ("%s Dump(allocated: %zu chunk(s))\n{\n", name, list->allocated);

    if (list->allocated==0)
    {
        alog ("  $*Empty$/*\n");
        alog ("}\n");
        return;
    }

    switch (showMode)
    {
    case HIDE:
        alog ("  $*Chunks are hidden$/*\n");
        break;

    case FIRSNLAST:
        if (list->allocated > 8)
        {
            for (size_t i = 0; i < list->allocated && i < 4; ++i)
            {
                chunkDump (&list->chunks[i], bytesDump);
            }
            alog ("  ...\n");
            for (size_t i = list->allocated - 4; list->allocated > 4 && i < list->allocated; ++i)
            {
                chunkDump (&list->chunks[i], bytesDump);
            }
            break;
        }
    case ALL:
        for (size_t i = 0; i < list->allocated; ++i)
        {
            chunkDump (&list->chunks[i], bytesDump);
        }
        break;

    default:
        alog ("%s:%d: chunkListDump: $#$rsyntax error:$d inappropriate display mode$/#\n", fileCalledFrom, lineCalledFrom);
        abort();
    }
    
    alog ("}\n");
}


#ifndef NDEBUG


uint64_t _chunkVerify (const char* fileCalledFrom, unsigned int lineCalledFrom, const Chunk* chunk)
{
    uint64_t error_accum = 0;

    if (chunk == NULL)
    {
        alog ("%s:%d: chunkVerify: $#$rverification error:$d received a NULL$/#\n", fileCalledFrom, lineCalledFrom);
        error_accum = error_accum | ERRCODE_NULL;
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
        return error_accum;
    }
    if (chunk->signature != HEXSPEAK)
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d received pointer doesn't point to the Chunk (Chunk signatures is corrupted)$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_SIGN;
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
        return error_accum;
    }
    if (chunk->ptr < Memory || chunk->ptr >= Memory + WORDCAP)
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d Chunk::ptr doesn’t belong to a region$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_PTROUTOFBOUNDS;
    }
    if (chunk->size < 2)
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d Chunk::size is 2 word(s) (0 in Release build)$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_ZEROSIZE;
    }
    if (chunk->ptr + chunk->size > Memory + WORDCAP)
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d Chunk::size is too big (chunk crosses the memory boundary)$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_TOOBIG;
    }
    if (*chunk->ptr != ((uintptr_t)chunk->ptr ^ HEXSPEAK))
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d Chunk::ptr has been changed (memory header is different)$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        memDump (chunk->ptr, chunk->size);
        error_accum = error_accum | ERRCODE_CH_PTRCHANGED;
    }
    if (*(chunk->ptr + chunk->size - 1) != ((uintptr_t)(chunk->ptr + chunk->size) ^ HEXSPEAK))
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d Chunk::size has been changed (memory header is different)$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        memDump (chunk->ptr, chunk->size);
        error_accum = error_accum | ERRCODE_CH_SIZECHANGED;
    }

    if (error_accum != 0)
    {
        alog
        (
            "%s:%d: chunkVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
    }
    return error_accum;
}

uint64_t _chunkListVerify (const char* fileCalledFrom, unsigned int lineCalledFrom, const ChunkList* list)
{
    uint64_t error_accum = 0;

    if (list == NULL)
    {
        alog ("%s:%d: chunkListVerify: $#$rverification error:$d received a NULL$/#\n", fileCalledFrom, lineCalledFrom);
        error_accum = error_accum | ERRCODE_NULL;
        alog
        (
            "%s:%d: chunkListVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
        return error_accum;
    }
    if (list->sign != CANARY || list->taleSign != CANARY)
    {
        alog
        (
            "%s:%d: chunkListVerify: $#$rverification error:$d ChunkList signatures is corrupted$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CL_SIGN;
        alog
        (
            "%s:%d: chunkListVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
        return error_accum;
    }
    else
    {
        if (list->allocated > CHUNKCAP)
        {
            alog
            (
                "%s:%d: chunkListVerify: $#$rverification error:$d ChunkList::allocated > CHUNKCAP (chunk lost)$/#\n",
                fileCalledFrom,
                lineCalledFrom
            );
            error_accum = error_accum | ERRCODE_CL_ALLOCATED;
            alog
            (
                "%s:%d: chunkListVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
                fileCalledFrom,
                lineCalledFrom,
                error_accum
            );
            return error_accum;
        }

        size_t words = 0;
        for (size_t i = 0; i < list->allocated; i++)
        {
            uint64_t chErrCode = chunkVerify(&list->chunks[i]);
            if (chErrCode != 0)
            {
                alog
                (
                    "%s:%d: chunkListVerify: $#$rverification error:$d Chunk[%zu] failed verification$/#\n",
                    fileCalledFrom,
                    lineCalledFrom,
                    i
                );
                error_accum = error_accum | chErrCode;
            }
            else
            {
                words += list->chunks[i].size;
                if (i >= 1)
                {
                    if (list->chunks[i-1].ptr > list->chunks[i].ptr)
                    {
                        alog
                        (
                            "%s:%d: chunkListVerify: $#$rverification error:$d ChunkList is unsorted$/#\n",
                            fileCalledFrom,
                            lineCalledFrom
                        );
                        error_accum = error_accum | ERRCODE_CL_UNSORTED;
                    }
                    else if (list->chunks[i-1].ptr + list->chunks[i-1].size > list->chunks[i].ptr)
                    {
                        alog
                        (
                            "%s:%d: chunkListVerify: $#$rverification error:$d Chunks [%zu] and [%zu] overlapping each other$/#\n",
                            fileCalledFrom,
                            lineCalledFrom,
                            i-1,
                            i
                        );
                        error_accum = error_accum | ERRCODE_CL_OVERLAPING;
                    }
                }
            }
        }
        if (list->words != words)
        {
            alog
            (
                "%s:%d: chunkListVerify: $#$rverification error:$d memory leak$/#\n",
                fileCalledFrom,
                lineCalledFrom
            );
            alog ("  ChunkList::words == %zu words, when counted %zu words\n", list->words, words);
            error_accum = error_accum | ERRCODE_CL_MEMLEAK;
        }
    }

    if (error_accum != 0)
    {
        alog
        (
            "%s:%d: chunkListVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_accum
        );
    }
    return error_accum;
}

uint64_t _regionVerify (const char* fileCalledFrom, unsigned int lineCalledFrom)
{
    uint64_t error_acum = 0;
    error_acum = error_acum | chunkListVerify(&allocated);
    error_acum = error_acum | chunkListVerify(&freed);
    if (allocated.words + freed.words != WORDCAP)
    {
        alog 
        (
            "%s:%d: regionVerify: $#$rverification error:$d memory leak detected$/#\n",
            fileCalledFrom,
            lineCalledFrom
        );
        error_acum = error_acum | ERRCODE_G_MEMLEAK;
    }

    if (error_acum != 0)
    {
        alog 
        (
            "%s:%d: regionVerify: $#$rverification error:$d verification failed with code: %llu$/#\n",
            fileCalledFrom,
            lineCalledFrom,
            error_acum
        );
    }
    return error_acum;
}

#endif

void* memalloc (size_t size)
{
    if (allocated.allocated < CHUNKCAP)
    {
        const size_t words = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t) IF_DBG(+2);
        if (words == 0 IF_DBG(+2) || allocated.words + words > WORDCAP) return NULL;

        for (size_t i=0; i < freed.allocated; i++)
        {
            const Chunk ichunk = freed.chunks[i];

            if (ichunk.size >= words)
            {
                size_t tail = ichunk.size - words;

                if (tail > 0)
                {
                    freed.chunks[i].ptr     +=  words;
                    freed.chunks[i].size    =   tail;
                    IF_DBG(* freed.chunks[i].ptr                =   ((uintptr_t) freed.chunks[i].ptr         ^ HEXSPEAK);)
                    IF_DBG(*(freed.chunks[i].ptr + tail - 1)    =   ((uintptr_t)(freed.chunks[i].ptr + tail) ^ HEXSPEAK);)
                } 
                else chunkPop (&freed, i);

                IF_DBG(* ichunk.ptr                 =   ((uintptr_t) ichunk.ptr          ^ HEXSPEAK);)
                IF_DBG(*(ichunk.ptr + words - 1)    =   ((uintptr_t)(ichunk.ptr + words) ^ HEXSPEAK);)

                chunkPush (&allocated, ichunk.ptr, words);

                allocated.words += words;
                freed.words     -= words;

                return ichunk.ptr IF_DBG(+1);
            }
        }
    }
    return NULL;
}

void memfree (void* ptr)
{
    IF_DBG(ptr = (uintptr_t*)ptr - 1;)

    int index = chunkFind (&allocated, ptr);
    if (index != -1)
    {
        const Chunk ichunk = allocated.chunks[index];
        chunkPop (&allocated, (size_t)index);
        chunkPushnMerge (&freed, ichunk.ptr, ichunk.size);

        allocated.words -= ichunk.size;
        freed.words     += ichunk.size;
    }
}