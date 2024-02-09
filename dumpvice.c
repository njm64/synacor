#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define OFFSET_IP   0x1e
#define OFFSET_SP   0x20
#define OFFSET_REG  0x0e
#define OFFSET_MEM  0x1000
#define OFFSET_STACK 0xC100

static char*  g_data;
static size_t g_size;

bool readData() {
  FILE* f = fopen("vice.dump", "rb");
  if(!f) {
    printf("Failed to open vice.dump\n");
    return false;
  }

  fseek(f, 0, SEEK_END);
  g_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  g_data = malloc(g_size);
  if(!fread(g_data, g_size, 1, f)) {
    printf("Failed to read data\n");
    return 1;
  }

  fclose(f);
  return true;
}

void hexDump(uint16_t* data, size_t size) {
  int lineSize = 16;
  for(int i = 0; i < size; i += lineSize) {
    printf("%04X:", i);
    for(int j = 0; j < lineSize && i + j < size; j++) {
      printf(" %04X", data[i+j]);
    }
    printf("\n");
  }
}

int main() {
  if(!readData()) {
    return 1;
  }

  char* p = memmem(g_data, g_size, "C64MEM", 6);
  if(!p) {
    printf("C64MEM module not found");
    return 1;
  }

  char* ram = p + 26;
  uint16_t ip = *(uint16_t*)(ram + OFFSET_IP);
  uint8_t sp = *(uint8_t*)(ram + OFFSET_SP) / 2;
  printf("IP: %04X SP: %04X\n", ip, sp);
  for(int i = 0; i < 8; i++) {
    printf("R%d: %04X ", i, *(uint16_t*)(ram + OFFSET_REG + i * 2));
  }
  printf("\n");
  printf("Stack:\n");
  hexDump((uint16_t*)(ram + OFFSET_STACK), sp);
  printf("Memory:\n");
  hexDump((uint16_t*)(ram + OFFSET_MEM), 16384);
  return 0;
}


