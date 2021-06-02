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
#define PUT_P(p, val) (*(unsigned int *)(p) = (int)(val))

#define NEXTRP(bp) ((char *)(bp) + WSIZE)
#define PREVRP(bp) ((char *)(bp))

#define NEXT_FREE_BLKP(bp) (GET_P((char *)(bp) + WSIZE))
#define PREV_FREE_BLKP(bp) (GET_P((char *)(bp)))

#define CHANGE_PREV(bp, val) (PUT_P(PREVRP(bp), val));
#define CHANGE_NEXT(bp, val) (PUT_P(NEXTRP(bp), val));

static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);

static void cut_link(void *bp);
static void push_first(void *bp);

static void seg_init(void);
static void *seg_find(int size);

static char *heap_listp;
static char *seg_listp;

/* makes segregated list */
void seg_init(void) {

  // assign space for segregated list
  if ((seg_listp = mem_sbrk(32 * WSIZE)) == (void *)-1)
    return;

  // initializee to NULL from 2^0 ~ 2^32
  for (int i = 0; i < 32; i++) {
    PUT_P(seg_listp + (i * WSIZE), NULL);
  }
}

/* Find segregaetd point that matches size */
static void *seg_find(int size) {
  static char *seg;

  int i = 0;
  // find proper value from 2^22
  for (i = 32; i > 0; i--) {
    if ((size & (1 << (i))) > 0) {
      break;
    }
  }

  // returns nth segreagetd address that matches 2^n
  seg = seg_listp + (i * WSIZE);

  return seg;
}

int mm_init(void) {
  seg_init();

  if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1)
    return -1;
  PUT(heap_listp, 0);

  PUT(heap_listp + (1 * WSIZE), PACK(2 * DSIZE, 1));
  PUT_P(heap_listp + (2 * WSIZE), NULL);
  PUT_P(heap_listp + (3 * WSIZE), NULL);
  PUT(heap_listp + (4 * WSIZE), PACK(2 * DSIZE, 1));

  PUT(heap_listp + (5 * WSIZE), PACK(0, 1));
  heap_listp += (2 * WSIZE);

  if (extend_heap(CHUNKSIZE / WSIZE) == NULL)
    return -1;

  return 0;
}

static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
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
    asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

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

static void *find_fit(size_t asize) {
  void *bp;
  void *best_bp = (char *)NULL;

  size_t best;

  static char *seg;

  int i = 0;
  for (i = 32; i > 0; i--) {
    if ((asize & (1 << (i))) > 0) {
      break;
    }
  }

  int j = i;
  for (j = i; j <= 32; j++) {
    seg = seg_listp + (j * WSIZE);
    if (GET_P(seg) != (char *)NULL) {
      best = (1 << (j + 1));
      for (bp = PREV_FREE_BLKP(seg); bp != (char *)NULL;
           bp = PREV_FREE_BLKP(bp)) {
        if (asize <= GET_SIZE(HDRP(bp)) && GET_SIZE(HDRP(bp)) - asize < best) {
          best_bp = bp;
          best = GET_SIZE(HDRP(bp)) - asize;
        }
      }
      if (best_bp != (char *)NULL) {
        return best_bp;
      }
    }
  }

  return NULL;
}
static void place(void *bp, size_t asize) {
  size_t csize = GET_SIZE(HDRP(bp));

  if ((csize - asize) >= (2 * DSIZE)) {
    cut_link(bp);
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));

    bp = NEXT_BLKP(bp);
    PUT(HDRP(bp), PACK(csize - asize, 0));
    PUT(FTRP(bp), PACK(csize - asize, 0));
    push_first(bp);

  } else {
    cut_link(bp);
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}

void mm_free(void *bp) {
  size_t size = GET_SIZE(HDRP(bp));

  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));

  coalesce(bp);
}

static void cut_link(void *bp) {
  if (PREV_FREE_BLKP(bp) != (char *)NULL) {
    CHANGE_NEXT(PREV_FREE_BLKP(bp), NEXT_FREE_BLKP(bp));
  }
  if (NEXT_FREE_BLKP(bp) != (char *)NULL) {
    CHANGE_PREV(NEXT_FREE_BLKP(bp), PREV_FREE_BLKP(bp));
  }
}

static void push_first(void *bp) {

  char *seg;
  seg = seg_find(GET_SIZE(HDRP(bp))); //

  if (PREV_FREE_BLKP(seg) != (char *)NULL) {
    CHANGE_NEXT(PREV_FREE_BLKP(seg), bp);
  }
  PUT_P(PREVRP(bp), PREV_FREE_BLKP(seg));
  PUT_P(NEXTRP(bp), seg);
  PUT_P(PREVRP(seg), bp);
}

static void *coalesce(void *bp) {
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  size_t size = GET_SIZE(HDRP(bp));

  if (prev_alloc && next_alloc) {
    push_first(bp);
    return bp;
  } else if (prev_alloc && !next_alloc) {
    cut_link(NEXT_BLKP(bp));

    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));

    push_first(bp);
  } else if (!prev_alloc && next_alloc) {
    cut_link(PREV_BLKP(bp));

    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

    push_first(PREV_BLKP(bp));

    bp = PREV_BLKP(bp);
  } else {
    cut_link(NEXT_BLKP(bp));
    cut_link(PREV_BLKP(bp));

    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));

    push_first(PREV_BLKP(bp));

    bp = PREV_BLKP(bp);
  }
  return bp;
}

void *mm_realloc(void *bp, size_t size) {
  char *old_dp = bp;
  char *new_dp;

  size_t old_size = GET_SIZE(HDRP(old_dp));
  size_t old_next_size = GET_SIZE(HDRP(NEXT_BLKP(old_dp)));

  if (!GET_ALLOC(HDRP(NEXT_BLKP(old_dp))) && old_size + old_next_size >= size) {
    size_t asize;

    if (size == 0)
      return NULL;

    if (size <= DSIZE)
      asize = 2 * DSIZE;
    else
      asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    cut_link(NEXT_BLKP(old_dp));

    if ((old_size + old_next_size - asize) >= (2 * DSIZE)) {
      PUT(HDRP(bp), PACK(asize, 1));
      PUT(FTRP(bp), PACK(asize, 1));

      new_dp = NEXT_BLKP(bp);
      PUT(HDRP(new_dp), PACK(old_size + old_next_size - asize, 0));
      PUT(FTRP(new_dp), PACK(old_size + old_next_size - asize, 0));
      push_first(new_dp);

    } else {
      PUT(HDRP(bp), PACK(old_size + old_next_size, 1));
      PUT(FTRP(bp), PACK(old_size + old_next_size, 1));
    }

    return bp;
  } else {
    size_t copySize;

    new_dp = mm_malloc(size);

    if (new_dp == NULL)
      return NULL;

    copySize = GET_SIZE(HDRP(old_dp));
    if (size < copySize)
      copySize = size;
    memcpy(new_dp, old_dp, copySize);

    mm_free(old_dp);

    return new_dp;
  }
}