#include <stdio.h>
#include "common.h"

static Memory g_mem;

int main() {
  if(!memRead("challenge.bin", g_mem)) {
    printf("Failed to read challenge.bin\n");
    return 1;
  }

  memDecrypt(g_mem);

  if(!memWrite("decrypted.bin", g_mem)) {
    printf("Failed to write decrypted.bin\n");
    return 1;
  }

  return 0;
}

