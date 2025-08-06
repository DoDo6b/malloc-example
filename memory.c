#include "memory.h"

uintptr_t Memory[WORDCAP];
size_t WordAlloc = 0;

ChunkList allocated={
    IF_DBG(.sign = CANARY,)
    .allocated = 0,
    .chunks={0},
    IF_DBG(.taleSign = CANARY,)
};
ChunkList freed={
    IF_DBG(.sign = CANARY,)
    .allocated=1,
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


void cDump (const char* fileCalledFrom, unsigned int lineCalledFrom, const Chunk* chunk, bool memDump)
{
    #ifndef NDEBUG
    
    uint64_t errCode = chunkVerify(chunk);
    if (errCode != 0)
    {
        fprintf (
            stderr,
            "%s:%d: chunkDump: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " verification failed with code: %llu\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom,
            errCode
        );
        abort();
    }

    #else

    if (chunk == NULL)
    {
        fprintf (stderr, "%s:%d: chunkDump: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received a NULL\n" STYLE_RESET, fileCalledFrom, lineCalledFrom);
        abort();
    }

    #endif
    

    const unsigned char* ptr        =   (unsigned char*)chunk->ptr;
    size_t               byteSize   =   chunk->size * sizeof(uintptr_t);

    printf
    (
        "  ptr: %p(%p), size: %zu word(s)\n",
        (void*) chunk->ptr,
        (void*) (chunk->ptr+chunk->size),
        chunk->size
    );

    if (memDump)
    {
        printf ("  {\n    ");
        
        for (size_t i = 0; i < byteSize; i++)
        {
            printf(COLOR_BLACK "%02zx " STYLE_RESET, i);
        }
        printf("\n    ");

        for (size_t i=0; i < byteSize; i++)
        {
            printf(COLOR_BCYAN "%02x " STYLE_RESET, *(ptr+i));
        }
        
        printf("\n  }\n");
    }
}

void clDump(const char* fileCalledFrom, unsigned int lineCalledFrom, const char* name, const ChunkList* list, ShowMode showMode, bool memDump){
    #ifndef NDEBUG

    uint64_t errCode = chunkListVerify(list);
    if (errCode != 0)
    {
        fprintf (
            stderr,
            "%s:%d: chunkListDump: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " verification failed with code: %llu\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom,
            errCode
        );
        abort();
    }

    #else

    if (list == NULL)
    {
        fprintf (stderr, "%s:%d: chunkListDump: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received a NULL\n" STYLE_RESET, fileCalledFrom, lineCalledFrom);
        abort();
    }

    #endif

    printf ("%s Dump(allocated: %zu chunk(s))\n{\n", name, list->allocated);

    if (list->allocated==0)
    {
        printf ("  " STYLE_ITALIC "Empty" STYLE_RESET "\n");
        printf("}\n");
        return;
    }

    switch (showMode)
    {
    case HIDE:
        printf ("  " STYLE_ITALIC "Chunks are hidden" STYLE_RESET "\n");
        break;

    case FIRSNLAST:
        if (list->allocated > 8)
        {
            for (size_t i = 0; i < list->allocated && i < 4; ++i)
            {
                chunkDump (&list->chunks[i], memDump);
            }
            printf("  ...\n");
            for (size_t i = list->allocated - 4; list->allocated > 4 && i < list->allocated; ++i)
            {
                chunkDump (&list->chunks[i], memDump);
            }
            break;
        }
    case ALL:
        for (size_t i = 0; i < list->allocated; ++i)
        {
            chunkDump (&list->chunks[i], memDump);
        }
        break;

    default:
        fprintf (stderr, "%s:%d: chunkListDump: " STYLE_BOLD COLOR_RED "syntax error:" COLOR_DEFAULT " inappropriate display mode\n" STYLE_RESET, fileCalledFrom, lineCalledFrom);
        abort();
    }
    
    printf("}\n");
}


#ifndef NDEBUG


uint64_t cVerify (const char* fileCalledFrom, unsigned int lineCalledFrom, const Chunk* chunk)
{
    uint64_t error_accum = 0;

    if (chunk == NULL)
    {
        fprintf (stderr, "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received a NULL\n" STYLE_RESET, fileCalledFrom, lineCalledFrom);
        error_accum = error_accum | ERRCODE_NULL;
        return error_accum;
    }
    if (chunk->signature != HEXSPEAK)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received pointer doesn't point to the Chunk (Chunk signatures is corrupted)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_SIGN;
        return error_accum;
    }
    if (chunk->size < 2)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk::size is 2 (0 in Release build)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_ZEROSIZE;
    }
    if (chunk->ptr < Memory || chunk->ptr >= Memory + WORDCAP)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk::ptr doesn’t belong to a region\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_PTROUTOFBOUNDS;
    }
    if (chunk->ptr + chunk->size > Memory + WORDCAP)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk::size is too big (chunk crosses the memory boundary)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_TOOBIG;
    }
    if (*chunk->ptr != ((uintptr_t)chunk->ptr ^ HEXSPEAK))
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk::ptr has been changed (memory header is different)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_PTRCHANGED;
    }
    if (*(chunk->ptr + chunk->size - 1) != ((uintptr_t)(chunk->ptr + chunk->size) ^ HEXSPEAK))
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk::size has been changed (memory header is different)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CH_SIZECHANGED;
    }

    return error_accum;
}

uint64_t clVerify (const char* fileCalledFrom, unsigned int lineCalledFrom, const ChunkList* list)
{
    uint64_t error_accum = 0;

    if (list == NULL)
    {
        fprintf (stderr, "%s:%d: chunkListVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received a NULL\n" STYLE_RESET, fileCalledFrom, lineCalledFrom);
        error_accum = error_accum | ERRCODE_NULL;
        return error_accum;
    }
    if (list->sign != CANARY || list->taleSign != CANARY)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkListVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " received pointer doesn't point to the ChunkList (ChunkList signatures is corrupted)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CL_SIGN;
        return error_accum;
    }
    if (list->allocated > CHUNKCAP)
    {
        fprintf
        (
            stderr,
            "%s:%d: chunkListVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " ChunkList::allocated > CHUNKCAP (chunk lost)\n" STYLE_RESET,
            fileCalledFrom,
            lineCalledFrom
        );
        error_accum = error_accum | ERRCODE_CL_ALLOCATED;
        return error_accum;
    }
    for (size_t i = 0; i < list->allocated; i++)
    {
        uint64_t chErrCode = chunkVerify(&list->chunks[i]);
        if (chErrCode != 0)
        {
            fprintf
            (
                stderr,
                "%s:%d: chunkListVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunk[%zu] failed verification\n" STYLE_RESET,
                fileCalledFrom,
                lineCalledFrom,
                i
            );
            error_accum = error_accum | chErrCode;
        }
        else
        {
            if(i>=1 && list->chunks[i-1].ptr + list->chunks[i-1].size > list->chunks[i].ptr)
            {
                fprintf
                (
                    stderr,
                    "%s:%d: chunkListVerify: " STYLE_BOLD COLOR_RED "verification error:" COLOR_DEFAULT " Chunks [%zu] and [%zu] overlapping each other\n" STYLE_RESET,
                    fileCalledFrom,
                    lineCalledFrom,
                    i-1,
                    i
                );
                error_accum = error_accum | ERRCODE_CL_OVERLAPING;
            }
        }
    }
    return error_accum;
}

#endif

void* memalloc (size_t size)
{
    if (allocated.allocated < CHUNKCAP)
    {
        const size_t words = (size + sizeof(uintptr_t) - 1) / sizeof(uintptr_t) IF_DBG(+2);
        if (words == 0 IF_DBG(+2) || WordAlloc + words > WORDCAP) return NULL;

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
                WordAlloc += words;
                return ichunk.ptr IF_DBG(+2);
            }
        }
    }
    return NULL;
}

void memfree (void* ptr)
{
    IF_DBG(ptr = (uintptr_t*)ptr - 2;)

    int index = chunkFind (&allocated, ptr);
    if (index != -1)
    {
        const Chunk ichunk = allocated.chunks[index];
        chunkPop (&allocated, (size_t)index);
        WordAlloc -= ichunk.size;
        chunkPushnMerge (&freed, ichunk.ptr, ichunk.size);
    }
}