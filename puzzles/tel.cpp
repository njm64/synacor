// Brute force solver for the teleportation puzzle
#include <stdio.h>
#include <unordered_map>

typedef std::unordered_map<uint32_t, uint16_t> Cache;

static Cache cache;
static uint16_t r7;

static uint16_t calc(uint16_t r0, uint16_t r1);

// Memoisation wrapper for calc
static uint16_t memoCalc(uint16_t r0, uint16_t r1) {
  uint32_t key = (r0 << 16) | r1;
  Cache::iterator it = cache.find(key);
  if(it != cache.end()) {
    return it->second;
  }
  uint16_t val = calc(r0, r1);
  cache[key] = val;
  return val;
}

// Disassembled calculation function
static uint16_t calc(uint16_t r0, uint16_t r1) {

  // 17A1: OP_JT R0 17A9
  // 17A4: OP_ADD R0 R1 0001
  // 17A8: OP_RET 
  if(r0 == 0) {         
    return r1 + 1;      
  }

  // 17A9: OP_JT R1 17B6
  // 17AC: OP_ADD R0 R0 7FFF
  // 17B0: OP_SET R1 R7
  // 17B3: OP_CALL 17A1
  // 17B5: OP_RET
  if(r1 == 0) {         
    return memoCalc(r0 - 1, r7);       
  }

  // 17B6: OP_PUSH R0
  // 17B8: OP_ADD R1 R1 7FFF
  // 17BC: OP_CALL 17A1
  // 17BE: OP_SET R1 R0
  // 17C1: OP_POP R0
  r1 = memoCalc(r0, r1 - 1);    

  // 17C3: OP_ADD R0 R0 7FFF
  // 17C7: OP_CALL 17A1
  // 17C9: OP_RET
  return calc(r0 - 1, r1);
}

int main() {
  for(int i = 0; i < 32768; i++) {
    cache.clear();
    r7 = i;
    uint16_t ret = calc(4, 1);
    printf("%d: %d\n", i, ret);
    if(ret == 6) {
      printf("Done\n");
      break;
    }
  }
  return 0;
}

// 25734: 6
