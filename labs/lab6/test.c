#include <stdio.h>
#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

int main(void) {
  printf("%lu\n", SIZE_T_SIZE);
  int i;
  for (i = 0; i < 20; i++) {
    printf("%d: %d\n", i, ALIGN(i));
  }
}
