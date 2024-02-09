#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

#define MEM_SIZE    32768
#define REG_COUNT   8
#define STACK_SIZE  65536

typedef unsigned short Word;
typedef Word Memory[MEM_SIZE];

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

typedef struct {
  int operandCount;
  const char* name;
} OpDef;

bool    memRead(const char* filename, Memory m);
bool    memWrite(const char* filename, Memory m);
void    memDecrypt(Memory m);
OpDef*  getOpDef(Word opcode);
int     disasm(Memory mem, Word ip);

#endif
