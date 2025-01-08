#include <assert.h>
#include <stdio.h>

#include "analyzer.h"
#include "table.h"
#include "vm.h"

Analyzer analyzer = {0};

void analyzer_ir_load(Inst *insts) { analyzer.ir = insts; }

__attribute__((unused)) static void analyzer_basic_blocks_dump(void) {
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

#define NEXT_INST &analyzer.ir[insts_pos++]
#define CUR_INST &analyzer.ir[insts_pos - 1]
#define PREV_INST &analyzer.ir[insts_pos - 2]

static void analyzer_basic_blocks_dismember(void) {
  size_t insts_pos = 0;

  while (1) {
    Basic_block block = {
        .block_no = analyzer.blocks_count,
        .start = insts_pos,
        .len = 0,
    };

    uint8_t block_len = 1;
    while (1) {
      Inst *next_inst = NEXT_INST;

      if (next_inst->type == INST_EOF) {
        block.len = block_len;
        BLOCKS_PUSH(block);
        return;
      }

      if (BLOCK_IS_END(next_inst->type)) {
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

// push 3
// defg a
// pop
// push 4
// defg a
// pop
// varg a
// print
// pop
// push 0
// str rax
// ret

static void analyzer_dse_local(Basic_block *block) {
  Hash_Table dse = hash_table_new();
  size_t insts_pos = block->start;

  while (insts_pos < block->start + block->len) {
    Inst *next_inst = NEXT_INST;

    if (next_inst->type == INST_EOF) {
      return;
    }

    if (next_inst->type == INST_DEFG || next_inst->type == INST_DEFL) {
      Sv name = next_inst->operand.as_sv;
      Word *maybe_used = hash_table_get(&dse, name);
      if (maybe_used && maybe_used->as_u64 == next_inst->type) {
        printf("Remove useless code: %zu ~ %zu\n", insts_pos - 1,
               insts_pos + 1);
      };

      hash_table_insert(&dse, name, (Word){.as_u64 = next_inst->type});
    }

    if (next_inst->type == INST_VARG || next_inst->type == INST_VARL) {
      Sv name = next_inst->operand.as_sv;
      hash_table_delete(&dse, name);
    }
  }
}

void analyzer_analyze_dse(void) {
  analyzer_basic_blocks_dismember();
  analyzer_basic_blocks_dump();

  for (size_t i = 0; i < analyzer.blocks_count; i++) {
    analyzer_dse_local(&analyzer.blocks[i]);
  }
}

#undef NEXT_INST
#undef CUR_INST
#undef PREV_INST
