#ifndef HOST_CE_H
#define HOST_CE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
typedef uint8_t ti_var_t;
enum {kb_KeyUp,kb_KeyDown,kb_KeyLeft,kb_KeyRight,kb_KeyEnter,kb_Key2nd,kb_KeyYequ,kb_KeyWindow,kb_KeyDel,kb_KeyTrace,kb_KeyGraph,kb_KeyMode,kb_KeyClear,kb_KeyZoom};
extern uint16_t gfx_palette[256];
#define gfx_RGBTo1555(r,g,b) ((uint16_t)(((r)>>3)<<10|((g)>>3)<<5|((b)>>3)))
void gfx_Begin(void);
void gfx_End(void);
void gfx_SetDrawBuffer(void);
void gfx_BlitBuffer(void);
void gfx_SetColor(uint8_t c);
void gfx_FillRectangle(int x,int y,int w,int h);
void kb_Scan(void);
bool kb_AnyKey(void);
bool kb_IsDown(int key);
ti_var_t ti_Open(const char *name,const char *mode);
void ti_Close(ti_var_t h);
uint16_t ti_GetSize(ti_var_t h);
size_t ti_Read(void *p,size_t s,size_t n,ti_var_t h);
size_t ti_Write(const void *p,size_t s,size_t n,ti_var_t h);
bool ti_SetArchiveStatus(bool archive,ti_var_t h);
void ti_SetGCBehavior(void (*before)(void),void (*after)(void));
#endif
