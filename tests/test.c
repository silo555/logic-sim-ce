#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host_ce.h"
#include "app.h"
#include "save.h"
/* Actual calculator storage adapter is tested against an in-memory TI OS double. */
#define HOST_TEST
#define main calculator_main
#include "../src/main.c"
#undef main
uint16_t gfx_palette[256];
static uint8_t pixels[320*240],color;
static unsigned rectangles;
static bool fail_open,short_write,fail_archive,corrupt_read;
static struct { bool exists;uint8_t data[512];size_t size,pos; } files[2];
void gfx_Begin(void) {}
void gfx_End(void) {}
void gfx_SetDrawBuffer(void) {}
void gfx_BlitBuffer(void) {}
void gfx_SetColor(uint8_t c) { assert(c<12);color=c; }
void gfx_FillRectangle(int x,int y,int w,int h) {
    assert(w>0 && h>0 && x>=0 && y>=0 && x+w<=320 && y+h<=240);
    rectangles++;
    for(int row=y;row<y+h;row++) memset(pixels+row*320+x,color,(size_t)w);
}
void kb_Scan(void) {}
bool kb_AnyKey(void) { return false; }
bool kb_IsDown(int key) { (void)key;return false; }
ti_var_t ti_Open(const char *name,const char *mode) {
    int i=!strcmp(name,"LGSAVE0")?0:!strcmp(name,"LGSAVE1")?1:-1;
    assert(i>=0);if(fail_open) return 0;
    if(*mode=='w') { files[i].exists=true;files[i].size=0; }
    if(!files[i].exists) return 0;
    files[i].pos=0;return i+1;
}
void ti_Close(ti_var_t h) { assert(h==1 || h==2); }
uint16_t ti_GetSize(ti_var_t h) { return (uint16_t)files[h-1].size; }
size_t ti_Read(void *p,size_t s,size_t n,ti_var_t h) {
    size_t avail=(files[h-1].size-files[h-1].pos)/s;if(n>avail)n=avail;
    memcpy(p,files[h-1].data+files[h-1].pos,s*n);files[h-1].pos+=s*n;
    if(corrupt_read && n*s==SAVE_SIZE) ((uint8_t*)p)[20]^=1;
    return n;
}
size_t ti_Write(const void *p,size_t s,size_t n,ti_var_t h) {
    if(short_write)n/=2;assert(s*n<=512);
    memcpy(files[h-1].data,p,s*n);files[h-1].size=s*n;return n;
}
bool ti_SetArchiveStatus(bool archive,ti_var_t h) { assert(archive && h);return !fail_archive; }
void ti_SetGCBehavior(void (*before)(void),void (*after)(void)) { (void)before;(void)after; }
static void evaluate(Board *b,uint8_t v[CELLS]) { assert(board_valid(b));board_eval(b,v); }
static void test_gates(void) {
    /* Independent truth tables, rows A=0/1/?, columns B=0/1/?. */
    static const uint8_t expected[7][9]={
        {0,0,0,0,1,2,0,2,2},{0,1,2,1,1,1,2,1,2},
        {1,1,1,0,0,0,2,2,2},{1,1,1,1,0,2,1,2,2},
        {1,0,2,0,0,0,2,0,2},{0,1,2,1,0,2,2,2,2},
        {1,0,2,0,1,2,2,2,2}
    };
    for(uint8_t t=AND;t<=XNOR;t++) for(uint8_t a=0;a<3;a++) for(uint8_t b=0;b<3;b++)
        assert(gate_eval(t,a,b)==expected[t-AND][a*3+b]);
    for(uint8_t a=0;a<3;a++) { assert(gate_eval(BUFFER,a,2)==a);assert(gate_eval(LIGHT,a,2)==a); }
    puts("PASS all gate truth tables, including unknown signals");
}
static void test_circuits(void) {
    Board b,original;uint8_t v[CELLS];
    for(uint8_t d=0;d<3;d++) for(uint8_t bits=0;bits<(d==2?8:4);bits++) {
        board_demo(&b,d);b.node[0].on=bits&1;b.node[12].on=(bits>>1)&1;b.node[18].on=(bits>>2)&1;
        evaluate(&b,v);
        unsigned sum=(bits&1)+((bits>>1)&1)+(d==2?((bits>>2)&1):0);
        if(d==0) assert(v[10]==(sum==2));
        if(d==1) assert(v[10]==sum%2 && v[16]==sum/2);
        if(d==2) assert(v[5]==sum%2 && v[17]==sum/2);
    }
    board_clear(&b);assert(board_resize(&b,12,8));board_place(&b,95,SWITCH);
    for(uint8_t i=0;i<95;i++) { board_place(&b,i,LIGHT);assert(board_connect(&b,95,i,0)); }
    b.node[95].on=1;evaluate(&b,v);for(uint8_t i=0;i<CELLS;i++) assert(v[i]==1);
    board_delete(&b,95);evaluate(&b,v);for(uint8_t i=0;i<CELLS;i++) assert(v[i]==UNKNOWN);
    board_clear(&b);assert(board_resize(&b,12,8));for(uint8_t i=0;i<CELLS;i++) board_place(&b,i,NOT);
    for(uint8_t i=0;i<95;i++) assert(board_connect(&b,i+1,i,0));
    original=b;assert(!board_connect(&b,0,95,0));assert(!memcmp(&original,&b,sizeof(b)));
    assert(!board_connect(&b,1,1,0));assert(!board_connect(&b,255,0,0));assert(!board_connect(&b,0,255,0));
    board_place(&b,95,SWITCH);b.node[95].on=1;evaluate(&b,v);assert(v[0]==0);
    for(unsigned step=0;step<10000;step++) {
        uint8_t c=rand()%CELLS,s=rand()%CELLS,p=rand()%2;
        switch(rand()%4) {
            case 0: board_place(&b,c,rand()%TYPES);break;
            case 1: board_delete(&b,c);break;
            case 2: board_connect(&b,s,c,p);break;
            case 3: if(b.node[c].type==SWITCH)b.node[c].on^=1;break;
        }
        evaluate(&b,v);for(uint8_t i=0;i<CELLS;i++)assert(v[i]<=UNKNOWN);
    }
    puts("PASS demos, 95-way fanout, deep chains, cycle rejection, 10000 mixed edits");
}
static void press(App *a,uint8_t k) { app_key(a,k);assert(board_valid(&a->board));app_draw(a); }
static void test_space(void) {
    Board b,original;uint8_t values[CELLS];board_demo(&b,1);
    b.node[0].on=b.node[12].on=1;original=b;
    assert(board_resize(&b,9,6));assert(board_valid(&b));
    /* Original cells 0,12,8,14,10,16 become 0,18,11,20,13,22. */
    assert(b.node[11].type==XOR && b.node[11].in[0]==0 && b.node[11].in[1]==18);
    assert(b.node[20].type==AND && b.node[13].in[0]==11 && b.node[22].in[0]==20);
    evaluate(&b,values);assert(values[13]==LOW && values[22]==HIGH);
    assert(board_resize(&b,12,8));assert(board_valid(&b));
    assert(b.node[14].in[1]==24 && b.node[28].in[0]==26);
    evaluate(&b,values);assert(values[16]==LOW && values[28]==HIGH);
    board_place(&b,95,SWITCH);Board large=b;
    assert(!board_resize(&b,6,4));assert(!memcmp(&b,&large,sizeof(b)));
    board_delete(&b,95);assert(board_resize(&b,6,4));assert(!memcmp(&b,&original,sizeof(b)));
    assert(!board_resize(&b,13,8));assert(!board_resize(&b,6,3));
    App a;app_init(&a);board_clear(&a.board);
    board_place(&a.board,0,SWITCH);a.board.node[0].on=1;
    board_place(&a.board,5,LIGHT);assert(board_connect(&a.board,0,5,0));app_draw(&a);
    /* The intervening empty cell's center is clear, despite the long wire. */
    assert(pixels[64*320+133]==2);
    board_place(&a.board,2,AND);app_draw(&a);
    assert(a.board.node[5].in[0]==0 && a.values[5]==HIGH && a.board.node[2].type==AND);
    assert(pixels[78*320+133]==5); /* thin wire now uses the row gutter */
    a.screen=SCREEN_MENU;a.choice=7;press(&a,K_ENTER);assert(a.screen==SCREEN_SIZE);
    press(&a,K_DOWN);press(&a,K_DOWN);press(&a,K_ENTER);assert(a.board.cols==12 && a.board.rows==8);
    for(int i=0;i<11;i++) press(&a,K_RIGHT);
    for(int i=0;i<7;i++) press(&a,K_DOWN);
    assert(a.cursor==95 && a.view_x==6 && a.view_y==4);
    press(&a,K_ENTER);press(&a,K_ENTER);assert(a.board.node[95].type==SWITCH);
    press(&a,K_WIRE);for(int i=0;i<11;i++) press(&a,K_LEFT);
    for(int i=0;i<7;i++) press(&a,K_UP);
    assert(a.cursor==0 && a.source==95 && a.view_x==0 && a.view_y==0);
    press(&a,K_RIGHT);press(&a,K_RIGHT);press(&a,K_ENTER);
    assert(a.board.node[2].in[0]==95); /* connection from an offscreen source */
    /* Rendering every camera position clips long wires out of headers and footers. */
    for(uint8_t c=0;c<CELLS;c++) { a.cursor=c;app_draw(&a); }
    press(&a,K_UNDO);press(&a,K_UNDO);press(&a,K_UNDO);
    assert(a.board.cols==6 && a.board.rows==4 && a.cursor<24 && a.view_x==0 && a.view_y==0);
    press(&a,K_SIZE);assert(a.screen==SCREEN_SIZE && a.choice==0);press(&a,K_BACK);
    puts("PASS all board sizes, resize/remap/shrink protection, wire-free cells, scrolling, offscreen wiring and resize undo");
}
static void test_ui(void) {
    App a;app_init(&a);app_draw(&a);Board initial=a.board;
    press(&a,K_ENTER);assert(a.board.node[0].on==1);
    press(&a,K_UNDO);assert(!memcmp(&a.board,&initial,sizeof(Board)));
    press(&a,K_MENU);press(&a,K_DOWN);press(&a,K_DOWN);press(&a,K_DOWN);press(&a,K_ENTER);
    assert(a.screen==SCREEN_CONFIRM);press(&a,K_DOWN);press(&a,K_ENTER);
    assert(a.board.node[0].type==EMPTY);
    press(&a,K_ENTER);assert(a.screen==SCREEN_PARTS);press(&a,K_ENTER);assert(a.board.node[0].type==SWITCH);
    press(&a,K_ENTER);assert(a.board.node[0].on==1);
    press(&a,K_RIGHT);press(&a,K_PALETTE);press(&a,K_DOWN);press(&a,K_DOWN);press(&a,K_ENTER);
    assert(a.board.node[1].type==AND);
    press(&a,K_LEFT);press(&a,K_WIRE);press(&a,K_RIGHT);press(&a,K_ENTER);
    assert(a.board.node[1].in[0]==0 && a.source==NONE);
    press(&a,K_LEFT);press(&a,K_WIRE);press(&a,K_RIGHT);press(&a,K_PIN);press(&a,K_ENTER);
    assert(a.board.node[1].in[1]==0 && a.values[1]==HIGH);
    press(&a,K_LEFT);press(&a,K_WIRE);press(&a,K_RIGHT);press(&a,K_PIN);press(&a,K_DELETE);
    assert(a.board.node[1].in[1]==NONE && a.values[1]==UNKNOWN);
    press(&a,K_BACK);press(&a,K_UNDO);assert(a.board.node[1].in[1]==0);
    press(&a,K_DELETE);assert(a.board.node[1].type==EMPTY);press(&a,K_UNDO);assert(a.board.node[1].type==AND);
    a.cursor=0;for(int i=0;i<40;i++)press(&a,K_ENTER);assert(a.history==16);
    for(int i=0;i<20;i++)press(&a,K_UNDO);assert(a.history==0);
    a.cursor=0;press(&a,K_LEFT);press(&a,K_UP);assert(a.cursor==0);
    a.cursor=23;press(&a,K_RIGHT);press(&a,K_DOWN);assert(a.cursor==23);
    for(uint8_t sc=SCREEN_PARTS;sc<=SCREEN_SIZE;sc++) { a.screen=sc;a.choice=0;app_draw(&a); }
    for(uint8_t t=1;t<TYPES;t++) { board_place(&a.board,23,t);a.screen=SCREEN_BOARD;a.cursor=23;a.source=0;app_draw(&a); }
    a.screen=SCREEN_MENU;a.choice=6;press(&a,K_ENTER);assert(a.screen==SCREEN_EXIT);
    press(&a,K_DOWN);assert(app_key(&a,K_ENTER)==EFFECT_EXIT);
    puts("PASS actual UI placement, both pins, toggle, disconnect, deletion, undo, menus, render bounds");
}
static void test_save(void) {
    Board b,out,unchanged;uint8_t data[SAVE_SIZE];uint32_t g;
    board_demo(&b,2);save_encode(data,&b,UINT32_MAX);assert(save_decode(data,SAVE_SIZE,&out,&g));
    assert(g==UINT32_MAX && !memcmp(&b,&out,sizeof(b)));
    unchanged=out;
    for(unsigned i=0;i<SAVE_SIZE*8;i++) {
        data[i/8]^=1<<(i%8);assert(!save_decode(data,SAVE_SIZE,&out,&g));assert(!memcmp(&out,&unchanged,sizeof(out)));data[i/8]^=1<<(i%8);
    }
    b.node[0].type=255;save_encode(data,&b,0);assert(!save_decode(data,SAVE_SIZE,&out,&g));
    board_demo(&b,2);b.node[2].in[0]=2;save_encode(data,&b,0);assert(!save_decode(data,SAVE_SIZE,&out,&g));
    memset(files,0,sizeof(files));slot=-1;generation=0;app_init(&app);
    Board first=app.board;assert(save());assert(slot==0 && generation==1);
    app.board.node[0].on=1;Board second=app.board;assert(save());assert(slot==1 && generation==2);
    memset(&app.board,0,sizeof(app.board));slot=-1;generation=0;load();assert(slot==1 && !memcmp(&app.board,&second,sizeof(Board)));
    files[1].data[30]^=1;slot=-1;generation=0;load();assert(slot==0 && !memcmp(&app.board,&first,sizeof(Board)));
    short_write=true;assert(!save());short_write=false;slot=-1;generation=0;load();assert(slot==0 && !memcmp(&app.board,&first,sizeof(Board)));
    assert(save());assert(slot==1);slot=0;
    /* A short write is not mistaken for a complete save; an unrelated var is preserved. */
    files[1].size=9;memcpy(files[1].data,"UNRELATED",9);assert(!save());assert(!memcmp(files[1].data,"UNRELATED",9));
    files[1].exists=false;fail_open=true;assert(!save());fail_open=false;
    fail_archive=true;assert(!save());fail_archive=false;
    corrupt_read=true;assert(!save());corrupt_read=false;
    files[1].exists=false;assert(save());
    /* Sequence numbers compare across wraparound. */
    generation=UINT32_MAX;save_encode(files[slot].data,&app.board,generation);
    assert(save());assert(generation==0);slot=-1;load();assert(generation==0);
    puts("PASS save roundtrip, every single-bit corruption, invalid graphs, dual-slot recovery, failed I/O, name collision, generation wrap");
    board_demo(&b,1);assert(board_resize(&b,12,8));board_place(&b,95,SWITCH);
    b.node[95].on=1;assert(board_connect(&b,95,14,0));
    save_encode(data,&b,42);assert(save_decode(data,SAVE_SIZE,&out,&g));
    assert(g==42 && !memcmp(&b,&out,sizeof(b)));
    assert(!save_decode(data,SAVE_SIZE-1,&out,&g));
    /* Build a v1 108-byte record independently, then exercise migration and storage loading. */
    uint8_t legacy[LEGACY_SAVE_SIZE]={0};board_demo(&b,1);
    memcpy(legacy,"LGCE01",6);legacy[6]=7;
    for(unsigned i=0;i<24;i++) {
        legacy[10+4*i]=b.node[i].type;legacy[11+4*i]=b.node[i].on;
        legacy[12+4*i]=b.node[i].in[0];legacy[13+4*i]=b.node[i].in[1];
    }
    uint16_t crc=0xFFFF;
    for(unsigned i=0;i<106;i++) { crc^=legacy[i];for(int bit=0;bit<8;bit++)crc=(crc>>1)^((crc&1)?0xA001:0); }
    legacy[106]=crc&255;legacy[107]=crc>>8;
    assert(save_decode(legacy,sizeof(legacy),&out,&g));assert(g==7 && !memcmp(&b,&out,sizeof(b)));
    memset(files,0,sizeof(files));files[0].exists=true;files[0].size=sizeof(legacy);memcpy(files[0].data,legacy,sizeof(legacy));
    slot=-1;generation=0;load();assert(slot==0 && generation==7 && !memcmp(&app.board,&b,sizeof(b)));
    assert(save());assert(slot==1 && files[1].size==SAVE_SIZE);
    slot=-1;generation=0;load();assert(slot==1 && generation==8 && !memcmp(&app.board,&b,sizeof(b)));
    puts("PASS expanded-circuit save, length rejection, v1 save migration and first v2 save");
}
int main(void) {
    test_gates();test_circuits();test_ui();test_space();test_save();
    printf("ALL PASS (%u validated drawing rectangles)\n",rectangles);return 0;
}
