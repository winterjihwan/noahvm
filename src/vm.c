#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "table.h"
#include "vm.h"

/*#define DEBUG*/

Vm vm = {0};

void vm_init(void) {
  vm.stack_count = 0;
  vm.program_size = 0;
  vm.env = hash_table_new();

  vm.reg[REG_IP].as_u64 = 0;
  vm.reg[REG_FP].as_u64 = 0;
  vm.reg[REG_SP].as_u64 = 0;
  vm.reg[REG_RA].as_u64 = 0;
  vm.reg[REG_CPSR].as_u64 = 0;
}

void vm_destruct(void) { hash_table_destruct(&vm.env); }

void vm_program_load_from_memory(Inst *insts, size_t insts_count) {
  assert(insts_count < PROGRAM_STACK_CAP);

  for (size_t i = 0; i < insts_count; i++) {
    vm.program[i] = insts[i];
  }

  vm.program_size = insts_count;
}

char *vm_inst_t_to_str(Inst_t type) {
  switch (type) {
  case INST_PUSH:
    return "\tpush";
  case INST_POP:
    return "\tpop";
  case INST_PLUS:
    return "\tplus";
  case INST_PLUSF:
    return "\tplusf";
  case INST_MINUS:
    return "\tminus";
  case INST_MULT:
    return "\tmult";
  case INST_DIV:
    return "\tdiv";
  case INST_EQ:
    return "\teq";
  case INST_NE:
    return "\tne";
  case INST_GT:
    return "\tgt";
  case INST_LT:
    return "\tlt";
  case INST_PRINT:
    return "\tprint";
  case INST_NEG:
    return "\tneg";
  case INST_DEFG:
    return "\tdefg";
  case INST_DEFL:
    return "\tdefl";
  case INST_VARG:
    return "\tvarg";
  case INST_VARL:
    return "\tvarl";
  case INST_JMPA:
    return "\tjmpa";
  case INST_JMPT:
    return "\tjmpt";
  case INST_JMPNT:
    return "\tjmpnt";
  case INST_RET:
    return "\tret";
  case INST_LDR:
    return "\tldr";
  case INST_STR:
    return "\tstr";
  case INST_MOV:
    return "\tmov";
  case INST_LABEL:
    return "Fn";
  case INST_EOF:
    return "\teof";
  default:
    __builtin_unreachable();
  }
}

static Inst_Context INST_CONTEXTS[INST_EOF + 1] = {
    [INST_PUSH] =
        {
            .has_operand = 1,
            .operand_type = WORD_ANY,
        },
    [INST_POP] =
        {
            .has_operand = 0,
        },
    [INST_PLUS] =
        {
            .has_operand = 0,
        },
    [INST_PLUSF] =
        {
            .has_operand = 0,
        },
    [INST_MINUS] =
        {
            .has_operand = 0,
        },
    [INST_MULT] =
        {
            .has_operand = 0,
        },
    [INST_DIV] =
        {
            .has_operand = 0,
        },
    [INST_EQ] =
        {
            .has_operand = 0,
        },
    [INST_NE] =
        {
            .has_operand = 0,
        },
    [INST_GT] =
        {
            .has_operand = 0,
        },
    [INST_LT] =
        {
            .has_operand = 0,
        },
    [INST_PRINT] =
        {
            .has_operand = 0,
        },
    [INST_PRINTS] =
        {
            .has_operand = 0,
        },
    [INST_NEG] =
        {
            .has_operand = 0,
        },
    [INST_DEFG] =
        {
            .has_operand = 1,
            .operand_type = WORD_SV,
        },
    [INST_DEFL] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_VARG] =
        {
            .has_operand = 1,
            .operand_type = WORD_SV,
        },
    [INST_VARL] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_JMPA] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_JMPT] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_JMPNT] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_RET] =
        {
            .has_operand = 0,
        },
    [INST_LDR] =
        {
            .has_operand = 1,
            .operand_type = WORD_REG,
        },
    [INST_STR] =
        {
            .has_operand = 1,
            .operand_type = WORD_REG,
        },
    [INST_MOV] =
        {
            .has_operand = 1,
            .operand_type = WORD_REG,
        },
    [INST_LABEL] =
        {
            .has_operand = 1,
            .operand_type = WORD_SV,
        },
    [INST_EOF] =
        {
            .has_operand = 0,
        },
};

void vm_stack_dump(void) {
  printf("Stack: \n");
  for (size_t i = 0; i < (size_t)vm.stack_count; i++) {
    printf("\t%zu: %lld\n", i, vm.stack[i].as_u64);
  }
  printf("-----\n\n");
}

__attribute__((unused)) static char *vm_word_reg_translate(uint64_t reg_no) {
  switch (reg_no) {
  case 11:
    return (char *)"fp";
  case 12:
    return (char *)"sp";
  case 13:
    return (char *)"ra";
  case 14:
    return (char *)"rax";
  case 15:
    return (char *)"cpsr";
  default:
    __builtin_unreachable();
  }
}

static void vm_inst_dump(const Inst *inst) {
  printf("%s ", vm_inst_t_to_str(inst->type));
  if (INST_CONTEXTS[inst->type].has_operand) {
    switch (INST_CONTEXTS[inst->type].operand_type) {
    case WORD_ANY:
      printf("%lld", inst->operand.as_u64);
      break;
    case WORD_REG:
      printf("%s", vm_word_reg_translate(inst->operand.as_u64));
      break;
    case WORD_U64:
      printf("%lld", inst->operand.as_u64);
      break;
    case WORD_I64:
      printf("%lld", inst->operand.as_i64);
      break;
    case WORD_F64:
      printf("%f", inst->operand.as_f64);
      break;
    case WORD_SV:
      printf("%.*s", inst->operand.as_sv.len, inst->operand.as_sv.str);
      break;
    case WORD_PTR:
      printf("%p", inst->operand.as_ptr);
      break;
    }
  }
  printf("\n");
}

void vm_program_dump(void) {
  printf("Program: \n");
  for (size_t i = 0; i < (size_t)vm.program_size; i++) {
    Inst *inst = &vm.program[i];
#ifdef DEBUG
    printf("%zu: ", i);
#endif
    vm_inst_dump(inst);
  }
  printf("-----\n\n");
}

inline static Word vm_env_resolve(const Sv label) {
  return *hash_table_get(&vm.env, label);
}

#ifndef DEBUG
#define SP_INCREMENT vm.reg[REG_SP].as_u64++;
#define SP_DECREMENT vm.reg[REG_SP].as_u64--;
#else
#define SP_INCREMENT                                                           \
  do {                                                                         \
    vm.reg[REG_SP].as_u64++;                                                   \
    printf("IP    : %lld\n", vm.ip);                                           \
    printf("RA    : %lld\n", vm.reg[REG_RA].as_u64);                           \
    printf("CPSR  : %lld\n", vm.reg[REG_CPSR].as_u64);                         \
    printf("FP    : %lld\n", vm.reg[REG_FP].as_u64);                           \
    printf("SP++  : %lld\n", vm.reg[REG_SP].as_u64);                           \
  } while (0)
#define SP_DECREMENT                                                           \
  do {                                                                         \
    vm.reg[REG_SP].as_u64--;                                                   \
    printf("IP    : %lld\n", vm.ip);                                           \
    printf("RA    : %lld\n", vm.reg[REG_RA].as_u64);                           \
    printf("CPSR  : %lld\n", vm.reg[REG_CPSR].as_u64);                         \
    printf("FP    : %lld\n", vm.reg[REG_FP].as_u64);                           \
    printf("SP--  : %lld\n", vm.reg[REG_SP].as_u64);                           \
  } while (0)
#endif

void vm_execute(void) {
  int n = 1;
#ifdef DEBUG
  n = 100;
#endif

  while (n) {
    const Inst inst = vm.program[vm.reg[REG_IP].as_u64++];
    Word word_one;
    Word word_two;
    uint64_t reg_no;
    uint64_t jmp_offset;
    uint64_t fp;

#ifdef DEBUG
    n--;
    vm_stack_dump();
    vm_inst_dump(&inst);
#endif

    switch (inst.type) {
    case INST_PUSH:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      vm.stack[vm.stack_count++] = inst.operand;
      SP_INCREMENT;
      continue;

    case INST_POP:
      assert(vm.stack_count > 0 && "Stack underflow");

      vm.stack_count--;
      SP_DECREMENT;
      continue;

    case INST_PLUS:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 += word_one.as_u64;
      SP_DECREMENT;
      continue;

    case INST_PLUSF:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_f64 += word_one.as_f64;
      SP_DECREMENT;
      continue;

    case INST_MINUS:
      assert(vm.stack_count > 1);

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 -= word_one.as_u64;
      SP_DECREMENT;
      continue;

    case INST_MULT:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 *= word_one.as_u64;
      SP_DECREMENT;
      continue;

    case INST_DIV:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      assert(word_one.as_u64 != 0);
      vm.stack[--vm.stack_count - 1].as_u64 /= word_one.as_u64;
      SP_DECREMENT;
      continue;

    case INST_EQ:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_two = vm.stack[vm.stack_count - 2];
      word_one = vm.stack[vm.stack_count - 1];

      uint64_t eq = 0;
      if (word_one.as_u64 == word_two.as_u64)
        eq = 1;

      vm.stack[vm.stack_count - 2] = (Word){.as_u64 = eq};

      vm.stack_count -= 1;
      SP_DECREMENT;

      continue;

    case INST_NE:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_two = vm.stack[vm.stack_count - 2];
      word_one = vm.stack[vm.stack_count - 1];

      uint64_t ne = 0;
      if (word_one.as_u64 != word_two.as_u64)
        ne = 1;

      vm.stack[vm.stack_count - 2] = (Word){.as_u64 = ne};

      vm.stack_count -= 1;
      SP_DECREMENT;

      continue;

    case INST_GT:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_two = vm.stack[vm.stack_count - 2];
      word_one = vm.stack[vm.stack_count - 1];

      uint64_t gt = 0;
      if (word_two.as_u64 > word_one.as_u64)
        gt = 1;

      vm.stack[vm.stack_count - 2] = (Word){.as_u64 = gt};

      vm.stack_count -= 1;
      SP_DECREMENT;

      continue;

    case INST_LT:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_two = vm.stack[vm.stack_count - 2];
      word_one = vm.stack[vm.stack_count - 1];

      uint64_t lt = 0;
      if (word_two.as_u64 < word_one.as_u64)
        lt = 1;

      vm.stack[vm.stack_count - 2] = (Word){.as_u64 = lt};

      vm.stack_count -= 1;
      SP_DECREMENT;

      continue;

    case INST_PRINT:
      assert(vm.stack_count > 0 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      printf("%lld\n", word_one.as_u64);
      continue;

    case INST_PRINTS:
      assert(vm.stack_count > 0 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      printf("%.*s\n", word_one.as_sv.len, word_one.as_sv.str);
      continue;

    case INST_NEG:
      assert(vm.stack_count > 0 && "Stack underflow");

      vm.stack[vm.stack_count - 1].as_u64 =
          -vm.stack[vm.stack_count - 1].as_u64;
      continue;

    case INST_DEFG:
      assert(vm.stack_count > 0 && "Stack underflow");

      Sv assign_name = inst.operand.as_sv;

      Word value = vm.stack[vm.stack_count - 1];

      hash_table_insert(&vm.env, assign_name, value);
      continue;

    case INST_DEFL:
      assert(vm.stack_count > 0 && "Stack underflow");

      uint64_t def_offset = inst.operand.as_u64;
      assert(def_offset < vm.stack_count && "Stack illegal access");

      Word word = vm.stack[def_offset];
      vm.stack[vm.stack_count++] = word;
      SP_INCREMENT;

      continue;

    case INST_VARG:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      Sv var_name = inst.operand.as_sv;
      assert(hash_table_keys_contains(&vm.env, var_name) == 1 &&
             "Undefined var");

      vm.stack[vm.stack_count++] = vm_env_resolve(var_name);
      SP_INCREMENT;
      continue;

    case INST_VARL:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      uint64_t var_offset = inst.operand.as_u64;
      fp = vm.reg[REG_FP].as_u64;
      assert(var_offset < vm.stack_count && "Stack illegal access");

      assert(var_offset < vm.stack_count && "Program illegal access");
      vm.stack[vm.stack_count++] = vm.stack[fp + var_offset];
      SP_INCREMENT;
      continue;

    case INST_JMPA:;
      jmp_offset = inst.operand.as_u64;
      assert(jmp_offset < vm.program_size && "Program illegal access");

      vm.reg[REG_IP].as_u64 = jmp_offset;
      continue;

    case INST_JMPT:;
      assert(vm.stack_count > 0 && "Stack underflow");

      eq = vm.stack[vm.stack_count-- - 1].as_u64;
      SP_DECREMENT;

      jmp_offset = inst.operand.as_u64;
      assert(jmp_offset < vm.program_size && "Program illegal access");

      if (eq)
        vm.reg[REG_IP].as_u64 = jmp_offset;

      continue;

    case INST_JMPNT:;
      assert(vm.stack_count > 0 && "Stack underflow");

      eq = vm.stack[vm.stack_count-- - 1].as_u64;
      SP_DECREMENT;

      jmp_offset = inst.operand.as_u64;
      assert(jmp_offset < vm.program_size && "Program illegal access");

      if (!eq)
        vm.reg[REG_IP].as_u64 = jmp_offset;

      continue;

    case INST_RET:
      vm.reg[REG_IP].as_u64 = vm.reg[REG_RA].as_u64;

      continue;

    case INST_STR:
      assert(vm.stack_count > 0 && "Stack underflow");

      reg_no = inst.operand.as_u64;

      value = vm.stack[vm.stack_count-- - 1];
      SP_DECREMENT;
      vm.reg[reg_no] = value;

      continue;

    case INST_LDR:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      reg_no = inst.operand.as_u64;

      vm.stack[vm.stack_count++] = vm.reg[reg_no];
      SP_INCREMENT;

      continue;

    case INST_LABEL:
      continue;

    case INST_MOV:
      assert(vm.stack_count > 0 && "Stack underflow");

      uint64_t reg_dst = inst.operand.as_u64;
      uint64_t reg_src = vm.stack[vm.stack_count-- - 1].as_u64;
      SP_DECREMENT;

      vm.reg[reg_dst] = vm.reg[reg_src];

      continue;

    case INST_EOF:
      return;

    default:
      __builtin_unreachable();
    }
  }
}
