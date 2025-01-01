#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>

typedef int64_t Word;

typedef enum { INST_PUSH, INST_POP, INST_PLUS, INST_EOF, INST_PRINT } Inst_t;

typedef struct {
  Inst_t type;
  Word operand;
} Inst;

#define VM_STACK_CAP 512
#define PROGRAM_STACK_CAP 128

typedef struct {
  Inst program[PROGRAM_STACK_CAP];
  Word ip;

  Word stack[VM_STACK_CAP];
  Word stack_count;
} Vm;

void vm_init(void);
void vm_program_load_from_memory(Inst *insts, size_t insts_count);
void vm_execute(void);

#endif
