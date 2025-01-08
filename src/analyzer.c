#include "analyzer.h"
#include "vm.h"

Analyzer analyzer = {0};
static uint64_t insts_pos = 0;

#define BLOCKS_PUSH(block) analyzer.blocks[analyzer.blocks_count++] = block;
#define BLOCK_IS_END(type)                                                     \
  (type == INST_JMPA || type == INST_JMPT || type == INST_JMPNT ||             \
   type == INST_RET)

void analyzer_analyze_basic_blocks(Inst *insts) {
  while (1) {
    Basic_block block = {
        .block_no = analyzer.blocks_count,
        .start = insts_pos,
        .len = 0,
    };

    uint8_t block_len = 0;
    while (1) {
      Inst_t inst_t = insts[insts_pos++].type;

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
    }
  }
}
