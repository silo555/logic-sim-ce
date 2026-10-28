#ifndef APP_H
#define APP_H
#include "logic.h"
enum { K_NONE,K_UP,K_DOWN,K_LEFT,K_RIGHT,K_ENTER,K_PALETTE,K_WIRE,K_PIN,K_DELETE,K_UNDO,K_MENU,K_BACK,K_SIZE };
enum { SCREEN_BOARD,SCREEN_PARTS,SCREEN_MENU,SCREEN_DEMOS,SCREEN_HELP,SCREEN_CONFIRM,SCREEN_EXIT,SCREEN_SIZE };
enum { EFFECT_NONE,EFFECT_SAVE,EFFECT_QUIT,EFFECT_EXIT };
typedef struct {
    Board board, undo[16];
    uint8_t cursor, pin, source, screen, choice, history, view_x, view_y;
    uint8_t values[CELLS], dirty;
    const char *message;
} App;
void app_init(App *a);
uint8_t app_key(App *a,uint8_t key);
void app_camera(App *a);
void app_draw(App *a);
/* Drawing backend: color indices use the palette in render.c. */
void draw_rect(int x,int y,int w,int h,uint8_t color);
extern const uint8_t colors[12][3];
#endif
