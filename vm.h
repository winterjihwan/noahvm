#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>

typedef int64_t Word;

typedef enum {
  INST_PUSH,
  INST_POP,
  INST_PLUS,
  INST_MINUS,
  INST_MULT,
  INST_DIV,
  INST_EOF,
  INST_PRINT
} Inst_t;

typedef struct {
  Inst_t type;
  Word operand;
} Inst;

#define VM_STACK_CAP 512
#define PROGRAM_STACK_CAP 128

typedef struct {
  Inst program[PROGRAM_STACK_CAP];
  Word program_size;
  Word ip;

  Word stack[VM_STACK_CAP];
  Word stack_count;
} Vm;

#define MAKE_PUSH(val)                                                         \
  (Inst) { .type = INST_PUSH, .operand = val }
#define MAKE_PLUS                                                              \
  (Inst) { .type = INST_PLUS }
#define MAKE_MINUS                                                             \
  (Inst) { .type = INST_MINUS }
#define MAKE_MULT                                                              \
  (Inst) { .type = INST_MULT }
#define MAKE_DIV                                                               \
  (Inst) { .type = INST_DIV }
#define MAKE_EOF                                                               \
  (Inst) { .type = INST_EOF }

void vm_init(void);
void vm_program_load_from_memory(Inst *insts, size_t insts_count);
void vm_execute(void);
char *vm_inst_t_to_str(Inst_t type);

void vm_stack_dump(void);
void vm_program_dump(void);

#endif
