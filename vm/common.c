#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include "common.h"

static OpDef opDefs[] = {
  0, "halt",
  2, "set",
  1, "push",
  1, "pop",
  3, "eq",
  3, "gt",
  1, "jmp",
  2, "jt",
  2, "jf",
  3, "add",
  3, "mult",
  3, "mod",
  3, "and",
  3, "or",
  2, "not",
  2, "rmem",
  2, "wmem",
  1, "call",
  0, "ret",
  1, "out",
  1, "in",
  0, "nop",
};

bool memRead(const char* filename, Memory mem) {
  FILE* f = fopen(filename, "rb");
  if(!f) {
    return false;
  }
 
  bool done = false, err = false;
  for(int addr = 0; !done && !err; addr++) {
    int lo = fgetc(f);
    int hi = fgetc(f);
    if(hi >= 0 && addr < MEM_SIZE) {
      mem[addr] = (hi << 8) | lo;
    } else if(feof(f)) {
      done = true;
    } else {
      err = true;
    }
  }

  fclose(f);
  return !err;
}

bool memWrite(const char* filename, Memory mem) {
  FILE* f = fopen(filename, "wb");
  if(!f) {
    return false;
  }

  for(int i = 0; i < MEM_SIZE; i++) {
    fputc(mem[i] & 0xff, f);
    fputc(mem[i] >> 8, f);
  }

  fclose(f);
  return true;
}

void memDecrypt(Memory mem) {
  // Decrypt the encrypted section of the byte code.
  // This original routine is located at 06D1.
  for(Word addr = 0x17CA; addr < 0x7505; addr++) {
    Word old = mem[addr];
    mem[addr] = mem[addr] ^ (addr * addr) ^ 0x4154;
  }

  // Patch out the call to the original decrypt routine
  mem[0x038B] = OP_NOP;
  mem[0x038C] = OP_NOP;
}

OpDef* getOpDef(Word op) {
  if(op >= sizeof(opDefs) / sizeof(OpDef)) {
    return NULL;
  }
  return &opDefs[op];
}

int disasm(Memory mem, Word ip) {
  Word op = mem[ip];
  OpDef* def = getOpDef(op);

  if(!def) {
    printf("%04X: %04X\n", ip, op);
    return 1;
  }

  printf("%04X: %s", ip, def->name);

  if(op == OP_OUT) {
    char c = mem[ip + 1];
    if(c == '\n') {
      printf(" '\\n'\n");
    } else {
      printf(" '%c'\n", mem[ip + 1]);
    }
    return 2;
  }

  for(int i = 1; i <= def->operandCount; i++) {
    Word v = mem[ip + i];
    if(v < MEM_SIZE) {
      printf(" %04X", v);
    } else if(v < MEM_SIZE + REG_COUNT) {
      printf(" R%d", v - MEM_SIZE);
    } else {
      printf(" INVALID");
    }
  }
  printf("\n");
  return 1 + def->operandCount;
}

