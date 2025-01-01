#include "vm.h"
#include <assert.h>

static Vm vm = {0};

inline static void vm_stack_push(Inst inst) {
  assert(vm.stack_count + 1 < VM_STACK_CAP);

  vm.stack[vm.stack_count++] = inst;
}

inline static Inst vm_stack_pop(Inst inst) {
  assert(vm.stack_count > 0);

  vm.stack_count--;
  return vm.stack[vm.stack_count];
}

int main(void) {}
