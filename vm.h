#ifndef VM_H
#define VM_H

#include "table.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
  INST_PUSH,
  INST_POP,
  INST_PLUS,
  INST_MINUS,
  INST_MULT,
  INST_DIV,
  INST_PRINT,
  INST_NEGATE,
  INST_DEFINE,
  INST_VAR,
  INST_EOF
} Inst_t;

typedef struct {
  Inst_t type;
  Word operand;
} Inst;

#define VM_STACK_CAP 512
#define PROGRAM_STACK_CAP 128

typedef uint64_t Addr;

typedef struct {
  Inst program[PROGRAM_STACK_CAP];
  uint64_t program_size;
  Addr ip;

  Word stack[VM_STACK_CAP];
  uint64_t stack_count;

  Hash_Table env;
} Vm;

#define MAKE_PUSH(word)                                                        \
  (Inst) { .type = INST_PUSH, .operand = word }
#define MAKE_POP                                                               \
  (Inst) { .type = INST_POP }
#define MAKE_PLUS                                                              \
  (Inst) { .type = INST_PLUS }
#define MAKE_MINUS                                                             \
  (Inst) { .type = INST_MINUS }
#define MAKE_MULT                                                              \
  (Inst) { .type = INST_MULT }
#define MAKE_DIV                                                               \
  (Inst) { .type = INST_DIV }
#define MAKE_NEGATE                                                            \
  (Inst) { .type = INST_NEGATE }
#define MAKE_DEFINE(label)                                                     \
  (Inst) {                                                                     \
    .type = INST_DEFINE, .operand = {.as_sv = label }                          \
  }
#define MAKE_VAR(label)                                                        \
  (Inst) {                                                                     \
    .type = INST_VAR, .operand = {.as_sv = label }                             \
  }
#define MAKE_PRINT                                                             \
  (Inst) { .type = INST_PRINT }
#define MAKE_EOF                                                               \
  (Inst) { .type = INST_EOF }

void vm_init(void);
void vm_destruct(void);
void vm_program_load_from_memory(Inst *insts, size_t insts_count);
void vm_execute(void);
char *vm_inst_t_to_str(Inst_t type);
Word vm_env_resolve(const Sv label);

void vm_stack_dump(void);
void vm_program_dump(void);

#endif
