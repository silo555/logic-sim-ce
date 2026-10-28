#include "save.h"
#include <string.h>
static uint16_t checksum(const uint8_t *p,size_t size) {
    uint16_t c=0xFFFF;
    for(unsigned i=0;i<size-2;i++) {
        c^=p[i];for(uint8_t bit=0;bit<8;bit++) c=(c>>1)^((c&1)?0xA001:0);
    }
    return c;
}
void save_encode(uint8_t out[SAVE_SIZE],const Board *b,uint32_t g) {
    memcpy(out,"LGCE02",6);
    for(uint8_t i=0;i<4;i++) out[6+i]=(uint8_t)(g>>(8*i));
    out[10]=b->cols;out[11]=b->rows;
    for(uint8_t i=0;i<CELLS;i++) {
        out[12+4*i]=b->node[i].type;out[13+4*i]=b->node[i].on;
        out[14+4*i]=b->node[i].in[0];out[15+4*i]=b->node[i].in[1];
    }
    uint16_t c=checksum(out,SAVE_SIZE);out[SAVE_SIZE-2]=c&255;out[SAVE_SIZE-1]=c>>8;
}
bool save_decode(const uint8_t *in,size_t size,Board *b,uint32_t *g) {
    bool legacy=size==LEGACY_SAVE_SIZE;
    if((size!=SAVE_SIZE && !legacy) || memcmp(in,legacy?"LGCE01":"LGCE02",6)) return false;
    if(checksum(in,size)!=(uint16_t)(in[size-2]|((uint16_t)in[size-1]<<8))) return false;
    Board candidate;board_clear(&candidate);uint32_t generation=0;
    for(uint8_t i=0;i<4;i++) generation|=(uint32_t)in[6+i]<<(8*i);
    if(!legacy) { candidate.cols=in[10];candidate.rows=in[11]; }
    uint8_t offset=legacy?10:12,count=legacy?24:CELLS;
    for(uint8_t i=0;i<count;i++) candidate.node[i]=(Node){in[offset+4*i],in[offset+4*i+1],{in[offset+4*i+2],in[offset+4*i+3]}};
    if(!board_valid(&candidate)) return false;
    *b=candidate;*g=generation;return true;
}
