#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

typedef unsigned short Word;

#define MEM_SIZE    32768
#define REG_COUNT   8
#define STACK_SIZE  65536

//#define TRACE(NAME, COUNT) trace(vm, NAME, COUNT)
#define TRACE(NAME, COUNT)

typedef struct{
  Word mem[MEM_SIZE];
  Word reg[REG_COUNT];
  Word stack[STACK_SIZE];
  size_t sp, ip;
  bool halt;
} VM;

typedef enum {
  OP_HALT   = 0,
  OP_SET    = 1,
  OP_PUSH   = 2,
  OP_POP    = 3,
  OP_EQ     = 4,
  OP_GT     = 5,
  OP_JMP    = 6,
  OP_JT     = 7,
  OP_JF     = 8,
  OP_ADD    = 9,
  OP_MULT   = 10,
  OP_MOD    = 11,
  OP_AND    = 12,
  OP_OR     = 13,
  OP_NOT    = 14,
  OP_RMEM   = 15,
  OP_WMEM   = 16,
  OP_CALL   = 17,
  OP_RET    = 18,
  OP_OUT    = 19,
  OP_IN     = 20,
  OP_NOP    = 21  
} OP;

static bool readProgram(const char* filename, VM* vm) {
  FILE* f = fopen(filename, "rb");
  if(!f) {
    return false;
  }
 
  bool done = false, err = false;
  for(int addr = 0; !done && !err; addr++) {
    int lo = fgetc(f);
    int hi = fgetc(f);
    if(hi >= 0 && addr < MEM_SIZE) {
      vm->mem[addr] = (hi << 8) | lo;
    } else if(feof(f)) {
      done = true;
    } else {
      printf("hi %d addr %d\n", (int)hi, (int)addr);
      err = true;
    }
  }

  fclose(f);
  return !err;
}

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

static void trace(VM* vm, const char* op, int numOperands) {
  Word ip = vm->ip - 1;
//  printRegisters(vm);
  printf("%04X: %s", ip, op);
  for(int i = 0; i < numOperands; i++) {
    Word v = vm->mem[vm->ip + i];
    if(v < MEM_SIZE) {
      printf(" %04X", v);
    } else if(v < MEM_SIZE + REG_COUNT) {
      printf(" R%d", v - MEM_SIZE);
    } else {
      printf(" INVALID");
    }
  }
  printf("\n");
}

static void step(VM* vm) {
  OP op = fetchWord(vm);
  Word a, b, c;
  switch(op) {
    case OP_HALT:
      TRACE("OP_HALT", 0);
      vm->halt = true;
      break;
    case OP_SET:
      TRACE("OP_SET", 2);
      a = fetchReg(vm);
      b = fetchVal(vm);
      vm->reg[a] = b;
      break;
    case OP_PUSH:
      TRACE("OP_PUSH", 1);
      a = fetchVal(vm);
      push(vm, a);
      break;
    case OP_POP:
      TRACE("OP_POP", 1);
      a = fetchReg(vm);
      b = pop(vm);
      vm->reg[a] = b;
      break;
    case OP_EQ:
      TRACE("OP_EQ", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b == c;
      break;
    case OP_GT:
      TRACE("OP_GT", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b > c;
      break;
    case OP_JMP:
      TRACE("OP_JMP", 1);
      a = fetchVal(vm);
      jump(vm, a);
      break;
    case OP_JT:
      TRACE("OP_JT", 2);
      a = fetchVal(vm);
      b = fetchVal(vm);
      if(a) {
        jump(vm, b);
      }
      break;
    case OP_JF:
      TRACE("OP_JF", 2);
      a = fetchVal(vm);
      b = fetchVal(vm);
      if(!a) {
        jump(vm, b);
      }
      break;
    case OP_ADD:
      TRACE("OP_ADD", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = (b + c) % 0x8000;
      break;
    case OP_MULT:
      TRACE("OP_MULT", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = (b * c) % 0x8000;
      break;
    case OP_MOD:
      TRACE("OP_MOD", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b % c;
      break;
    case OP_AND:
      TRACE("OP_AND", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b & c;
      break;
    case OP_OR:
      TRACE("OP_OR", 3);
      a = fetchReg(vm);
      b = fetchVal(vm);
      c = fetchVal(vm);
      vm->reg[a] = b | c;
      break;
    case OP_NOT:
      TRACE("OP_NOT", 2);
      a = fetchReg(vm);
      b = fetchVal(vm);
      vm->reg[a] = (~b) & 0x7FFF;
      break;
    case OP_RMEM:
      TRACE("OP_RMEM", 2);
      a = fetchReg(vm);
      b = fetchVal(vm);
      vm->reg[a] = vm->mem[b];
      break;
    case OP_WMEM:
      TRACE("OP_WMEM", 2);
      a = fetchVal(vm);               
      b = fetchVal(vm);
      vm->mem[a] = b;
      break;
    case OP_CALL:
      TRACE("OP_CALL", 1);
      a = fetchVal(vm);
      push(vm, vm->ip);
      jump(vm, a);
      break;
    case OP_RET:
      TRACE("OP_RET", 0);
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
      putchar(a);
      break;
    case OP_IN:
      TRACE("OP_IN", 1);
      a = fetchReg(vm);
      b = getchar() & 32767;
      vm->reg[a] = b;
      if(b == '\n') {
        // We write the complete game state to save.dat after line
        // of user input. Copy this file to load.dat to have the game
        // load it on startup.
        save(vm);
      }
      break;
    case OP_NOP:
      TRACE("OP_NOP", 0);
      break;
    default:
      fatalError("Unknown opcode %04x", op);
      break;
  }
}

int main() {
  VM vm;
  memset(&vm, 0, sizeof(vm));

  if(!readProgram("challenge.bin", &vm)) {
    printf("Error reading challenge.bin\n");
    return 1;
  }

  load(&vm);

  // Patch for the teleporter puzzle
  /*
  vm.reg[7] = 25734;
  for(Word addr = 0x1587; addr <= 0x158F; addr++) {
      vm.mem[addr] = OP_NOP;
  }
  */

  while(!vm.halt) {
    step(&vm);
  }

  return 0;
}

