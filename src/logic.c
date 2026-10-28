#include "logic.h"
#include <string.h>
const char *const names[TYPES] = {"EMPTY","SWITCH","LIGHT","AND","OR","NOT","NAND","NOR","XOR","XNOR","BUF"};
uint8_t pins(uint8_t t) { return t==EMPTY || t==SWITCH ? 0 : t==LIGHT || t==NOT || t==BUFFER ? 1 : 2; }
void board_clear(Board *b) {
    memset(b,0,sizeof(*b));
    b->cols=VIEW_COLS;b->rows=VIEW_ROWS;
    for(uint8_t i=0;i<CELLS;i++) b->node[i].in[0]=b->node[i].in[1]=NONE;
}
uint8_t board_count(const Board *b) { return b->cols*b->rows; }
bool board_resize(Board *b,uint8_t cols,uint8_t rows) {
    if(cols<VIEW_COLS || cols>COLS || rows<VIEW_ROWS || rows>ROWS) return false;
    /* Reject a shrink that would discard a part. Wires are remapped by position. */
    for(uint8_t i=0;i<board_count(b);i++)
        if(b->node[i].type!=EMPTY && (i%b->cols>=cols || i/b->cols>=rows)) return false;
    Board next;board_clear(&next);next.cols=cols;next.rows=rows;
    for(uint8_t i=0;i<board_count(b);i++) {
        if(i%b->cols>=cols || i/b->cols>=rows) continue;
        Node n=b->node[i];
        for(uint8_t p=0;p<2;p++) if(n.in[p]!=NONE)
            n.in[p]=(n.in[p]/b->cols)*cols+n.in[p]%b->cols;
        next.node[(i/b->cols)*cols+i%b->cols]=n;
    }
    *b=next;return true;
}
void board_delete(Board *b,uint8_t c) {
    if(c>=board_count(b)) return;
    b->node[c]=(Node){EMPTY,0,{NONE,NONE}};
    for(uint8_t i=0;i<CELLS;i++) for(uint8_t p=0;p<2;p++)
        if(b->node[i].in[p]==c) b->node[i].in[p]=NONE;
}
void board_place(Board *b,uint8_t c,uint8_t t) {
    if(c>=board_count(b) || t>=TYPES) return;
    if(t==EMPTY) { board_delete(b,c);return; }
    b->node[c].type=t; b->node[c].on=0;
    for(uint8_t p=pins(t);p<2;p++) b->node[c].in[p]=NONE;
}
static bool visit(const Board *b,uint8_t c,uint8_t mark[CELLS]) {
    if(mark[c]==1) return false;
    if(mark[c]==2) return true;
    mark[c]=1;
    for(uint8_t p=0;p<pins(b->node[c].type);p++) {
        uint8_t s=b->node[c].in[p];
        if(s!=NONE && !visit(b,s,mark)) return false;
    }
    mark[c]=2; return true;
}
bool board_valid(const Board *b) {
    if(b->cols<VIEW_COLS || b->cols>COLS || b->rows<VIEW_ROWS || b->rows>ROWS) return false;
    uint8_t mark[CELLS]={0};
    for(uint8_t c=0;c<CELLS;c++) {
        const Node *n=&b->node[c];
        if(n->type>=TYPES || n->on>1) return false;
        if(c>=board_count(b) && (n->type!=EMPTY || n->on)) return false;
        for(uint8_t p=0;p<2;p++) {
            uint8_t s=n->in[p];
            if(p>=pins(n->type) && s!=NONE) return false;
            if(s!=NONE && (s>=board_count(b) || b->node[s].type==EMPTY)) return false;
        }
    }
    for(uint8_t c=0;c<CELLS;c++) if(!visit(b,c,mark)) return false;
    return true;
}
bool board_connect(Board *b,uint8_t s,uint8_t t,uint8_t p) {
    if(s>=board_count(b) || t>=board_count(b) || p>=pins(b->node[t].type) || b->node[s].type==EMPTY) return false;
    uint8_t old=b->node[t].in[p]; b->node[t].in[p]=s;
    if(!board_valid(b)) { b->node[t].in[p]=old;return false; }
    return true;
}
uint8_t gate_eval(uint8_t t,uint8_t a,uint8_t b) {
    uint8_t v=UNKNOWN;
    if(t==LIGHT || t==BUFFER) return a;
    if(t==NOT) return a==UNKNOWN ? UNKNOWN : !a;
    if(t==AND || t==NAND) v=a==LOW || b==LOW ? LOW : a==HIGH && b==HIGH ? HIGH : UNKNOWN;
    if(t==OR || t==NOR) v=a==HIGH || b==HIGH ? HIGH : a==LOW && b==LOW ? LOW : UNKNOWN;
    if(t==XOR || t==XNOR) v=a==UNKNOWN || b==UNKNOWN ? UNKNOWN : a!=b;
    if(t==NAND || t==NOR || t==XNOR) return v==UNKNOWN ? UNKNOWN : !v;
    return v;
}
static uint8_t eval_node(const Board *b,uint8_t c,uint8_t out[CELLS],uint8_t mark[CELLS]) {
    if(c==NONE || c>=CELLS || mark[c]==1) return UNKNOWN;
    if(mark[c]==2) return out[c];
    const Node *n=&b->node[c]; mark[c]=1;
    if(n->type==SWITCH) out[c]=n->on;
    else if(n->type==EMPTY) out[c]=UNKNOWN;
    else {
        uint8_t a=eval_node(b,n->in[0],out,mark);
        uint8_t d=pins(n->type)==2 ? eval_node(b,n->in[1],out,mark) : UNKNOWN;
        out[c]=gate_eval(n->type,a,d);
    }
    mark[c]=2;return out[c];
}
void board_eval(const Board *b,uint8_t out[CELLS]) {
    uint8_t mark[CELLS]={0};
    for(uint8_t c=0;c<CELLS;c++) out[c]=UNKNOWN;
    for(uint8_t c=0;c<CELLS;c++) eval_node(b,c,out,mark);
}
static void add(Board *b,uint8_t c,uint8_t t,uint8_t a,uint8_t d) {
    board_place(b,c,t); b->node[c].in[0]=a;b->node[c].in[1]=d;
}
void board_demo(Board *b,uint8_t d) {
    board_clear(b);
    add(b,0,SWITCH,NONE,NONE);add(b,12,SWITCH,NONE,NONE);
    if(d==0) {
        add(b,8,AND,0,12);add(b,10,LIGHT,8,NONE);
    } else if(d==1) {
        add(b,8,XOR,0,12);add(b,14,AND,0,12);
        add(b,10,LIGHT,8,NONE);add(b,16,LIGHT,14,NONE);
    } else {
        add(b,18,SWITCH,NONE,NONE);
        add(b,2,XOR,0,12);add(b,8,AND,0,12);
        add(b,4,XOR,2,18);add(b,14,AND,2,18);add(b,16,OR,8,14);
        add(b,5,LIGHT,4,NONE);add(b,17,LIGHT,16,NONE);
    }
}
