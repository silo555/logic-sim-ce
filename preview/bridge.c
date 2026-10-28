#include "app.h"
#include "save.h"
#include <string.h>
#include <assert.h>
static App app;
static uint8_t image[320*240],snapshot[SAVE_SIZE];
void draw_rect(int x,int y,int w,int h,uint8_t c) {
    assert(x>=0 && y>=0 && w>0 && h>0 && x+w<=320 && y+h<=240 && c<12);
    for(int row=y;row<y+h;row++) memset(image+row*320+x,c,(size_t)w);
}
void preview_init(void) { app_init(&app);app_draw(&app); }
int preview_key(int k) {
    int effect=app_key(&app,(uint8_t)k);
    if(effect==EFFECT_SAVE || effect==EFFECT_QUIT) {
        app.screen=SCREEN_BOARD;app.message="SAVED IN DESKTOP PREVIEW";
    }
    if(effect==EFFECT_EXIT) { app.screen=SCREEN_BOARD;app.message="CALCULATOR WOULD EXIT HERE"; }
    app_draw(&app);return effect;
}
const uint8_t *preview_pixels(void) { return image; }
const uint8_t *preview_colors(void) { return &colors[0][0]; }
const uint8_t *preview_save(void) { save_encode(snapshot,&app.board,0);return snapshot; }
int preview_load(const uint8_t *bytes,int size) {
    Board b;uint32_t g;if(!save_decode(bytes,(size_t)size,&b,&g)) return 0;
    app_init(&app);app.board=b;app.message="DESKTOP CIRCUIT RESTORED";app_draw(&app);return 1;
}
int preview_value(int c) { return c<0 || c>=CELLS ? UNKNOWN : app.values[c]; }
int preview_cursor(void) { return app.cursor; }
int preview_type(int c) { return c<0 || c>=CELLS ? EMPTY : app.board.node[c].type; }
int preview_screen(void) { return app.screen; }

int preview_save_size(void) { return SAVE_SIZE; }
int preview_count(void) { return board_count(&app.board); }
int preview_cols(void) { return app.board.cols; }
int preview_rows(void) { return app.board.rows; }
