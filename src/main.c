#ifdef HOST_TEST
#include "host_ce.h"
#else
#include <tice.h>
#include <graphx.h>
#include <keypadc.h>
#include <fileioc.h>
#endif
#include <string.h>
#include "app.h"
#include "save.h"
static App app;
static int slot=-1;
static uint32_t generation;
static const char *const slots[]={"LGSAVE0","LGSAVE1"};
void draw_rect(int x,int y,int w,int h,uint8_t c) {
    gfx_SetColor(c);gfx_FillRectangle(x,y,w,h);
}
static uint8_t key(void) {
    do { kb_Scan(); } while(kb_AnyKey());
    for(;;) {
        kb_Scan();
        if(kb_IsDown(kb_KeyUp)) return K_UP;
        if(kb_IsDown(kb_KeyDown)) return K_DOWN;
        if(kb_IsDown(kb_KeyLeft)) return K_LEFT;
        if(kb_IsDown(kb_KeyRight)) return K_RIGHT;
        if(kb_IsDown(kb_KeyEnter)) return K_ENTER;
        if(kb_IsDown(kb_Key2nd)) return K_PALETTE;
        if(kb_IsDown(kb_KeyYequ)) return K_WIRE;
        if(kb_IsDown(kb_KeyWindow)) return K_PIN;
        if(kb_IsDown(kb_KeyZoom)) return K_SIZE;
        if(kb_IsDown(kb_KeyDel)) return K_DELETE;
        if(kb_IsDown(kb_KeyTrace)) return K_UNDO;
        if(kb_IsDown(kb_KeyGraph) || kb_IsDown(kb_KeyMode)) return K_MENU;
        if(kb_IsDown(kb_KeyClear)) return K_BACK;
    }
}
static void load(void) {
    Board best;uint8_t bytes[SAVE_SIZE];bool found=false;
    for(int s=0;s<2;s++) {
        ti_var_t h=ti_Open(slots[s],"r");
        if(h) {
            uint32_t g;Board b;
            uint16_t size=ti_GetSize(h);
            bool valid=(size==SAVE_SIZE || size==LEGACY_SAVE_SIZE) && ti_Read(bytes,1,size,h)==size && save_decode(bytes,size,&b,&g);
            ti_Close(h);
            if(valid && (!found || (int32_t)(g-generation)>0)) { best=b;generation=g;slot=s;found=true; }
        }
    }
    if(found) { app.board=best;app.message="CIRCUIT RESTORED"; }
}
static bool save(void) {
    uint8_t bytes[SAVE_SIZE],verify[SAVE_SIZE];int next=slot==0?1:0;
    ti_var_t h=ti_Open(slots[next],"r");
    if(h) {
        /* Do not overwrite an unrelated AppVar with the same name. */
        bool own=ti_GetSize(h)>=6 && ti_Read(verify,1,6,h)==6 && !memcmp(verify,"LGCE0",5) && (verify[5]=='1' || verify[5]=='2');
        ti_Close(h);if(!own) return false;
    }
    save_encode(bytes,&app.board,generation+1);h=ti_Open(slots[next],"w");
    if(!h) return false;
    bool ok=ti_Write(bytes,1,SAVE_SIZE,h)==SAVE_SIZE;
    ti_Close(h);
    if(!ok) return false;
    h=ti_Open(slots[next],"r");if(!h) return false;
    ok=ti_GetSize(h)==SAVE_SIZE && ti_Read(verify,1,SAVE_SIZE,h)==SAVE_SIZE && !memcmp(verify,bytes,SAVE_SIZE);
    if(ok) ok=ti_SetArchiveStatus(true,h);
    ti_Close(h);
    if(ok) { slot=next;generation++;app.dirty=0; }
    return ok;
}
static void gc_before(void) { gfx_End(); }
static void gc_after(void) {
    gfx_Begin();gfx_SetDrawBuffer();
    for(uint8_t i=0;i<12;i++) gfx_palette[i]=gfx_RGBTo1555(colors[i][0],colors[i][1],colors[i][2]);
}
int main(void) {
    app_init(&app);load();gc_after();ti_SetGCBehavior(gc_before,gc_after);
    for(;;) {
        app_draw(&app);gfx_BlitBuffer();
        uint8_t effect=app_key(&app,key());
        if(effect==EFFECT_EXIT) break;
        if(effect==EFFECT_SAVE || effect==EFFECT_QUIT) {
            if(save()) { app.message="SAVED TO ARCHIVE";if(effect==EFFECT_QUIT) break; }
            else { app.screen=SCREEN_BOARD;app.message="SAVE FAILED. GRAPH TO RETRY."; }
        }
    }
    gfx_End();return 0;
}
