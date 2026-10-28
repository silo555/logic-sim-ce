#ifndef SAVE_H
#define SAVE_H
#include "logic.h"
#include <stddef.h>
#define LEGACY_SAVE_SIZE 108
#define SAVE_SIZE (14+4*CELLS)
void save_encode(uint8_t out[SAVE_SIZE],const Board *b,uint32_t generation);
bool save_decode(const uint8_t *in,size_t size,Board *b,uint32_t *generation);
#endif
