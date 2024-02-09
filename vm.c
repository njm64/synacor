#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "common.h"

typedef struct {
  Memory mem;
  Word reg[REG_COUNT];
  Word stack[STACK_SIZE];
  size_t sp, ip;
  bool halt;
} VM;

bool debug;

//#define STATS
#ifdef STATS
#define PAGE_SIZE 256
static int maxStack;
static bool pageMap[MEM_SIZE / PAGE_SIZE];
static int instructionCount;
#endif

static void save(VM* vm) {
  FILE* f = fopen("save.dat", "wb");
  if(f) {
    fwrite(vm, sizeof(VM), 1, f);
    fclose(f);
  }
}

static void load(VM* vm) {
  FILE* f = fopen("load.dat", "rb");
  if(f) {
    fread(vm, sizeof(VM), 1, f);
    fclose(f);
  }
}

static void hexDump(FILE* f, Word* data, int size) {
  int lineSize = 16;
  for(int i = 0; i < size; i += lineSize) {
    fprintf(f, "%04X:", i);
    for(int j = 0; j < lineSize && i + j < size; j++) {
      fprintf(f, " %04X", data[i+j]);
    }
    fprintf(f, "\n");
  }
}

static void dumpDebug(VM* vm) {
  FILE* f = fopen("dump.txt", "w");
  fprintf(f, "IP: %04X SP: %04X\n", (Word)vm->ip, (Word)vm->sp);
  for(int i = 0; i < REG_COUNT; i++) {
    fprintf(f, "R%d: %04X ", i, vm->reg[i]);
  }
  fprintf(f, "\n");

  fprintf(f, "Stack:\n");
  hexDump(f, vm->stack, vm->sp);

  fprintf(f, "Memory:\n");
  hexDump(f, vm->mem, 8192);
  fclose(f);
}


static void fatalError(const char* fmt, ...) {
  va_list v;
  va_start(v, fmt);
  vfprintf(stderr, fmt, v);
  va_end(v);
  exit(1);
}

static Word fetchWord(VM* vm) {
  if(vm->ip >= MEM_SIZE) {
      fatalError("IP %04X out of range", vm->ip);
  }
  return vm->mem[vm->ip++];
}

static Word fetchVal(VM* vm) {
  Word v = fetchWord(vm);
  if(v < MEM_SIZE) {
    return v;
  } else if(v < MEM_SIZE + REG_COUNT) {
    return vm->reg[v - MEM_SIZE] & 0x7FFF;
  } else {
    fatalError("Invalid value %02X", v);
    return 0;
  }
}

static Word fetchReg(VM* vm) {
  Word w = fetchWord(vm);
  if(w < MEM_SIZE && w > MEM_SIZE + REG_COUNT) {
    fatalError("Invalid register");
  }
  return w - MEM_SIZE;
}

static void jump(VM* vm, Word ip) {
  if(ip >= MEM_SIZE) {
    fatalError("Invalid jump %04X", ip);
  }
  vm->ip = ip;
}

static void push(VM* vm, Word w) {
  if(vm->sp == STACK_SIZE) {
    fatalError("Stack overflow");
  }
  vm->stack[vm->sp++] = w;
#ifdef STATS
  if(vm->sp > maxStack) {
    maxStack = vm->sp;
  }
#endif
}

static Word pop(VM* vm) {
  if(vm->sp == 0) {
    fatalError("Stack underflow");
  }
  return vm->stack[--vm->sp];
}

static void printRegisters(VM* vm) {
  for(int r = 0; r < REG_COUNT; r++) {
    printf("R%d %04X ", r, vm->reg[r]);
  }
}

#ifdef STATS
static void printStats() {
  int modifiedPages = 0;
  for(int i = 0; i < MEM_SIZE / PAGE_SIZE; i++) {
    if(pageMap[i]) {
      modifiedPages++;
    }
  }
  printf("Max Stack: %d Modified Pages: %d Instructions: %d\n",
      maxStack,
      modifiedPages,
      instructionCount);
}
#endif

static void step(VM* vm) {
  if(debug) {
    printRegisters(vm);
    printf("\n");
    printf("\n");
    disasm(vm->mem, vm->ip);
  }
#ifdef STATS
  instructionCount++;
#endif
  OP op = fetchWord(vm);
  Word a, b, c;
  switch(op) {
    case OP_HALT:
      vm->halt = true;
      break;
    case OP_SET:
      a = fetchReg(vm);
      b = fetchVal(vm);
      vm->reg[a] = b;
      break;
    case OP_PUSH:
      a = fetchVal(vm);
      push(vm, a);
      break;
    case OP_POP:
      a = fetchReg(vm);
      b = pop(vm);
      vm->reg[a] = b;
      break;
    case OP_EQ:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b == c;
      break;
    case OP_GT:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b > c;
      break;
    case OP_JMP:
      a = fetchVal(vm);
      jump(vm, a);
      break;
    case OP_JT:
      a = fetchVal(vm);
      b = fetchVal(vm);
      if(a) {
        jump(vm, b);
      }
      break;
    case OP_JF:
      a = fetchVal(vm);
      b = fetchVal(vm);
      if(!a) {
        jump(vm, b);
      }
      break;
    case OP_ADD:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = (b + c) % 0x8000;
      break;
    case OP_MULT:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = (b * c) % 0x8000;
      break;
    case OP_MOD:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b % c;
      break;
    case OP_AND:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b & c;
      break;
    case OP_OR:
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b | c;
      break;
    case OP_NOT:
      a = fetchReg(vm);
      b = fetchVal(vm);
      vm->reg[a] = (~b) & 0x7FFF;
      break;
    case OP_RMEM:
      a = fetchReg(vm);
      b = fetchVal(vm);
      if(b >= 16384) {
       // printf("%04X: Trying to read %04X\n", (Word)vm->ip - 3, b);
      }
      vm->reg[a] = vm->mem[b] & 0x7FFF;
      break;
    case OP_WMEM:
      a = fetchVal(vm);               
      b = fetchVal(vm);
      if(a >= 16384) {
       // printf("%04X: Trying to write %04X\n", (Word)vm->ip - 3, a);
      }
      vm->mem[a] = b;
      printf("%04X: Writing %04X to %04X\n", (Word)vm->ip - 3, b, a);
#ifdef STATS
      pageMap[a/PAGE_SIZE] = true;
#endif
      break;
    case OP_CALL:
      a = fetchVal(vm);
      push(vm, vm->ip);
      jump(vm, a);
      break;
    case OP_RET:
      if(vm->sp == 0) {
        vm->halt = true;
      } else {
        a = pop(vm);
        jump(vm, a);
      }
      break;
    case OP_OUT:
      //TRACE("OP_OUT", 1);
      a = fetchVal(vm);
      printf("{%02X}", a);
      putchar(a);
      break;
    case OP_IN:
      a = fetchReg(vm);
      b = getchar();
      if(feof(stdin)) {
        vm->halt = true;
        break;
      }
      vm->reg[a] = b;
      if(b == '\n') {
        // We write the complete game state to save.dat after line
        // of user input. Copy this file to load.dat to have the game
        // load it on startup.
        save(vm);
#ifdef STATS
        printStats();
        instructionCount = 0;
#endif
      }
      break;
    case OP_NOP:
      break;
    default:
      fatalError("Unknown opcode %04x", op);
      break;
  }
}

static void patchTeleporter(VM* vm) {

  // Patch the instruction at 1561. It checks to ensure that R7 is non-zero,
  // and if so, the teleporter code is skipped. Instead, we set R7 to the
  // correct value, as calculated by tel.cpp.
  // old: 1561: OP_JF R7 15FB
  // new: 1561: OP_SET R7 6486
  vm->mem[0x1561] = OP_SET;
  vm->mem[0x1563] = 0x6486;

  // Remove the call to the teleporter confirmation routine, and the check
  // that it returns the correct value (6).
  // 1587: OP_CALL 17A1
  // 1589: OP_EQ R1 R0 0006
  // 158D: OP_JF R1 15E1
  for(int addr = 0x1587; addr <= 0x158F; addr++) {
    vm->mem[addr] = OP_NOP;
  }
}

int main() {
  VM vm;
  memset(&vm, 0, sizeof(vm));

  if(!memRead("challenge.bin", vm.mem)) {
    printf("Error reading challenge.bin\n");
    return 1;
  }

  load(&vm);
//  patchTeleporter(&vm);
//  memDecrypt(vm.mem);
  while(!vm.halt) {
 /*   if(vm.ip == 1000) {
      debug = true;
    }
    if(vm.ip == 0x043e) {
      dumpDebug(&vm);
      break;
    }
    */

    step(&vm);
  }

  return 0;
}

