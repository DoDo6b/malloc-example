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


static void chunkPush (ChunkList* list, uintptr_t* ptr, size_t size)
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


static void chunkPop (ChunkList* list, size_t index)
{
    assert (index < list->allocated);
    list->allocated--;
    for (size_t i = index; i < list->allocated; i++)
    {
        list->chunks[i] = list->chunks[i+1];
    }
}


static int chunkFind (const ChunkList* list, const uintptr_t* ptr)
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

static size_t chunkFindPlace (const ChunkList* list, const uintptr_t* ptr)
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


static void chunkPushnMerge (ChunkList* list, uintptr_t* ptr, size_t size)
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


void memDump (const void* pointer, size_t words)
{
    const unsigned char* ptr        =   (const unsigned char*)pointer;
    size_t               byteSize   =   words * sizeof(uintptr_t);

    log_string ("  Memory dump of %p(%zu byte(s))\n", pointer, byteSize);
    log_string ("  {\n    ");
    
    log_string ("$x");
    for (size_t i = 0; i < byteSize; i++)
    {
        log_string ("%02zx ", i);
    }
    log_string ("$d\n    $c");

    for (size_t i=0; i < byteSize; i++)
    {
        log_string ("%02x ", *(ptr+i));
    }
        
    log_string ("$d\n  }\n");
}

void chunkDump_ (const char* callerFile, unsigned int callerLine, const Chunk* chunk, bool bytesDump)
{
    #ifndef NDEBUG
    
    if (chunkVerify(chunk) != 0)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk failed verification$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #else

    if (chunk == NULL)
    {
        log_string ("%s:%d: %s: $#$rverification error:$d received a NULL$/#\n", callerFile, callerLine, __func__);
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #endif


    log_string
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

void chunkListDump_ (const char* callerFile, unsigned int callerLine, const char* name, const ChunkList* list, ShowMode showMode, bool bytesDump){
    #ifndef NDEBUG

    if (chunkListVerify(list) != 0)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d ChunkList failed verification$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #else

    if (list == NULL)
    {
        log_string ("%s:%d: %s: $#$rverification error:$d received a NULL$/#\n", callerFile, callerLine, __func__);
        IF_SAFE(exit(EXIT_FAILURE);)
    }

    #endif

    log_string ("%s Dump(allocated: %zu chunk(s))\n{\n", name, list->allocated);

    if (list->allocated==0)
    {
        log_string ("  $*Empty$/*\n");
        log_string ("}\n");
        return;
    }

    switch (showMode)
    {
    case HIDE:
        log_string ("  $*Chunks are hidden$/*\n");
        break;

    case FIRSNLAST:
        if (list->allocated > 8)
        {
            for (size_t i = 0; i < list->allocated && i < 4; ++i)
            {
                chunkDump (&list->chunks[i], bytesDump);
            }
            log_string ("  ...\n");
            for (size_t i = list->allocated - 4; list->allocated > 4 && i < list->allocated; ++i)
            {
                chunkDump (&list->chunks[i], bytesDump);
            }
            break;
        }
        for (size_t i = 0; i < list->allocated; ++i)
        {
            chunkDump (&list->chunks[i], bytesDump);
        }
        break;
    case ALL:
        for (size_t i = 0; i < list->allocated; ++i)
        {
            chunkDump (&list->chunks[i], bytesDump);
        }
        break;

    default:
        log_string ("%s:%d: %s: $#$rsyntax error:$d inappropriate display mode$/#\n", callerFile, callerLine, __func__);
        IF_SAFE(exit(EXIT_FAILURE);)
    }
    
    log_string ("}\n");
}


#ifndef NDEBUG


uint64_t chunkVerify_ (const char* callerFile, unsigned int callerLine, const Chunk* chunk)
{
    uint64_t error_accum = 0;

    if (chunk == NULL)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d received a NULL$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_NULL;
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (chunk->signature != HEXSPEAK)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d received pointer doesn't point to the Chunk (Chunk signatures is corrupted)$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_CH_SIGN;
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (chunk->ptr < Memory || chunk->ptr >= Memory + WORDCAP)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk::ptr doesn’t belong to a region$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_CH_PTROUTOFBOUNDS;
    }
    if (chunk->size < 2)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk::size is 2 word(s) (0 in Release build)$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_CH_ZEROSIZE;
    }
    if (chunk->ptr + chunk->size > Memory + WORDCAP)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk::size is too big (chunk crosses the memory boundary)$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_CH_TOOBIG;
    }
    if (*chunk->ptr != ((uintptr_t)chunk->ptr ^ HEXSPEAK))
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk::ptr has been changed (memory header is different)$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        memDump (chunk->ptr, chunk->size);
        error_accum = error_accum | ERRCODE_CH_PTRCHANGED;
    }
    if (*(chunk->ptr + chunk->size - 1) != ((uintptr_t)(chunk->ptr + chunk->size) ^ HEXSPEAK))
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d Chunk::size has been changed (memory header is different)$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        memDump (chunk->ptr, chunk->size);
        error_accum = error_accum | ERRCODE_CH_SIZECHANGED;
    }

    if (error_accum != 0)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
    }
    return error_accum;
}

uint64_t chunkListVerify_ (const char* callerFile, unsigned int callerLine, const ChunkList* list)
{
    uint64_t error_accum = 0;

    if (list == NULL)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d received a NULL$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_NULL;
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    if (list->sign != CANARY || list->taleSign != CANARY)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d ChunkList signatures is corrupted$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_accum = error_accum | ERRCODE_CL_SIGN;
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
        return error_accum;
    }
    else
    {
        if (list->allocated > CHUNKCAP)
        {
            log_string
            (
                "%s:%d: %s: $#$rverification error:$d ChunkList::allocated > CHUNKCAP (chunk lost)$/#\n",
                callerFile,
                callerLine,
                __func__
            );
            error_accum = error_accum | ERRCODE_CL_ALLOCATED;
            log_string
            (
                "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
                callerFile,
                callerLine,
                __func__,
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
                log_string
                (
                    "%s:%d: %s: $#$rverification error:$d Chunk[%zu] failed verification$/#\n",
                    callerFile,
                    callerLine,
                    __func__,
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
                        log_string
                        (
                            "%s:%d: %s: $#$rverification error:$d ChunkList is unsorted$/#\n",
                            callerFile,
                            callerLine,
                            __func__
                        );
                        error_accum = error_accum | ERRCODE_CL_UNSORTED;
                    }
                    else if (list->chunks[i-1].ptr + list->chunks[i-1].size > list->chunks[i].ptr)
                    {
                        log_string
                        (
                            "%s:%d: %s: $#$rverification error:$d Chunks [%zu] and [%zu] overlapping each other$/#\n",
                            callerFile,
                            callerLine,
                            __func__,
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
            log_string
            (
                "%s:%d: %s: $#$rverification error:$d memory leak$/#\n",
                callerFile,
                callerLine,
                __func__
            );
            log_string ("  ChunkList::words == %zu words, when counted %zu words\n", list->words, words);
            error_accum = error_accum | ERRCODE_CL_MEMLEAK;
        }
    }

    if (error_accum != 0)
    {
        log_string
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
            error_accum
        );
    }
    return error_accum;
}

uint64_t regionVerify_ (const char* callerFile, unsigned int callerLine)
{
    uint64_t error_acum = 0;
    error_acum = error_acum | chunkListVerify(&allocated);
    error_acum = error_acum | chunkListVerify(&freed);
    if (allocated.words + freed.words != WORDCAP)
    {
        log_string 
        (
            "%s:%d: %s: $#$rverification error:$d memory leak detected$/#\n",
            callerFile,
            callerLine,
            __func__
        );
        error_acum = error_acum | ERRCODE_G_MEMLEAK;
    }

    if (error_acum != 0)
    {
        log_string 
        (
            "%s:%d: %s: $#$rverification error:$d verification failed with code: %llu$/#\n",
            callerFile,
            callerLine,
            __func__,
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

    int index = chunkFind (&allocated, (uintptr_t*)ptr);
    if (index != -1)
    {
        const Chunk ichunk = allocated.chunks[index];
        chunkPop (&allocated, (size_t)index);
        chunkPushnMerge (&freed, ichunk.ptr, ichunk.size);

        allocated.words -= ichunk.size;
        freed.words     += ichunk.size;
    }
}