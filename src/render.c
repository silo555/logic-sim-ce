#include "app.h"
#include <stdio.h>
#include <string.h>
/* Small pixel font, drawn identically by the CE and desktop backends. */
static const char charset[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ?=:/-.+>";
static const uint8_t font[][5]={
 {126,9,9,9,126},{127,73,73,73,54},{62,65,65,65,34},{127,65,65,34,28},
 {127,73,73,73,65},{127,9,9,9,1},{62,65,73,73,122},{127,8,8,8,127},
 {65,65,127,65,65},{32,64,65,63,1},{127,8,20,34,65},{127,64,64,64,64},
 {127,2,12,2,127},{127,4,8,16,127},{62,65,65,65,62},{127,9,9,9,6},
 {62,65,81,33,94},{127,9,25,41,70},{38,73,73,73,50},{1,1,127,1,1},
 {63,64,64,64,63},{31,32,64,32,31},{63,64,56,64,63},{99,20,8,20,99},
 {3,4,120,4,3},{97,81,73,69,67},
 {62,81,73,69,62},{0,66,127,64,0},{98,81,73,73,70},{34,65,73,73,54},
 {24,20,18,127,16},{39,69,69,69,57},{60,74,73,73,48},{1,113,9,5,3},
 {54,73,73,73,54},{6,73,73,41,30},
 {0,0,0,0,0},{2,1,81,9,6},{20,20,20,20,20},{0,54,54,0,0},
 {32,16,8,4,2},{8,8,8,8,8},{0,96,96,0,0},{8,8,62,8,8},{65,34,20,8,0}
};
/* BG, panel, grid, ink, muted, high, unknown, selection, low, dark ink, red, pale */
const uint8_t colors[12][3]={
 {12,18,28},{24,35,49},{40,55,70},{226,237,246},{142,160,177},{79,225,147},
 {255,192,75},{94,177,255},{102,124,143},{9,34,25},{255,110,119},{192,223,255}
};
static void box(int x,int y,int w,int h,uint8_t c) {
    draw_rect(x,y,w,1,c);draw_rect(x,y+h-1,w,1,c);
    draw_rect(x,y,1,h,c);draw_rect(x+w-1,y,1,h,c);
}
static void text(const char *s,int x,int y,uint8_t c) {
    for(;*s && x<315;s++,x+=6) {
        const char *p=strchr(charset,*s);unsigned i=p ? (unsigned)(p-charset) : 37;
        for(int col=0;col<5;col++) for(int row=0;row<7;row++)
            if(font[i][col]&(1<<row)) draw_rect(x+col,y+row,1,1,c);
    }
}
/* Wires live in the gaps, independently of the cells occupied by parts.
   Clip only wire segments to the board viewport, never the status bars. */
static void wire_rect(int x,int y,int w,int h,uint8_t c) {
    if(x<0) { w+=x;x=0; } if(y<40) { h+=y-40;y=40; }
    if(x+w>320) w=320-x;if(y+h>198) h=198-y;
    if(w>0 && h>0) draw_rect(x,y,w,h,c);
}
static void hline(int a,int b,int y,uint8_t c) { if(a>b) { int t=a;a=b;b=t; }wire_rect(a,y,b-a+1,1,c); }
static void vline(int x,int a,int b,uint8_t c) { if(a>b) { int t=a;a=b;b=t; }wire_rect(x,a,1,b-a+1,c); }
static int nx(const App *a,uint8_t c) { return 10+((int)(c%a->board.cols)-a->view_x)*53; }
static int ny(const App *a,uint8_t c) { return 52+((int)(c/a->board.cols)-a->view_y)*38; }
static bool visible(const App *a,uint8_t c) {
    int x=(c%a->board.cols)-a->view_x,y=(c/a->board.cols)-a->view_y;
    return x>=0 && x<VIEW_COLS && y>=0 && y<VIEW_ROWS;
}
static uint8_t signal_color(uint8_t v) { return v==HIGH ? 5 : v==LOW ? 8 : 6; }
static char signal_char(uint8_t v) { return v==UNKNOWN ? '?' : '0'+v; }
static void frame(const char *title,const char *subtitle) {
    draw_rect(0,0,320,240,0);draw_rect(0,0,320,23,1);
    text(title,8,8,3);text(subtitle,8,29,4);
}
static void menu(const char *title,const char *const *items,uint8_t count,uint8_t choice) {
    frame(title,"ARROWS: CHOOSE / ENTER: SELECT");
    for(uint8_t i=0;i<count;i++) {
        if(i==choice) draw_rect(7,44+i*16,306,15,7);
        text(items[i],14,48+i*16,i==choice?0:3);
    }
    text("CLEAR: BACK",8,226,4);
}
void app_draw(App *a) {
    static const char *const parts[]={"SWITCH - TOGGLE 0/1","LIGHT - OUTPUT PROBE","AND","OR","NOT","NAND","NOR","XOR","XNOR","BUFFER"};
    static const char *const menus[]={"RESUME","SAVE CIRCUIT","LOAD A DEMO","NEW EMPTY BOARD","HOW TO PLAY","SAVE AND QUIT","QUIT WITHOUT SAVING","SANDBOX SIZE"};
    static const char *const demos[]={"AND - FIRST CIRCUIT","HALF ADDER - TWO BITS","FULL ADDER - THREE BITS"};
    static const char *const confirm[]={"KEEP MY CIRCUIT","CLEAR BOARD - CAN UNDO"};
    static const char *const sizes[]={"SMALL 6X4 - 24 PARTS","MEDIUM 9X6 - 54 PARTS","LARGE 12X8 - 96 PARTS"};
    static const char *const quit[]={"KEEP PLAYING","QUIT - LOSE UNSAVED CHANGES"};
    if(a->screen==SCREEN_PARTS) { menu("PARTS",parts,10,a->choice);return; }
    if(a->screen==SCREEN_MENU) { menu("LOGIC SIM CE / MENU",menus,8,a->choice);return; }
    if(a->screen==SCREEN_DEMOS) { menu("EXAMPLE CIRCUITS",demos,3,a->choice);return; }
    if(a->screen==SCREEN_CONFIRM) { menu("START AN EMPTY BOARD?",confirm,2,a->choice);return; }
    if(a->screen==SCREEN_SIZE) { menu("SANDBOX SIZE / KEEPS YOUR PARTS",sizes,3,a->choice);return; }
    if(a->screen==SCREEN_EXIT) { menu("QUIT WITHOUT SAVING?",quit,2,a->choice);return; }
    if(a->screen==SCREEN_HELP) {
        frame("HOW TO PLAY","A SMALL DIGITAL LOGIC SANDBOX");
        const char *lines[]={
          "ARROWS   MOVE / SCROLL THE BOARD",
          "ENTER    PLACE / TOGGLE SWITCH",
          "2ND      ADD OR CHANGE A PART",
          "Y=       START WIRE FROM THIS PART",
          "ARROWS   PICK DESTINATION PART",
          "WINDOW   CHOOSE ITS INPUT A OR B",
          "ENTER    CONNECT THE WIRE",
          "DEL      REMOVE PART / WIRED INPUT",
          "TRACE    UNDO UP TO 16 CHANGES",
          "GRAPH    MENU / SIZE / SAVE / QUIT",
          "CLEAR    CANCEL WIRE / OPEN MENU",
          "GREEN=1 / GRAY=0 / AMBER=?",
          "WIRES USE GAPS - ALL CELLS FREE",
          "ZOOM     CHANGE SANDBOX SIZE"
        };
        for(uint8_t i=0;i<14;i++) text(lines[i],8,47+i*12,i>=11?6:3);
        text("ANY KEY: BACK",8,226,4);return;
    }
    app_camera(a);
    board_eval(&a->board,a->values);
    frame("LOGIC SIM CE",a->message ? a->message : "ENTER: PLACE OR TOGGLE / 2ND: PARTS");
    char size[16];snprintf(size,sizeof(size),"%uX%u",(unsigned)a->board.cols,(unsigned)a->board.rows);text(size,215,8,4);
    snprintf(size,sizeof(size),"VIEW %u:%u",(unsigned)a->view_x+1,(unsigned)a->view_y+1);text(size,128,8,4);
    text(a->source==NONE ? "SANDBOX" : "WIRING",260,8,a->source==NONE?5:6);
    for(uint8_t i=0;i<CELLS;i++) {
        int x=nx(a,i),y=ny(a,i);if(visible(a,i)) draw_rect(x+17,y+12,2,2,2);
        for(uint8_t p=0;p<pins(a->board.node[i].type);p++) {
            uint8_t s=a->board.node[i].in[p];if(s==NONE) continue;
            int sx=nx(a,s)+37,sy=ny(a,s)+12,ty=y+(pins(a->board.node[i].type)==1?12:p==0?6:19);
            int exit=sx+5,entry=x-5,lane=ny(a,s)+26;
            uint8_t c=signal_color(a->values[s]);
            hline(sx,exit,sy,c);vline(exit,sy,lane,c);hline(exit,entry,lane,c);vline(entry,lane,ty,c);hline(entry,x-3,ty,c);
        }
    }
    for(uint8_t i=0;i<CELLS;i++) {
        if(!visible(a,i)) continue;
        const Node *n=&a->board.node[i];int x=nx(a,i),y=ny(a,i);
        char num[4];snprintf(num,sizeof(num),"%02u",(unsigned)i+1);
        text(num,x-6,y-9,4);
        if(n->type!=EMPTY) {
            uint8_t c=signal_color(a->values[i]);
            draw_rect(x,y,37,25,1);box(x,y,37,25,i==a->source?6:2);
            text(n->type==SWITCH?"SW":n->type==LIGHT?"LED":names[n->type],x+4,y+4,3);
            char state[2]={signal_char(a->values[i]),0};text(state,x+27,y+15,c);
            if(n->type==SWITCH) {
                draw_rect(x+5,y+15,17,6,2);draw_rect(x+(n->on?15:5),y+15,7,6,c);
            } else if(n->type==LIGHT) draw_rect(x+5,y+15,15,6,c);
            else { draw_rect(x+5,y+17,15,2,c); }
            draw_rect(x+37,y+11,3,4,c);
            for(uint8_t p=0;p<pins(n->type);p++) {
                int py=y+(pins(n->type)==1?12:p==0?6:19);
                uint8_t s=n->in[p];draw_rect(x-3,py-1,4,4,s==NONE?6:signal_color(a->values[s]));
                if(i==a->cursor && a->source!=NONE && (pins(n->type)==1 || p==a->pin)) box(x-5,py-3,8,8,7);
            }
        }
    }
    int cx=nx(a,a->cursor)-8,cy=ny(a,a->cursor)-10;
    box(cx,cy,52,38,7);
    if(a->board.node[a->cursor].type==EMPTY) text("PLACE",cx+11,cy+18,4);
    const Node *n=&a->board.node[a->cursor];char info[53],av[4],bv[4];
    if(n->in[0]==NONE) strcpy(av,"--");else snprintf(av,sizeof(av),"%02u",(unsigned)n->in[0]+1);
    if(n->in[1]==NONE) strcpy(bv,"--");else snprintf(bv,sizeof(bv),"%02u",(unsigned)n->in[1]+1);
    draw_rect(0,199,320,24,1);
    if(a->source!=NONE) snprintf(info,sizeof(info),"WIRE %02u > %02u / INPUT %c",(unsigned)a->source+1,(unsigned)a->cursor+1,pins(n->type)==1?'A':'A'+a->pin);
    else snprintf(info,sizeof(info),"C%02u R%02u %s=%c A:%s B:%s",(unsigned)(a->cursor%a->board.cols)+1,(unsigned)(a->cursor/a->board.cols)+1,names[n->type],signal_char(a->values[a->cursor]),av,bv);
    text(info,7,203,3);
    text(a->source!=NONE?"ENTER CONNECT / WINDOW A/B / CLEAR CANCEL":"ENTER ACT / 2ND PARTS / Y= WIRE",7,214,4);
    text("ZOOM SIZE / TRACE UNDO / GRAPH MENU",7,230,4);
}
