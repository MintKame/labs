/*
 * @Author: your name
 * @Date: 2021-06-03 12:56:25
 * @LastEditTime: 2021-06-06 19:00:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \malloc\mm.c
 */
/*
 * mm.c - malloc package. // todo
 * Structure: Segregated free lists  
 *            A allocted block has payload, headers and footers.
 *            A free block has payload, headers and footers, next / prev free ptr.
 * Allocate: best fit
 * Blocks may be coalesced when free. 
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

// #define DEBUG_R
// #define DEBUG_F
// #define DEBUG_A
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
#define ARR_SIZE 1023

/* get / put a word(4) at addr */
#define GET(addr) (*(unsigned int *)(addr))
#define PUT(addr, val) (*(unsigned int *)(addr) = (unsigned int)(val))

/* pack size, allocated in to a word */
#define PACK(size, allocated) ((size) | (allocated))

/* get size / allocated from word */
#define SIZE(addr) (GET(addr) & ~0x7)
#define ALLOC(addr) (GET(addr) & 1)

/* get addr of header, footer from block ptr (point to 1st non-header byte of blk) */
#define HDR(bp) ((char *)(bp)-WSIZE)
#define FTR(bp) ((char *)(bp) + SIZE(HDR(bp)) - (DSIZE))

/* get addr of prev / next block ptr from block ptr */
#define PREV_BLK(bp) ((char *)(bp)-SIZE((char *)(bp) - (DSIZE)))
#define NEXT_BLK(bp) ((char *)(bp) + SIZE(HDR(bp)))

/* get addr of prev / next ptr of free block */
#define PREV_FREE(bp) (void *)(*(unsigned int *)(bp))
#define NEXT_FREE(bp) (void *)(*(unsigned int *)((char *)(bp) + WSIZE))

/* set pre / next free at addr */
#define SET_PREV_FREE(bp, val) PUT((bp), (val))
#define SET_NEXT_FREE(bp, val) PUT(((char *)(bp) + WSIZE), (val))

void *list[ARR_SIZE];  // the array of free blocks list
void *list_hdr = NULL; // point to the first block of heap
int cnt = 0;           // the total call of free, malloc, realloc
static int mm_check();
static int get_index_by_size(int size);
static void add_to_list(void *bp);
static void remove_from_list(void *bp);
static char *merge(char *bp);
static void *request_free_block(int newsize);
static int place(char *bp, int newsize);
static char *find_free(size_t size);
static void printblk();
static void printfreelist();

/* define of helper-func **********************************************/
int get_index_by_size(int size)
{
    if (size < 1024)
    {
        return size - 4;
    }
    else if (size <= 2047)
    {
        return 1020;
    }
    else if (size <= 4095)
    {
        return 1021;
    }
    else
    {
        return 1022;
    }
}

/* add block to begin of list */
void add_to_list(void *bp)
{
    int index = get_index_by_size(SIZE(HDR(bp)));
    SET_PREV_FREE(bp, &(list[index]));
    SET_NEXT_FREE(bp, list[index]);
    if (list[index] != NULL)
        SET_PREV_FREE(list[index], bp);
    list[index] = bp;
}

/* remove block from list */
void remove_from_list(void *bp)
{
    int index = get_index_by_size(SIZE(HDR(bp)));
    void *prev_ptr = PREV_FREE(bp);
    void *next_ptr = NEXT_FREE(bp);
    if (next_ptr != NULL)
    {
        SET_PREV_FREE(next_ptr, prev_ptr);
    }
    if (prev_ptr == &(list[index]))
    {
        list[index] = next_ptr;
    }
    else
    {
        SET_NEXT_FREE(prev_ptr, next_ptr);
    }
    SET_PREV_FREE(bp, NULL);
    SET_NEXT_FREE(bp, NULL);
}

/* 
 * merge with prev or next free block
 * return new addr after merge
 */
char *merge(char *bp)
{
    int alloc = ALLOC(HDR(bp)), size = SIZE(HDR(bp));
    void *prev = PREV_BLK(bp), *next = NEXT_BLK(bp);
    int prev_alloc = ALLOC(HDR(prev)), next_alloc = ALLOC(HDR(next));
    int prev_size = SIZE(HDR(prev)), next_size = SIZE(HDR(next));
    // 4 cases
    if (alloc || (prev_alloc && next_alloc))
        ;
    else if (!prev_alloc && next_alloc)
    {
        // remove neighbor from list
        remove_from_list(prev);
        // merge
        size += prev_size;
        bp = PREV_BLK(bp);
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    else if (prev_alloc && !next_alloc)
    {
        // remove neighbor from list
        remove_from_list(next);
        // merge
        size += next_size;
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    else
    {
        // remove neighbor from list
        remove_from_list(prev);
        remove_from_list(next);
        // merge
        size += prev_size + next_size;
        bp = PREV_BLK(bp);
        PUT(HDR(bp), PACK(size, 0));
        PUT(FTR(bp), PACK(size, 0));
    }
    return bp;
}

/* 
 * inc the heap for new free block, at least a chunck
 * return the ptr of new block
 */
void *request_free_block(int newsize)
{
    void *bp = NULL;
    newsize = newsize > CHUNK ? newsize : CHUNK; // at least a chunck
    if ((bp = mem_sbrk(newsize)) == (void *)-1)
        return NULL;
    else
    {
        // put the new block and epilogue block
        PUT(HDR(bp), PACK(newsize, 0));
        PUT(FTR(bp), PACK(newsize, 0));
        PUT(HDR(NEXT_BLK(bp)), PACK(0, 1));
        // merge with neighbour
        bp = merge(bp);
        // put in the list
        add_to_list(bp);
        return bp;
    }
}

/* 
 * place newsize(include header and footer) allocated block at bp,
 * return 1 if add remainder to free list
  */
int place(char *bp, int newsize)
{
    int size = SIZE(HDR(bp));
    int remainder = size - newsize;
    if (remainder >= 2 * DSIZE)
    {
        // use part of free block
        PUT(HDR(bp), PACK(newsize, 1));
        PUT(FTR(bp), PACK(newsize, 1));
        // add reminder to the list
        void *next = NEXT_BLK(bp);
        PUT(HDR(next), PACK(remainder, 0));
        PUT(FTR(next), PACK(remainder, 0));
        add_to_list(next);
        return 1;
    }
    else
    {
        // use entire free block
        PUT(HDR(bp), PACK(size, 1));
        PUT(FTR(bp), PACK(size, 1));
        return 0;
    }
}

/* traverse block list to find ptr of fit free block, return NULL if not find */
char *find_free(size_t size)
{
    // loop the arr of all lists
    //      traverse the list to get the fit free block
    // same size of list , not need best fit
    int index = get_index_by_size(size);
    for (size_t i = index; i <= 1019; i++)
    {
        if (list[i] != NULL)
        {
            // printf("index: %d find\n", i);
            // printfreelist();
            // printblk();
            return list[i];
        }
    }
    // need best fit
    for (size_t i = 1020 > index ? 1020 : index; i < ARR_SIZE; i++)
    {
        // printf("a");
        void *best_bp = NULL;
        int best_size = 0;
        for (void *ptr = list[i]; ptr != NULL; ptr = NEXT_FREE(ptr))
        {
            // printf("b");
            int ptr_size = SIZE(HDR(ptr));
            if (ptr_size == size) // already best size
            {
                // printf("c index: %d find\n", i);
                // printfreelist();
                // printblk();
                return ptr;
            }
            else if (ptr_size > size &&
                     (best_bp == NULL || best_size > ptr_size)) // better blk
            {
                // printf("d index: %d better blk\n", i);
                best_bp = ptr;
                best_size = SIZE(HDR(best_bp));
            }
        }
        if (best_bp != NULL)
        {
            // printf("d index: %d find\n", i);
            // printfreelist();
            // printblk();
            return best_bp;
        }
    }
    return NULL;
}

/* helper-func end **********************************************/

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // init list with NULL
    for (size_t i = 0; i < ARR_SIZE; i++)
    {
        list[i] = NULL;
    }

    // allocate inital heap
    list_hdr = mem_sbrk(4 * WSIZE);
    if (list_hdr == (void *)-1) // have problem
    {
        return -1;
    }
    else
    {
        // init prologue and epilogue of the heap
        PUT(list_hdr, 0);
        PUT(list_hdr + WSIZE, PACK(8, 1));
        PUT(list_hdr + DSIZE, PACK(8, 1));
        PUT(list_hdr + WSIZE + DSIZE, PACK(0, 1));
        list_hdr += DSIZE;
#ifdef DEBUG
        printf("********************************************\n");
        printf("\nbefore init\n");
        mm_check();
        // printfreelist();
#endif
        // add a free block, payload is chunk
        request_free_block(CHUNK);
        // printf("%p", tmp);
#ifdef DEBUG
        printf("\nafter init\n");
        mm_check();
        printf("********************************************\n");
        // printfreelist();
#endif
        return 0;
    }
}

/* 
 * mm_malloc - return the ptr to the allocated block, 
 *             may inc the heap by mem_sbrk if not find a fit free block
 */
void *mm_malloc(size_t payload)
{
    cnt++;
    void *bp = NULL;
    // adjust size
    int size = payload + DSIZE; // for hdr and ftr
    if (size < (DSIZE << 1))    // at least 4 word (hdr, ftr, 2*ptr)
        size = DSIZE << 1;
    if (size % 8 != 0) // align to 8 (low 3 bit is a/f)
        size += 8 - (size % 8);
#ifdef DEBUG_A
    printf("********************************************\n");
    printf("\nbefore malloc %d (payload)\n", payload);
    mm_check();
    printfreelist();

#endif
    bp = find_free(size);
    // not find, request a new free block, size is max(chunck size, size)
    if (bp == NULL)
    {
        if ((bp = request_free_block(size)) == NULL)
        {
            return NULL;
        }
    }
    // remove block from list
    remove_from_list(bp);
    // place in the the block, may split
    place(bp, size);
#ifdef DEBUG_A
    printf("\nafter malloc %d(payload)\n", payload);
    printf("brk: %p\n", mem_sbrk(0));
    printfreelist();
    mm_check();
    printf("********************************************\n");
#endif
    return bp;
}

/*
 * mm_free - Free the block pointed by ptr.
 */
void mm_free(void *ptr)
{
#ifdef DEBUG_F
    printf("********************************************\n");
    printf("\nbefore free %p\n", ptr);
    mm_check();
    printfreelist();
#endif
    cnt++;
    if (ptr == NULL || !ALLOC(HDR(ptr)))
        return;
    // get size of block
    int size = SIZE(HDR(ptr));
    // change the hdr and ftr to free
    PUT(HDR(ptr), PACK(size, 0));
    PUT(FTR(ptr), PACK(size, 0));
    // if neighbor is free, merge; then add to free list
    ptr = merge(ptr);
    add_to_list(ptr);
#ifdef DEBUG_F
    printf("\nafter free\n");
    mm_check();
    printfreelist();
    printf("********************************************\n");
#endif
}

/*
 * mm_realloc
    allocate a space at least newsize to hold the content from oldptr
 */
void *mm_realloc(void *oldptr, size_t newpayload)
{
#ifdef DEBUG_R
    // printf("********************************************\n");
    // printf("realloc %d at %p\n", newpayload, oldptr);
    // printf("\nbefore realloc\n");
    // printfreelist();
    // mm_check();
    // printf("a\n");
#endif
    cnt++;
    if (oldptr == NULL)
    {
        cnt--;
        return mm_malloc(newpayload);
    }
    if (newpayload == 0)
    {
        cnt--;
        mm_free(oldptr);
        return NULL;
    }
    // adjust size
    int newsize = newpayload + DSIZE; // for hdr and ftr
    if (newsize < (DSIZE << 1))       // at least 4 word (hdr, ftr, 2*ptr)
        newsize = DSIZE << 1;
    if (newsize % 8 != 0) // align to 8 (low 3 bit is a/f)
        newsize += 8 - (newsize % 8);
    // compare with oldsize
    size_t oldsize = SIZE(HDR(oldptr)); // include header and footer
    if (newsize <= oldsize)
    {
        if (newsize < oldsize)
        {
            if (place(oldptr, newsize))
            {
                // merge with next block
                void *remainder = NEXT_BLK(oldptr);
                void *next = NEXT_BLK(remainder);
                if (!ALLOC(HDR(next)))
                {
                    // remove neighbor from list
                    remove_from_list(remainder);
                    remove_from_list(next);
                    // merge
                    int size = SIZE(HDR(remainder)) + SIZE(HDR(next));
                    PUT(HDR(remainder), PACK(size, 0));
                    PUT(FTR(remainder), PACK(size, 0));
                    add_to_list(remainder);
                }
            }
        }
#ifdef DEBUG_R
        // printf("\nafter realloc 1\n");
        mm_check();
        // printfreelist();
        // printf("********************************************\n");
#endif
        return oldptr;
    }
    else // need new block
    {
        void *bp = find_free(newsize);
        // not find, request a new free block, size is max(chunck size, size)
        if (bp == NULL)
        {
            if ((bp = request_free_block(newsize)) == NULL)
            {
                return NULL;
            }
        }
        // remove block from list
        remove_from_list(bp);
        // place in the the block, may split
        place(bp, newsize);
        if (bp == NULL)
            return NULL;
        memcpy(bp, oldptr, oldsize - DSIZE);
        cnt--;
        mm_free(oldptr);
#ifdef DEBUG_R
        // printf("\nafter realloc 2\n");
        mm_check();
        // printfreelist();
        // printf("********************************************\n");
#endif
        return bp;
    }
}

/* check consistent of all block and free_list */
int mm_check()
{
    // printf("cnt of op: %d\n", cnt);
    int tag = 0;
    for (size_t i = 0; i < ARR_SIZE; i++)
    {
        if (list[i] != NULL)
        {
            for (void *ptr = list[i]; ptr != NULL; ptr = NEXT_FREE(ptr))
            {
                if (ALLOC(HDR(ptr)) != 0) // block in free list are free
                {
                    printf("block  %p in free list  not free\n", ptr);
                    tag = 1;
                }
            }
        }
    }
    for (char *ptr = list_hdr; SIZE(HDR(ptr)) > 0; ptr = NEXT_BLK(ptr))
    {
        if (SIZE(HDR(ptr)) != SIZE(FTR(ptr))) // check blk size of hdr and ftr
        {
            printf("block at %p size not same\n", ptr);
            tag = 1;
        }
        if (ALLOC(HDR(ptr)) != ALLOC(FTR(ptr))) // check blk alloc of hdr and ftr
        {
            printf("block at %p alloc not same\n", ptr);
            tag = 1;
        }
        if (!ALLOC(HDR(NEXT_BLK(ptr))) && !ALLOC(HDR(ptr))) // not merge 2 free blk
        {
            printf("free block at %p not merge with next block %p\n", ptr, NEXT_BLK(ptr));
            tag = 1;
        }
        if (!ALLOC(HDR(ptr))) // all free block in free list ?
        {
            int index = get_index_by_size(SIZE(HDR(ptr)));
            int find = 0;
            for (void *tmp_ptr = list[index]; tmp_ptr != NULL; tmp_ptr = NEXT_FREE(tmp_ptr))
            {
                if (tmp_ptr == ptr)
                {
                    find = 1;
                    break;
                }
            }
            if (!find)
            {
                printf("free block %p not in free list[%d]\n", ptr, index);
                printfreelist();
                tag = 1;
            }
        }
    }
    // print all block
    // printblk();
    // printf("\n");

    if (tag)
    {
        exit(0);
    }
    return 1; // consistent
}

void printfreelist()
{
    printf("the free lists:\n");
    for (size_t i = 0; i < ARR_SIZE; i++)
    {
        if (list[i] != NULL)
        {
            printf("index: %d\t", i);
            for (void *ptr = list[i]; ptr != NULL; ptr = NEXT_FREE(ptr))
            {
                printf("%p:", ptr);
                printf("%d ", SIZE(HDR(ptr)));
                printf("p: %p ", PREV_FREE(ptr));
                printf("n: %p | ", NEXT_FREE(ptr));
            }
            printf("\n");
        }
    }
    printf("\n");
}

void printblk()
{
    printf("print all block:\n");
    for (char *ptr = list_hdr; 1; ptr = NEXT_BLK(ptr))
    {
        if (SIZE(HDR(ptr)) <= 0)
        {
            printf("%p: %d %c ", ptr, SIZE(HDR(ptr)), ALLOC(HDR(ptr)) ? 'A' : 'F');
            break;
        }
        else
        {
            printf("%p: %d %c ", ptr, SIZE(HDR(ptr)), ALLOC(HDR(ptr)) ? 'A' : 'F');
            printf("%d %c | ", SIZE(FTR(ptr)), ALLOC(FTR(ptr)) ? 'A' : 'F');
        }
    }
    printf("\n");
}