#include "app.h"
#include <string.h>
static void checkpoint(App *a) {
    if(a->history==16) { memmove(a->undo,a->undo+1,15*sizeof(Board));a->history=15; }
    a->undo[a->history++]=a->board;a->dirty=1;
}
void app_init(App *a) {
    memset(a,0,sizeof(*a));a->source=NONE;
    board_demo(&a->board,1);a->message="HALF ADDER: SUM 11 / CARRY 17";
}
void app_camera(App *a) {
    if(a->cursor>=board_count(&a->board)) a->cursor=0;
    uint8_t x=a->cursor%a->board.cols,y=a->cursor/a->board.cols;
    if(a->view_x>a->board.cols-VIEW_COLS) a->view_x=a->board.cols-VIEW_COLS;
    if(a->view_y>a->board.rows-VIEW_ROWS) a->view_y=a->board.rows-VIEW_ROWS;
    if(x<a->view_x) a->view_x=x;
    if(x>=a->view_x+VIEW_COLS) a->view_x=x-VIEW_COLS+1;
    if(y<a->view_y) a->view_y=y;
    if(y>=a->view_y+VIEW_ROWS) a->view_y=y-VIEW_ROWS+1;
}
uint8_t app_key(App *a,uint8_t k) {
    if(k==K_NONE) return EFFECT_NONE;
    if(a->screen==SCREEN_HELP) { a->screen=SCREEN_BOARD;return EFFECT_NONE; }
    if(a->screen!=SCREEN_BOARD) {
        uint8_t count=a->screen==SCREEN_PARTS ? TYPES-1 : a->screen==SCREEN_MENU ? 8 : a->screen==SCREEN_DEMOS || a->screen==SCREEN_SIZE ? 3 : 2;
        if(k==K_BACK || k==K_MENU) { a->screen=SCREEN_BOARD;return EFFECT_NONE; }
        if(k==K_UP || k==K_LEFT) a->choice=(a->choice+count-1)%count;
        if(k==K_DOWN || k==K_RIGHT) a->choice=(a->choice+1)%count;
        if(k!=K_ENTER && k!=K_PALETTE) return EFFECT_NONE;
        if(a->screen==SCREEN_PARTS) {
            checkpoint(a);board_place(&a->board,a->cursor,a->choice+1);
            a->message="PLACED. Y= CONNECTS ITS OUTPUT.";
        } else if(a->screen==SCREEN_MENU) {
            switch(a->choice) {
                case 0: break;
                case 1: a->screen=SCREEN_BOARD;return EFFECT_SAVE;
                case 2: a->screen=SCREEN_DEMOS;a->choice=0;return EFFECT_NONE;
                case 3: a->screen=SCREEN_CONFIRM;a->choice=0;return EFFECT_NONE;
                case 4: a->screen=SCREEN_HELP;return EFFECT_NONE;
                case 5: return EFFECT_QUIT;
                case 6: a->screen=SCREEN_EXIT;a->choice=0;return EFFECT_NONE;
                case 7: a->screen=SCREEN_SIZE;a->choice=a->board.cols==6?0:a->board.cols==9?1:2;return EFFECT_NONE;
            }
        } else if(a->screen==SCREEN_DEMOS) {
            uint8_t cols=a->board.cols,rows=a->board.rows;
            checkpoint(a);board_demo(&a->board,a->choice);board_resize(&a->board,cols,rows);a->cursor=0;a->source=NONE;
            a->message=a->choice==0 ? "AND: BOTH SWITCHES ON = LIGHT ON" : a->choice==1 ? "HALF ADDER: XOR SUM / AND CARRY" : "FULL ADDER: ADD THREE SWITCHES";
        } else if(a->screen==SCREEN_SIZE) {
            static const uint8_t cols[]={6,9,12},rows[]={4,6,8};
            Board next=a->board;
            if(board_resize(&next,cols[a->choice],rows[a->choice])) {
                uint8_t x=a->cursor%a->board.cols,y=a->cursor/a->board.cols;
                if(next.cols!=a->board.cols || next.rows!=a->board.rows) { checkpoint(a);a->board=next; }
                a->cursor=x<next.cols && y<next.rows ? y*next.cols+x : 0;
                a->source=NONE;app_camera(a);a->message="RESIZED. ARROWS SCROLL THE BOARD.";
            } else a->message="SHRINK WOULD CUT PARTS. NOT CHANGED.";
        } else if(a->screen==SCREEN_EXIT && a->choice==1) {
            return EFFECT_EXIT;
        } else if(a->screen==SCREEN_CONFIRM && a->choice==1) {
            uint8_t cols=a->board.cols,rows=a->board.rows;
            checkpoint(a);board_clear(&a->board);board_resize(&a->board,cols,rows);a->source=NONE;a->message="EMPTY BOARD. ENTER TO PLACE.";
        }
        a->screen=SCREEN_BOARD;return EFFECT_NONE;
    }
    if(k==K_UP && a->cursor>=a->board.cols) a->cursor-=a->board.cols;
    if(k==K_DOWN && a->cursor<board_count(&a->board)-a->board.cols) a->cursor+=a->board.cols;
    if(k==K_LEFT && a->cursor%a->board.cols) a->cursor--;
    if(k==K_RIGHT && a->cursor%a->board.cols<a->board.cols-1) a->cursor++;
    Node *n=&a->board.node[a->cursor];
    if(k==K_BACK) {
        if(a->source!=NONE) { a->source=NONE;a->message="WIRE CANCELLED"; }
        else { a->screen=SCREEN_MENU;a->choice=0; }
    }
    if(k==K_MENU) { a->source=NONE;a->screen=SCREEN_MENU;a->choice=0; }
    if(k==K_SIZE) { a->source=NONE;a->screen=SCREEN_SIZE;a->choice=a->board.cols==6?0:a->board.cols==9?1:2; }
    if(k==K_UNDO) {
        if(a->history) {
            uint8_t x=a->cursor%a->board.cols,y=a->cursor/a->board.cols;
            a->board=a->undo[--a->history];a->cursor=x<a->board.cols && y<a->board.rows ? y*a->board.cols+x : 0;
            a->dirty=1;a->source=NONE;a->message="UNDONE";
        }
        else a->message="NOTHING TO UNDO";
    }
    if(k==K_PIN) { a->pin^=1;a->message=a->pin ? "TARGET INPUT B SELECTED" : "TARGET INPUT A SELECTED"; }
    if(k==K_WIRE) {
        if(a->source!=NONE) { a->source=NONE;a->message="WIRE CANCELLED"; }
        else if(n->type!=EMPTY) { a->source=a->cursor;a->pin=0;a->message="PICK TARGET. WINDOW: A/B PIN."; }
        else a->message="SELECT A PART TO WIRE FROM";
    }
    if(k==K_DELETE) {
        if(a->source!=NONE) {
            uint8_t p=pins(n->type)==1 ? 0 : a->pin;
            if(p<pins(n->type) && n->in[p]!=NONE) { checkpoint(a);n->in[p]=NONE;a->message="INPUT DISCONNECTED"; }
        } else if(n->type!=EMPTY) { checkpoint(a);board_delete(&a->board,a->cursor);a->message="PART REMOVED. TRACE: UNDO."; }
    }
    if(k==K_PALETTE && a->source==NONE) {
        a->screen=SCREEN_PARTS;a->choice=n->type ? n->type-1 : 0;
    }
    if(k==K_ENTER) {
        if(a->source!=NONE) {
            uint8_t p=pins(n->type)==1 ? 0 : a->pin;
            Board next=a->board;
            if(board_connect(&next,a->source,a->cursor,p)) {
                checkpoint(a);a->board=next;a->source=NONE;a->message="CONNECTED. GREEN = 1 / GRAY = 0";
            } else a->message="NO INPUT OR LOOP. PICK ELSEWHERE.";
        } else if(n->type==SWITCH) {
            checkpoint(a);n->on^=1;a->message=n->on ? "SWITCH ON" : "SWITCH OFF";
        } else if(n->type==EMPTY) { a->screen=SCREEN_PARTS;a->choice=0; }
        else a->message="Y= WIRE / 2ND CHANGE / DEL REMOVE";
    }
    return EFFECT_NONE;
}
