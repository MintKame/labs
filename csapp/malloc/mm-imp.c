/*
 * @Author: your name
 * @Date: 2021-06-03 12:56:25
 * @LastEditTime: 2021-06-06 17:34:57
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \malloc\mm.c
 */
/*
 * mm.c - The fastest, least memory-efficient malloc package.
 * 
 * A block has payload, headers and footers.
 * A block is allocated by searching 1st-fit free blocks  
 * Blocks may be coalesced when free. 
 * 
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

#define NDEBUG
/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "noname",
    /* First member's full name */
    "Yz",
    /* First member's email address */
    "@bupt",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* macro   **************************************************/
/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define CHUNK (1 << 12) // 4kb
#define WSIZE 4
#define DSIZE 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* get / put a word(4) at addr */
#define GET(addr) (*(unsigned int *)(addr))

#define PUT(addr, val) (*(unsigned int *)(addr) = (val))

/* pack size, allocated in to a word */
#define PACK(size, allocated) ((size) | (allocated))

/* get size / allocated from word */
#define SIZE(addr) (GET(addr) & ~0x7)
#define ALLOC(addr) (GET(addr) & 1)

/* get addr of header, footer from block ptr (point to 1st non-header byte of blk) */
#define HDR(bp) ((char *)(bp)-WSIZE)
#define FTR(bp) ((char *)(bp) + SIZE(HDR(bp)) - (DSIZE))

/* get addr of prev / next  block ptr from block ptr */
#define PREV(bp) ((char *)(bp)-SIZE((char *)(bp) - (DSIZE)))
#define NEXT(bp) ((char *)(bp) + SIZE(HDR(bp)))
char *list_hdr = NULL; // point to the 1st block of heap
char *list_ftr = NULL; // point to the last block of heap

/* helper-func **********************************************/

/* 
 * merge the free block with prev or next free block, 
 * return new addr after merge
 */
char *merge(char *bp)
{
    int size = SIZE(HDR(bp));
    int prev_alloc = ALLOC(HDR(PREV(bp))), next_alloc = ALLOC(HDR(NEXT(bp)));
    int prev_size = SIZE(HDR(PREV(bp))), next_size = SIZE(HDR(NEXT(bp)));
    // 4 cases
    if (prev_alloc && next_alloc)
        ;
    else if (!prev_alloc && next_alloc)
    {
        size += prev_size;
        bp = PREV(bp);
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    else if (prev_alloc && !next_alloc)
    {
        size += next_size;
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    else
    {
        size += prev_size + next_size;
        bp = PREV(bp);
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    return bp;
}

/* 
 * place newsize(include header and footer) allocated block at bp
  */
void place(char *bp, int newsize)
{
    int remainder = SIZE(HDR(bp)) - newsize;
    if (remainder > DSIZE)
    {
        // use part of free block
        PUT(HDR(bp), PACK(newsize, 1));
        PUT(FTR(bp), PACK(newsize, 1));
        // add reminder to the list
        PUT(HDR(NEXT(bp)), PACK(remainder, 0));
        PUT(FTR(NEXT(bp)), PACK(remainder, 0));
    }
    else
    {
        // use entire free block
        newsize = SIZE(HDR(bp));
        PUT(HDR(bp), PACK(newsize, 1));
        PUT(FTR(bp), PACK(newsize, 1));
    }
}

/* traverse block list to find ptr of fit free block, return NULL if not find */
char *find_free(size_t size)
{
    for (char *ptr = list_hdr; SIZE(HDR(ptr)) > 0; ptr = NEXT(ptr))
    {
        if (!ALLOC(HDR(ptr)) && SIZE(HDR(ptr)) >= size) // a free block fit
        {
            return ptr;
        }
    }
    return NULL;
}

/* 
 *  get_free
 *  get free block at least newsize from free list or request new block
 *  return ptr of free block, NULL if fail
 */
char *get_free(size_t newsize)
{
    char *bp = NULL;
    if ((bp = find_free(newsize)) == NULL) // no free block fit
    {
        // inc the heap for new free block, at least a chunck
        newsize = newsize > CHUNK ? newsize : CHUNK;
        if ((bp = mem_sbrk(newsize)) == (void *)-1)
            return NULL;
        else
        {
            // put the new block and epilogue block
            PUT(HDR(bp), PACK(newsize, 0));
            PUT(FTR(bp), PACK(newsize, 0));
            PUT(HDR(NEXT(bp)), PACK(0, 1));
            // merge the new free block
            return merge(bp);
        }
    }
    return bp;
}

/* helper-func end **********************************************/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // allocate inital heap
    list_hdr = mem_sbrk(4 * WSIZE);
    if (list_hdr == (void *)-1) // have problem
    {
        return -1;
    }
    else
    { // init prologue and epilogue of the heap
        PUT(list_hdr, 0);
        PUT(list_hdr + WSIZE, PACK(8, 1));
        PUT(list_hdr + DSIZE, PACK(8, 1));
        PUT(list_hdr + WSIZE + DSIZE, PACK(0, 1));
        list_hdr += (DSIZE);
        get_free(CHUNK);
        return 0;
    }
}

/* 
 * mm_malloc - return the ptr to the allocated block's payload, 
 *             may inc the heap by mem_sbrk if not find a fit free block
 */
void *mm_malloc(size_t size)
{
    if (size <= 0)
        return NULL;
    // adjust the size to contain header and footer and be multiple of ALIGNMENT
    int newsize = ALIGN(size + SIZE_T_SIZE); // todo
    char *bp = get_free(newsize);            // get ptr of free block
    if (bp == NULL)
        return NULL;
    // place in the free block
    place(bp, newsize);
#ifdef DEBUG
    printf("\nafter malloc\n");
    mm_check(); // todo
    printf("\n");
#endif
    return bp; // payload ptr is 8-byte aligned
}

/*
 * mm_free - Free the block pointed by ptr.
 */
void mm_free(void *ptr)
{
    if (ptr == NULL || !ALLOC(HDR(ptr)))
        return;
    int size = SIZE(HDR(ptr));
    PUT(HDR(ptr), PACK(size, 0));
    PUT(FTR(ptr), PACK(size, 0));
    merge(ptr);
#ifdef DEBUG
    printf("\nafter free\n");
    mm_check(); // todo
    printf("\n");
#endif
}

/*
 * mm_realloc
    allocate a space at least newsize to hold the content from oldptr
 */
void *mm_realloc(void *oldptr, size_t newsize)
{
    if (oldptr == NULL)
        return mm_malloc(newsize);
    if (newsize == 0)
    {
        mm_free(oldptr);
        return NULL;
    }
    // adjust the size to contain header and footer and be multiple of ALIGNMENT
    newsize = ALIGN(newsize + SIZE_T_SIZE); // todo
    size_t oldsize = SIZE(HDR(oldptr));     // include header and footer
    if (newsize <= oldsize)
    {
        if (newsize < oldsize)
        {
            place(oldptr, newsize);
            merge(NEXT(oldptr));
        }
        return oldptr;
    }
    else
    {
        char *bp = get_free(newsize); // get ptr of free block
        if (bp == NULL)
            return NULL;
        // place in the free block
        place(bp, newsize);
        memcpy(bp, oldptr, oldsize - DSIZE);
        mm_free(oldptr);
#ifdef DEBUG
        printf("\nafter realloc\n");
        mm_check(); // todo
        printf("\n");
#endif
        return bp;
    }
}

int mm_check()
{
    for (char *ptr = list_hdr; SIZE(HDR(ptr)) > 0; ptr = NEXT(ptr))
    {
        printf("%d %c | ", SIZE(HDR(ptr)), ALLOC(HDR(ptr)) ? 'A' : 'F');
        // if (!ALLOC(HDR(ptr)) && SIZE(HDR(ptr)) >= size) // a free block fit
        // {
        //     return ptr;
        // }
    }
    return 1; // consistent
}