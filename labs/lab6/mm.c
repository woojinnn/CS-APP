#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)
#define MIN_BLOCK_SIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned)(val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define NEXT_FP(bp) ((int *)(bp))
#define PREV_FP(bp) ((int *)((char *)(bp) + WSIZE))

#define NEXT_FP_CONTENT(bp) ((void *)*(int *)(bp))
#define PREV_FP_CONTENT(bp) ((void *)*(int *)((char *)(bp) + WSIZE))

#define SEG_CLASS(size) (MIN(MSB(size), SEGLIST_CLASSES) - 1)
#define SEGLIST_CLASSES 32
#define SEGLIST_ROOT(class) (heap_listp + (class * MIN_BLOCK_SIZE))

static void *extend_heap(size_t size);
static void place(void *ptr, size_t size);
static void *find_fit(size_t size);
static void *coalesce(void *ptr);

static void delete_node(void *ptr);
static void insert_node(void *ptr);
static int MSB(int size);

char *heap_listp;

int mm_init(void) {
  if ((heap_listp = mem_sbrk(DSIZE + (SEGLIST_CLASSES * MIN_BLOCK_SIZE))) ==
      (void *)-1)
    return -1;

  PUT(heap_listp, 0); // alignment padding

  // prologue
  PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
  PUT(heap_listp + DSIZE, PACK(DSIZE, 1));
  heap_listp += DSIZE;

  // seglist
  for (int i = 0; i < SEGLIST_CLASSES; i++) {
    char *segroot = SEGLIST_ROOT(i);
    PUT(HDRP(segroot), PACK(MIN_BLOCK_SIZE, 1));
    PUT(segroot, NULL);
    PUT(FTRP(segroot), PACK(MIN_BLOCK_SIZE, 1));
  }

  // epilogue
  PUT(SEGLIST_ROOT(SEGLIST_CLASSES) + WSIZE, PACK(0, 1));

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    return -1;

  return 0;
}

// Malloc
void *mm_malloc(size_t size) {
  size_t asize;
  size_t extend_size;
  char *bp;

  if (size == 0)
    return NULL;

  if (size <= DSIZE)
    asize = MIN_BLOCK_SIZE;
  else
    asize = ALIGN(size + DSIZE);

  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  extend_size = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extend_size / WSIZE)) == NULL)
    return NULL;

  place(bp, asize);
  return bp;
}

// free
void mm_free(void *ptr) {
  size_t size = GET_SIZE(HDRP(ptr));

  PUT(HDRP(ptr), PACK(size, 0));
  PUT(FTRP(ptr), PACK(size, 0));
  coalesce(ptr);
}

void *mm_realloc(void *ptr, size_t size) {
  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }

  if (ptr == NULL)
    return mm_malloc(size);

  void *prev = PREV_BLKP(ptr);
  int prev_alloc = GET_ALLOC(HDRP(prev));
  size_t prev_size = GET_SIZE(HDRP(prev));

  void *next = NEXT_BLKP(ptr);
  int next_alloc = GET_ALLOC(HDRP(next));
  size_t next_size = GET_SIZE(HDRP(next));

  size_t curr_size = GET_SIZE(HDRP(ptr));
  void *tmp;

  void *new_ptr = ptr;
  size_t new_size;
  if (size <= DSIZE)
    new_size = 2 * DSIZE;
  else
    new_size = ALIGN(DSIZE + size);

  if (new_size <= curr_size) {
    place(ptr, new_size);
    return ptr;
  }

  if ((!next_alloc) && (curr_size + next_size > new_size)) {
    delete_node(next);
    PUT(HDRP(ptr), PACK(curr_size + next_size, 1));
    PUT(FTRP(ptr), PACK(curr_size + next_size, 1));
    place(ptr, curr_size + next_size);
    return ptr;
  }

  else if (!prev_alloc && (prev_size > curr_size)) {
    delete_node(prev);

    PUT(HDRP(prev), PACK(new_size, 1));
    memcpy(prev, ptr, MIN(new_size, curr_size));
    PUT(FTRP(prev), PACK(new_size, 1));

    if ((prev_size + curr_size) >= (new_size + 2 * DSIZE)) {
      tmp = NEXT_BLKP(prev);
      PUT(HDRP(tmp), PACK(prev_size + curr_size - new_size, 0));
      PUT(FTRP(tmp), PACK(prev_size + curr_size - new_size, 0));
      coalesce(tmp);
    }
    return prev;
  }

  new_ptr = mm_malloc(size);
  if (new_ptr == NULL)
    return NULL;

  memcpy(new_ptr, ptr, MIN(size, curr_size));
  mm_free(ptr);
  return new_ptr;
}

static void *extend_heap(size_t words) {
  char *bp;

  size_t size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((bp = mem_sbrk(size)) == (void *)-1)
    return NULL;

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

  return coalesce(bp);
}

static void *coalesce(void *bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc) {
    insert_node(bp);
    return bp;
  }

  else if (prev_alloc && !next_alloc) {
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    delete_node(NEXT_BLKP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    insert_node(bp);
  }

  else if (!prev_alloc && next_alloc) {
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    delete_node(PREV_BLKP(bp));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insert_node(bp);
  }

  else {
    size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp))));
    delete_node(PREV_BLKP(bp));
    delete_node(NEXT_BLKP(bp));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    bp = PREV_BLKP(bp);
    insert_node(bp);
  }

  return bp;
}

// First-fit
static void *find_fit(size_t size) {
  int *ptr;
  int seg_class = SEG_CLASS(size);

  for (int class = seg_class; class < SEGLIST_CLASSES; class ++) {
    ptr = NEXT_FP_CONTENT(SEGLIST_ROOT(class));
    while (ptr != NULL) {
      if (size <= GET_SIZE(HDRP(ptr)))
        return ptr;

      ptr = NEXT_FP_CONTENT(ptr);
    }
  }
  return NULL;
}

static void place(void *bp, size_t asize) {
  size_t csize = GET_SIZE(HDRP(bp));

  if ((csize - asize) >= (MIN_BLOCK_SIZE)) {
    if (GET_ALLOC(HDRP(bp)) == 0) // don't require delete_node for realloc
      delete_node(bp);
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(csize - asize, 0));
    PUT(FTRP(bp), PACK(csize - asize, 0));
    insert_node(bp);
  }

  else {
    if (GET_ALLOC(HDRP(bp)) == 0)
      delete_node(bp);
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}

int MSB(int num) {
  int r = 0;
  while (num >>= 1)
    r++;
  return r;
}

// referred
// https://www.geeksforgeeks.org/insert-value-sorted-way-sorted-doubly-linked-list/
static void insert_node(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));
  void *prev = SEGLIST_ROOT(SEG_CLASS(size));

  // sorted doubly linked list for optimization
  while ((NEXT_FP_CONTENT(prev) != NULL) &&
         (GET_SIZE(HDRP(NEXT_FP_CONTENT(prev))) < size))
    prev = NEXT_FP_CONTENT(prev);

  void *next = NEXT_FP_CONTENT(prev);
  if (next != NULL)
    PUT(PREV_FP(next), bp);
  PUT(NEXT_FP(bp), next);
  PUT(PREV_FP(bp), prev);
  PUT(NEXT_FP(prev), bp);
}

static void delete_node(void *bp) {
  void *next = NEXT_FP_CONTENT(bp);
  void *prev = PREV_FP_CONTENT(bp);
  PUT(NEXT_FP(prev), next);
  if (next != NULL)
    PUT(PREV_FP(next), prev);
  return;
}