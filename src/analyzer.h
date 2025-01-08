#ifndef ANALYZER_H
#define ANALYZER_H

#include "vm.h"

typedef struct {
  uint16_t block_no;

  uint8_t len;
  uint8_t start;
} Basic_block;

typedef struct {
  const Inst *ir;

  Basic_block blocks[INSTS_CAP];
  uint64_t blocks_count;
} Analyzer;

void analyzer_ir_load(const Inst *insts);
void analyzer_analyze_dse(void);

#endif
