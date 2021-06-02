#include <unistd.h>

void mem_init(void);
void mem_deinit(void);

/*
 * Expands the heap by incr bytes, where incr is a positive non-zero integer and
 * returns a generic pointer to the first byte of the newly allocated heap area.
 * The semantics are identical to the Unix sbrk function, except that mem_sbrk
 * accepts only a positive non-zero integer argument.
 */
void *mem_sbrk(int incr);

void mem_reset_brk(void);

/* Returns a generic pointer to the first byte in the heap. */
void *mem_heap_lo(void);

/* Returns a generic pointer to the last byte in the heap. */
void *mem_heap_hi(void);

/* Returns the current size of the heap in bytes. */
size_t mem_heapsize(void);

/* Returns the system’s page size in bytes (4K on Linux systems). */
size_t mem_pagesize(void);