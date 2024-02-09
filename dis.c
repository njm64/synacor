#include <stdio.h>
#include "common.h"

static Memory g_mem;

int main() {
  if(!memRead("challenge.bin", g_mem)) {
    printf("Failed to read challenge.bin\n");
    return 1;
  }

  memDecrypt(g_mem);

  // Just disassemble the whole thing. I started off following jumps and
  // calls, but realised that excluded the failure conditions from the
  // self test.
  int ip = 0;
  while(ip < MEM_SIZE) {
    ip += disasm(g_mem, ip);
  }

  return 0;
}

