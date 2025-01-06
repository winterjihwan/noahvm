#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "table.h"
#include "vm.h"

Vm vm = {0};

void vm_init(void) {
  vm.ip = 0;
  vm.stack_count = 0;
  vm.program_size = 0;
  vm.env = hash_table_new();
}

void vm_destruct(void) { hash_table_destruct(&vm.env); }

void vm_program_load_from_memory(Inst *insts, size_t insts_count) {
  assert(insts_count < PROGRAM_STACK_CAP);

  for (size_t i = 0; i < insts_count; i++) {
    vm.program[vm.ip++] = insts[i];
  }

  vm.program_size = vm.ip;
  vm.ip = 0;
}

char *vm_inst_t_to_str(Inst_t type) {
  switch (type) {
  case INST_PUSH:
    return "push";
  case INST_POP:
    return "pop";
  case INST_PLUS:
    return "plus";
  case INST_PLUSF:
    return "plusf";
  case INST_MINUS:
    return "minus";
  case INST_MULT:
    return "mult";
  case INST_DIV:
    return "div";
  case INST_PRINT:
    return "print";
  case INST_NEGATE:
    return "negate";
  case INST_DEF_GLOBAL:
    return "def_global";
  case INST_DEF_LOCAL:
    return "def_local";
  case INST_VAR_GLOBAL:
    return "var_global";
  case INST_VAR_LOCAL:
    return "var_local";
  case INST_JMP_ABS:
    return "jmp_abs";
  case INST_RET:
    return "ret";
  case INST_LDR:
    return "ldr";
  case INST_STR:
    return "str";
  case INST_EOF:
    return "eof";
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
    [INST_PRINT] =
        {
            .has_operand = 0,
        },
    [INST_NEGATE] =
        {
            .has_operand = 0,
        },
    [INST_DEF_GLOBAL] =
        {
            .has_operand = 1,
            .operand_type = WORD_SV,
        },
    [INST_DEF_LOCAL] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_VAR_GLOBAL] =
        {
            .has_operand = 1,
            .operand_type = WORD_SV,
        },
    [INST_VAR_LOCAL] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
        },
    [INST_JMP_ABS] =
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
            .operand_type = WORD_U64,
        },
    [INST_STR] =
        {
            .has_operand = 1,
            .operand_type = WORD_U64,
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

static void vm_inst_dump(const Inst *inst) {
  printf("%s ", vm_inst_t_to_str(inst->type));
  if (INST_CONTEXTS[inst->type].has_operand) {
    switch (INST_CONTEXTS[inst->type].operand_type) {
    case WORD_ANY:
      printf("%lld", inst->operand.as_u64);
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
    vm_inst_dump(inst);
  }
  printf("-----\n\n");
}

Word vm_env_resolve(const Sv label) { return *hash_table_get(&vm.env, label); }

void vm_execute(void) {
  int end = 30;
  while (end-- != 0) {
    const Inst inst = vm.program[vm.ip++];
    Word word_one;
    uint64_t reg_no;

    /*vm_stack_dump();*/
    /*vm_inst_dump(&inst);*/

    switch (inst.type) {
    case INST_PUSH:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      vm.stack[vm.stack_count++] = inst.operand;
      continue;

    case INST_POP:
      assert(vm.stack_count > 0 && "Stack underflow");

      vm.stack_count--;
      continue;

    case INST_PLUS:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 += word_one.as_u64;
      continue;

    case INST_PLUSF:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_f64 += word_one.as_f64;
      continue;

    case INST_MINUS:
      assert(vm.stack_count > 1);

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 -= word_one.as_u64;
      continue;

    case INST_MULT:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1].as_u64 *= word_one.as_u64;
      continue;

    case INST_DIV:
      assert(vm.stack_count > 1 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      assert(word_one.as_u64 != 0);
      vm.stack[--vm.stack_count - 1].as_u64 /= word_one.as_u64;
      continue;

    case INST_PRINT:
      assert(vm.stack_count > 0 && "Stack underflow");

      word_one = vm.stack[vm.stack_count - 1];
      printf("%lld\n", word_one.as_u64);
      continue;

    case INST_NEGATE:
      assert(vm.stack_count > 0 && "Stack underflow");

      vm.stack[vm.stack_count - 1].as_u64 =
          -vm.stack[vm.stack_count - 1].as_u64;
      continue;

    case INST_DEF_GLOBAL:
      assert(vm.stack_count > 0 && "Stack underflow");

      Sv assign_name = inst.operand.as_sv;

      Word value = vm.stack[vm.stack_count - 1];

      hash_table_insert(&vm.env, assign_name, value);
      continue;

    case INST_DEF_LOCAL:
      assert(vm.stack_count > 0 && "Stack underflow");

      uint64_t def_offset = inst.operand.as_u64;
      assert(def_offset < vm.stack_count && "Stack illegal access");

      Word word = vm.stack[def_offset];
      vm.stack[vm.stack_count++] = word;

      continue;

    case INST_VAR_GLOBAL:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      Sv var_name = inst.operand.as_sv;
      assert(hash_table_keys_contains(&vm.env, var_name) == 1 &&
             "Undefined var");

      vm.stack[vm.stack_count++] = vm_env_resolve(var_name);
      continue;

    case INST_VAR_LOCAL:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      uint64_t var_offset = inst.operand.as_u64;
      assert(var_offset < vm.stack_count && "Stack illegal access");

      vm.stack[vm.stack_count++] = vm.stack[var_offset];
      continue;

    case INST_JMP_ABS:;
      uint64_t jmp_offset = inst.operand.as_u64;
      assert(jmp_offset < vm.program_size && "Program illegal access");

      vm.ip = jmp_offset;
      continue;

    case INST_RET:
      vm.ip = vm.reg[REG_RA].as_u64;

      continue;

    case INST_STR:
      assert(vm.stack_count > 0 && "Stack underflow");

      reg_no = inst.operand.as_u64;

      value = vm.stack[vm.stack_count-- - 1];
      vm.reg[reg_no] = value;

      continue;

    case INST_LDR:
      assert(vm.stack_count + 1 < VM_STACK_CAP && "Stack overflow");

      reg_no = inst.operand.as_u64;

      vm.stack[vm.stack_count++] = vm.reg[reg_no];

      continue;

    case INST_EOF:
      return;

    default:
      __builtin_unreachable();
    }
  }
}
