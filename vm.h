#ifndef VM_H
#define VM_H

#include "table.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
  INST_PUSH,
  INST_POP,
  INST_PLUS,
  INST_PLUSF,
  INST_MINUS,
  INST_MULT,
  INST_DIV,
  INST_PRINT,
  INST_NEGATE,
  INST_DEF_GLOBAL,
  INST_DEF_LOCAL,
  INST_VAR_GLOBAL,
  INST_VAR_LOCAL,
  INST_JMP_ABS,
  INST_EOF
} Inst_t;

typedef struct {
  Inst_t type;
  Word operand;
} Inst;

typedef struct {
  int has_operand;
  Word_t operand_type;
} Inst_Context;

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
#define MAKE_PLUSF                                                             \
  (Inst) { .type = INST_PLUSF }
#define MAKE_MINUS                                                             \
  (Inst) { .type = INST_MINUS }
#define MAKE_MULT                                                              \
  (Inst) { .type = INST_MULT }
#define MAKE_DIV                                                               \
  (Inst) { .type = INST_DIV }
#define MAKE_NEGATE                                                            \
  (Inst) { .type = INST_NEGATE }
#define MAKE_DEF_GLOBAL(label)                                                 \
  (Inst) {                                                                     \
    .type = INST_DEF_GLOBAL, .operand = {.as_sv = label }                      \
  }
#define MAKE_DEF_LOCAL(offset)                                                 \
  (Inst) {                                                                     \
    .type = INST_DEF_LOCAL, .operand = {.as_u64 = offset }                     \
  }
#define MAKE_VAR_GLOBAL(name)                                                  \
  (Inst) {                                                                     \
    .type = INST_VAR_GLOBAL, .operand = {.as_sv = name }                       \
  }
#define MAKE_VAR_LOCAL(offset)                                                 \
  (Inst) {                                                                     \
    .type = INST_VAR_LOCAL, .operand = {.as_u64 = offset }                     \
  }
#define MAKE_PRINT                                                             \
  (Inst) { .type = INST_PRINT }
#define MAKE_JMP_ABS(offset)                                                   \
  (Inst) {                                                                     \
    .type = INST_JMP_ABS, .operand = {.as_u64 = offset }                       \
  }
#define MAKE_EOF                                                               \
  (Inst) { .type = INST_EOF }

void vm_init(void);
void vm_destruct(void);
void vm_program_load_from_memory(Inst *insts, size_t insts_count);
void vm_execute(void);
char *vm_inst_t_to_str(Inst_t type);
Word vm_env_resolve(const Sv name);

void vm_stack_dump(void);
void vm_program_dump(void);

#endif
