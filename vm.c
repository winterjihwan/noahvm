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
  case INST_EOF:
    return "eof";
  default:
    __builtin_unreachable();
  }
}

inline static int vm_inst_has_operand(Inst *inst) {
  switch (inst->type) {
  case INST_PUSH:
    return 1;
  case INST_POP:
    return 0;
  case INST_PLUS:
    return 0;
  case INST_MINUS:
    return 0;
  case INST_MULT:
    return 0;
  case INST_DIV:
    return 0;
  case INST_PRINT:
    return 0;
  case INST_NEGATE:
    return 0;
  case INST_DEF_GLOBAL:
    return 1;
  case INST_DEF_LOCAL:
    return 1;
  case INST_VAR_GLOBAL:
    return 1;
  case INST_VAR_LOCAL:
    return 1;
  case INST_EOF:
    return 0;
  default:
    __builtin_unreachable();
  }
}

void vm_stack_dump(void) {
  printf("Stack: \n");
  for (size_t i = 0; i < (size_t)vm.stack_count; i++) {
    printf("\t%zu: %lld\n", i, vm.stack[i].as_u64);
  }
  printf("-----\n\n");
}

void vm_program_dump(void) {
  printf("Program: \n");
  for (size_t i = 0; i < (size_t)vm.program_size; i++) {
    Inst *inst = &vm.program[i];
    printf("%s ", vm_inst_t_to_str(inst->type));
    if (vm_inst_has_operand(inst)) {
      printf("%lld", inst->operand.as_u64);
    }
    printf("\n");
  }
  printf("-----\n\n");
}

Word vm_env_resolve(const Sv label) { return *hash_table_get(&vm.env, label); }

void vm_execute(void) {
  while (1) {
    const Inst inst = vm.program[vm.ip++];
    Word word_one;

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

      uint64_t def_offset = (int)inst.operand.as_u64;
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

      uint64_t var_offset = (int)inst.operand.as_u64;
      assert(var_offset < vm.stack_count && "Stack illegal access");

      vm.stack[vm.stack_count++] = vm.stack[var_offset];
      continue;

    case INST_EOF:
      return;

    default:
      __builtin_unreachable();
    }
  }
}
