#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void) {
  char *x = "1234";
  pid_t t = (pid_t)atoi(x);
  printf("%d\n", t);
}