#include "analyzer.h"
#include "vm.h"
#include <stdio.h>

Analyzer analyzer = {0};
static uint64_t insts_pos = 0;

void analyzer_ir_load(Inst *insts) { analyzer.ir = insts; }

void analyzer_basic_blocks_dump(void) {
  printf("Basic blocks: \n");
  for (size_t i = 0; i < analyzer.blocks_count; i++) {
    Basic_block *block = &analyzer.blocks[i];
    printf("B%d\n", block->block_no);
    for (size_t j = 0; j < block->len; j++) {
      printf("\t");
      vm_inst_dump(&analyzer.ir[block->start + j]);
    }
  }
}

#define BLOCKS_PUSH(block) analyzer.blocks[analyzer.blocks_count++] = block;
#define BLOCK_IS_END(type)                                                     \
  (type == INST_JMPA || type == INST_JMPT || type == INST_JMPNT ||             \
   type == INST_RET)

void analyzer_basic_blocks_dismember(void) {
  while (1) {
    Basic_block block = {
        .block_no = analyzer.blocks_count,
        .start = insts_pos,
        .len = 0,
    };

    uint8_t block_len = 1;
    while (1) {
      Inst_t inst_t = analyzer.ir[insts_pos++].type;

      if (inst_t == INST_EOF) {
        block.len = block_len;
        BLOCKS_PUSH(block);
        return;
      }

      if (BLOCK_IS_END(inst_t)) {
        block.len = block_len;
        BLOCKS_PUSH(block);
        break;
      }

      block_len++;
    }
  }
}

#undef BLOCKS_PUSH
#undef BLOCKS_IS_END

void analyzer_dead_store_elimination(void) {}
