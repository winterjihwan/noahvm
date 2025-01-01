#include "vm.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static Vm vm = {0};

void vm_init(void) {
  vm.ip = 0;
  vm.stack_count = 0;
}

void vm_program_load_from_memory(Inst *insts, size_t insts_count) {
  assert(insts_count < PROGRAM_STACK_CAP);

  for (size_t i = 0; i < insts_count; i++) {
    vm.program[vm.ip++] = insts[i];
  }

  vm.ip = 0;
}

inline static char *vm_inst_t_to_str(Inst_t type) {
  switch (type) {
  case INST_PUSH:
    return "push";
  case INST_POP:
    return "pop";
  case INST_PLUS:
    return "plus";
  case INST_EOF:
    return "eof";
  default:
    __builtin_unreachable();
  }
}

static void vm_dump(void) {
  printf("Stack: \n");
  for (size_t i = 0; i < (size_t)vm.stack_count; i++) {
    printf("\t%zu: %lld\n", i, vm.stack[i]);
  }
  printf("-----\n");
}

// Temporary implementation of vm interpreter
//
// Will further reform into assembly translator
void vm_execute(void) {
  while (1) {
    const Inst inst = vm.program[vm.ip++];

    switch (inst.type) {
    case INST_PUSH:
      assert(vm.stack_count + 1 < VM_STACK_CAP);

      vm.stack[vm.stack_count++] = inst.operand;
      continue;

    case INST_POP:
      assert(vm.stack_count > 0);

      vm.stack_count--;

    case INST_PLUS:
      assert(vm.stack_count > 1);

      Word word_one = vm.stack[vm.stack_count - 1];
      vm.stack[--vm.stack_count - 1] += word_one;

    case INST_EOF:
      return;

    default:
      __builtin_unreachable();
    }
  }
}

int main(void) {
  vm_init();

  Inst insts[5] = {(Inst){.type = INST_PUSH, .operand = 2},
                   (Inst){.type = INST_PUSH, .operand = 3},
                   (Inst){.type = INST_PLUS}, (Inst){.type = INST_EOF}};

  vm_program_load_from_memory(insts, 5);
  vm_dump();
  vm_execute();
  vm_dump();
}
