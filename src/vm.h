#ifndef VM_H
#define VM_H

#include "table.h"
#include <stddef.h>
#include <stdint.h>

// TODO: Err handle
typedef enum {
  VM_STACK_OVERFLOW,
  VM_STACK_UNDERFLOW,
} Err;

typedef enum {
  INST_PUSH,
  INST_POP,
  INST_PLUS,
  INST_PLUSF,
  INST_MINUS,
  INST_MULT,
  INST_DIV,
  INST_EQ,
  INST_NE,
  INST_GT,
  INST_LT,
  INST_PRINT,
  INST_PRINTS,
  INST_NEG,
  INST_DEFG,
  INST_DEFL,
  INST_VARG,
  INST_VARL,
  INST_JMPA,
  INST_JMPT,
  INST_JMPNT,
  INST_RET,
  INST_LDR,
  INST_STR,
  INST_MOV,
  INST_LABEL,
  INST_EOF,
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
#define INSTS_CAP 256

typedef uint64_t Addr;

typedef struct {
  Inst program[INSTS_CAP];
  uint64_t program_size;

  Word stack[VM_STACK_CAP];
  uint64_t stack_count;

  Hash_Table env;

#define REG_IP 10
#define REG_FP 11
#define REG_SP 12
#define REG_RA 13
#define REG_RAX 14
#define REG_CPSR 15
  Word reg[16];
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
#define MAKE_EQ                                                                \
  (Inst) { .type = INST_EQ }
#define MAKE_NE                                                                \
  (Inst) { .type = INST_NE }
#define MAKE_GT                                                                \
  (Inst) { .type = INST_GT }
#define MAKE_LT                                                                \
  (Inst) { .type = INST_LT }
#define MAKE_NEG                                                               \
  (Inst) { .type = INST_NEG }
#define MAKE_DEFG(label)                                                       \
  (Inst) {                                                                     \
    .type = INST_DEFG, .operand = {.as_sv = label }                            \
  }
#define MAKE_DEFL(offset)                                                      \
  (Inst) {                                                                     \
    .type = INST_DEFL, .operand = {.as_u64 = offset }                          \
  }
#define MAKE_VARG(name)                                                        \
  (Inst) {                                                                     \
    .type = INST_VARG, .operand = {.as_sv = name }                             \
  }
#define MAKE_VARL(offset)                                                      \
  (Inst) {                                                                     \
    .type = INST_VARL, .operand = {.as_u64 = offset }                          \
  }
#define MAKE_PRINT                                                             \
  (Inst) { .type = INST_PRINT }
#define MAKE_PRINTS                                                            \
  (Inst) { .type = INST_PRINTS }
#define MAKE_JMPA(offset)                                                      \
  (Inst) {                                                                     \
    .type = INST_JMPA, .operand = {.as_u64 = offset }                          \
  }
#define MAKE_JMPT(offset)                                                      \
  (Inst) {                                                                     \
    .type = INST_JMPT, .operand = {.as_u64 = offset }                          \
  }
#define MAKE_JMPNT(offset)                                                     \
  (Inst) {                                                                     \
    .type = INST_JMPNT, .operand = {.as_u64 = offset }                         \
  }
#define MAKE_RET                                                               \
  (Inst) { .type = INST_RET }
#define MAKE_LDR(reg_no)                                                       \
  (Inst) {                                                                     \
    .type = INST_LDR, .operand = {.as_u64 = reg_no }                           \
  }
#define MAKE_STR(reg_no)                                                       \
  (Inst) {                                                                     \
    .type = INST_STR, .operand = {.as_u64 = reg_no }                           \
  }
#define MAKE_MOV(reg_no)                                                       \
  (Inst) {                                                                     \
    .type = INST_MOV, .operand = {.as_u64 = reg_no }                           \
  }
#define MAKE_LABEL(label)                                                      \
  (Inst) {                                                                     \
    .type = INST_LABEL, .operand = {.as_sv = label }                           \
  }
#define MAKE_EOF                                                               \
  (Inst) { .type = INST_EOF }

void vm_init(void);
void vm_destruct(void);
void vm_program_load_from_memory(Inst *insts, size_t insts_count);
void vm_execute(void);
char *vm_inst_t_to_str(Inst_t type);

void vm_inst_dump(const Inst *inst);
void vm_stack_dump(void);
void vm_program_dump(void);

#endif
