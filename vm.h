#ifndef VM_H
#define VM_H

#include <stdint.h>

typedef int64_t Word;

typedef enum {
  INST_PUSH,
  INST_PLUS,
} Inst_t;

typedef struct {
  Inst_t type;
  Word operand;
} Inst;

#define VM_STACK_CAP 512

typedef struct {
  Inst stack[VM_STACK_CAP];
  Word stack_count;

  Word ip;
} Vm;

#endif
