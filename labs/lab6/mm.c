#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))

#define GET_P(p) ((char *)*(unsigned int *)(p))
#define PUT_P(p, val) (*(unsigned int *)(p) = (unsigned)(val))

#define NEXTRP(bp) ((char *)(bp) + WSIZE)
#define PREVRP(bp) ((char *)(bp))

#define NEXT_FREE(bp) (GET_P((char *)(bp) + WSIZE))
#define PREV_FREE(bp) (GET_P((char *)(bp)))

#define SET_PREV(bp, val) (PUT_P(PREVRP(bp), val));
#define SET_NEXT(bp, val) (PUT_P(NEXTRP(bp), val));

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

static void delete_node(void *bp);
static void insert_node(void *bp);

static char *heap_listp;
static char *free_listp_start = NULL;
static char *free_listp_end = NULL;

int mm_init(void) {
  if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1)
    return -1;

  PUT(heap_listp, 0);

  PUT(heap_listp + (1 * WSIZE), PACK(16, 1));
  PUT_P(heap_listp + (2 * WSIZE), NULL);
  PUT_P(heap_listp + (3 * WSIZE), NULL);
  PUT(heap_listp + (4 * WSIZE), PACK(16, 1));

  PUT(heap_listp + (5 * WSIZE), PACK(0, 1));
  heap_listp += (2 * WSIZE);
  free_listp_start = heap_listp;

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    return -1;

  return 0;
}

static void *extend_heap(size_t words) {
  char *bp;
  size_t size = ALIGN(words) * WSIZE;

  if ((long)(bp = mem_sbrk(size)) == -1)
    return NULL;

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

  return coalesce(bp);
}

void *mm_malloc(size_t size) {
  size_t asize;
  size_t extendsize;
  char *bp;

  if (size == 0)
    return NULL;

  if (size <= DSIZE)
    asize = 2 * DSIZE;
  else
    asize = ALIGN(DSIZE + size);

  if ((bp = find_fit(asize)) != NULL) {
    place(bp, asize);
    return bp;
  }

  extendsize = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
    return NULL;

  place(bp, asize);

  return bp;
}

void mm_free(void *bp) {

  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  coalesce(bp);
}

void *mm_realloc(void *bp, size_t size) {
  if (size == 0) {
    mm_free(bp);
    return NULL;
  }

  if (bp == NULL) {
    return mm_malloc(size);
  }

  void *prev = PREV_BLKP(bp);
  void *next = NEXT_BLKP(bp);

  int prev_alloc = GET_ALLOC(HDRP(prev));
  int next_alloc = GET_ALLOC(HDRP(next));

  size_t prev_size = GET_SIZE(HDRP(prev));
  size_t curr_size = GET_SIZE(HDRP(bp));
  size_t next_size = GET_SIZE(HDRP(next));

  void *tmp;

  if (size <= DSIZE)
    size = 2 * DSIZE;
  else
    size = ALIGN(DSIZE + size);

  // can accomodate within current block
  if (curr_size >= size + DSIZE) {
    if (curr_size - size - DSIZE >= 2 * DSIZE) {

      if ((curr_size - size) > (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
        PUT(HDRP(next), PACK(curr_size - size, 0));
        PUT(FTRP(next), PACK(curr_size - size, 0));
        coalesce(next);
        return bp;
      }
    }
    return bp;
  }

  else {
    if (next_alloc == 0 && (next_size + curr_size >= size + DSIZE)) {
      delete_node(next);

      if ((next_size + curr_size) < (size + 2 * DSIZE))
        size = curr_size + next_size;

      PUT_P(HDRP(bp), PACK(size, 1));
      PUT_P(FTRP(bp), PACK(size, 1));

      if ((next_size + curr_size) >= (size + 2 * DSIZE)) {
        tmp = NEXT_BLKP(bp);
        PUT_P(HDRP(tmp), PACK(next_size + curr_size - size, 0));
        PUT_P(FTRP(tmp), PACK(next_size + curr_size - size, 0));
        coalesce(tmp);
      }
      return bp;
    }

    else if (prev_alloc == 0 && (prev_size + curr_size >= size + DSIZE)) {
      delete_node(prev);

      if ((prev_size + curr_size) < (size + 2 * DSIZE))
        size = prev_size + curr_size;

      size_t copysize = MIN(size, curr_size);
      memmove(prev, bp, copysize);
      PUT_P(HDRP(prev), PACK(size, 1));
      PUT_P(FTRP(prev), PACK(size, 1));

      if ((prev_size + curr_size) >= (size + 2 * DSIZE)) {
        tmp = NEXT_BLKP(prev);
        PUT_P(HDRP(tmp), PACK(prev_size + curr_size - size, 0));
        PUT_P(FTRP(tmp), PACK(prev_size + curr_size - size, 0));
        coalesce(tmp);
      }
      return prev;
    }

    else if (prev_alloc == 0 && next_alloc == 0 &&
             prev_size + curr_size + next_size >= size + DSIZE) {
      delete_node(prev);
      delete_node(next);

      if ((prev_size + curr_size + next_size) < (size + 2 * DSIZE))
        size = prev_size + curr_size + next_size;

      size_t copysize = MIN(size, curr_size);
      memmove(prev, bp, copysize);
      PUT(HDRP(prev), PACK(size, 1));
      PUT(FTRP(prev), PACK(size, 1));

      if ((prev_size + curr_size + next_size) >= (size + 2 * DSIZE)) {
        tmp = NEXT_BLKP(prev);
        PUT(HDRP(tmp), PACK(prev_size + curr_size + next_size - size, 0));
        PUT(FTRP(tmp), PACK(prev_size + curr_size + next_size - size, 0));
        coalesce(tmp);
      }
      return prev;
    }

    else {
      size_t asize;
      size_t extendsize;

      if (size <= DSIZE)
        asize = 2 * DSIZE;
      else
        asize = ALIGN(DSIZE + size);

      if ((tmp = find_fit(asize)) != NULL) {
        place(tmp, asize);
        memcpy(tmp, bp, MIN(size, curr_size));
        mm_free(bp);
        return tmp;
      }

      extendsize = asize;
      if ((tmp = extend_heap(extendsize / WSIZE)) == NULL)
        return NULL;
      place(tmp, asize);
      memcpy(tmp, bp, MIN(size, curr_size));
      mm_free(bp);
      return tmp;

      // tmp = mm_malloc(size);
      // if (tmp == NULL)
      //   return NULL;

      // memcpy(tmp, bp, MIN(size, curr_size));
      // mm_free(bp);

      // return tmp;
    }
  }
}

static void *find_fit(size_t asize) {
  void *bp;

  for (bp = free_listp_start; bp != free_listp_end; bp = PREV_FREE(bp)) {
    if (GET_SIZE(HDRP(bp)) >= asize) {
      return bp;
    }
  }

  return NULL;
}

static void place(void *bp, size_t asize) {
  size_t csize = GET_SIZE(HDRP(bp));

  if ((csize - asize) >= (2 * DSIZE)) {
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    delete_node(bp);

    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(csize - asize, 0));
    PUT(FTRP(bp), PACK(csize - asize, 0));
    insert_node(bp);
  }

  else {
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
    delete_node(bp);
  }
}

static void delete_node(void *bp) {
  if (bp == free_listp_start && bp == free_listp_end) {
    free_listp_start = NULL;
    free_listp_end = NULL;
  }

  else if (bp == free_listp_start) {
    free_listp_start = PREV_FREE(bp);
    PUT_P(NEXTRP(free_listp_start), 0);
  } else if (bp == free_listp_end) {
    free_listp_end = NEXT_FREE(bp);
    PUT_P(PREVRP(free_listp_end), 0);
  }

  if (PREV_FREE(bp) != (char *)NULL)
    SET_NEXT(PREV_FREE(bp), NEXT_FREE(bp));

  if (NEXT_FREE(bp) != (char *)NULL)
    SET_PREV(NEXT_FREE(bp), PREV_FREE(bp));
}

static void insert_node(void *bp) {
  if (free_listp_start == NULL) {
    PUT_P(PREVRP(bp), 0);
    PUT_P(NEXTRP(bp), 0);
    free_listp_start = bp;
    free_listp_end = bp;
    return;
  }

  else {
    PUT_P(PREVRP(bp), free_listp_start);
    PUT_P(NEXTRP(bp), NULL);
    PUT_P(NEXTRP(free_listp_start), bp);
    free_listp_start = bp;
    return;
  }
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
    delete_node(NEXT_BLKP(bp));

    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    insert_node(bp);
  }

  else if (!prev_alloc && next_alloc) {
    delete_node(PREV_BLKP(bp));

    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

    insert_node(PREV_BLKP(bp));

    bp = PREV_BLKP(bp);
  }

  else {
    delete_node(PREV_BLKP(bp));
    delete_node(NEXT_BLKP(bp));
    insert_node(PREV_BLKP(bp));

    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));

    bp = PREV_BLKP(bp);
  }

  return bp;
}
