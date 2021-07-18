#include "sam.h"


#ifndef _SAM_H_
#define _SAM_H_

//void dma_enable();
void dma_enable();
void dma_send(uint8_t ch, uint32_t src_addr, uint32_t dst_addr, uint16_t count);
void dma_start_transaction(uint8_t ch);
void dma_ch_init(uint8_t ch);
void dma_add_descriptior(uint8_t ch);
#endif
