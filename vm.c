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
  case INST_ASSIGN:
    return "assign";
  case INST_EOF:
    return "eof";
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
    printf("%s %lld\n", vm_inst_t_to_str(inst->type), inst->operand.as_u64);
  }
  printf("-----\n\n");
}

inline static void vm_word_print(Word *word) {
  printf("Word: \n");
  printf("\tas_u64: %lld\n", word->as_u64);
  printf("\tas_i64: %lld\n", word->as_i64);
  printf("\tas_f64: %llu\n", word->as_u64);
  // TODO: print %s
  printf("\tas_str: %p\n", word->as_str);
  printf("\tas_ptr: %p\n", word->as_ptr);
}

// Temporary implementation of vm interpreter
//
// Will further reform into assembly translator

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

    case INST_NEGATE:
      assert(vm.stack_count > 0 && "Stack underflow");

      vm.stack[vm.stack_count - 1].as_u64 =
          -vm.stack[vm.stack_count - 1].as_u64;
      continue;

    case INST_ASSIGN:
      assert(vm.stack_count > 0);

      char *label = inst.operand.as_str;
      assert(hash_table_keys_contains(&vm.env, label) && "Redefinition of var");

      word_one = vm.stack[vm.stack_count - 1];
      void *value = (void *)(&word_one);

      hash_table_insert(&vm.env, label, value);
      continue;

    case INST_EOF:
      return;

    default:
      __builtin_unreachable();
    }
  }
}
