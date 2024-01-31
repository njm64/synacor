#include <stdio.h>
#include <stdlib.h>

const int MAP_WIDTH = 4;
const int MAP_HEIGHT = 4;

const int OP_ADD = 100;
const int OP_SUB = 200;
const int OP_MUL = 300;

typedef struct State State;

struct State {
  int x, y, weight;
  State* prev;
  State* qnext;
};

static State* queueHead;
static State* queueTail;

static int gameMap[] = {
  OP_MUL, 8,      OP_SUB, 1,
  4,      OP_MUL, 11,     OP_MUL,
  OP_ADD, 4,      OP_SUB, 18,
  22,     OP_SUB, 9,      OP_MUL
};

static int getToken(int x, int y) {
  return gameMap[x + MAP_WIDTH * y];
}

static void push(State* s) {
  if(queueTail) {
    queueTail->qnext = s;
  } else {
    queueHead = s;
  }
  queueTail = s;
}

static State* pop() {
  State* s = queueHead;
  queueHead = s->qnext;
  if(!queueHead) {
    queueTail = NULL;
  }
  return s;
}

static State* solve() {

  // Push the initial state
  State* s = malloc(sizeof(State));
  s->x = 0;
  s->y = 3;
  s->weight = 22;
  s->prev = NULL;
  s->qnext = NULL;
  push(s);

  while(queueHead) {
    s = pop();

    // If we've reached the goal, check to see if the weight is 30

    if(s->x == 3 && s->y == 0) {
      if(s->weight == 30) {
        return s;
      } else {
        continue;
      }
    }

    // Otherwise find the neighbours
    
    for(int x = s->x - 1; x <= s->x + 1; x++) {
      for(int y = s->y - 1; y <= s->y + 1; y++) {

        // Exclude the current position
        if(x == s->x && y == s->y) {
          continue;
        }

        // Exclude diagonals
        if(x != s->x && y != s->y) {
          continue;
        }

        // Make sure they're in range
        if(x < 0 || y < 0 || x >= MAP_WIDTH || y >= MAP_HEIGHT) {
          continue;
        }

        // Can't move back to the start location
        if(x == 0 && y == 3) {
          continue;
        }

        int weight = s->weight;
        int token = getToken(x, y);
        int prevToken = getToken(s->x, s->y);

        switch(prevToken) {
          case OP_ADD:
            weight += token;
            break;
          case OP_SUB:
            weight -= token;
            break;
          case OP_MUL:
            weight *= token;
            break;
          default:
            break;
        }

        // Push a new state
        State* n = malloc(sizeof(State));
        n->x = x;
        n->y = y;
        n->weight = weight;
        n->prev = s;
        n->qnext = NULL;
        push(n);
      }
    }
  }

  return NULL;
}

int main() {
  State* s = solve();

  // Just print the nodes in reverse order
  while(s) {
    int tok = getToken(s->x, s->y);
    switch(tok) {
      case OP_ADD:
        printf("+\n");
        break;
      case OP_SUB:
        printf("-\n");
        break;
      case OP_MUL:
        printf("*\n");
        break;
      default:
        printf("%d\n", tok);
        break;
    }
    s = s->prev;
  }
  return 0;
}
