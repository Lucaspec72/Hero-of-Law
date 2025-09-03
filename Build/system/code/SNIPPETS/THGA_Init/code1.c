#include "global.h"

extern void THGA_Init_New(TwoHeadGfxArena* thga, void* start, size_t size);
    asm("THGA_Init_New = 0x8009FEC0");
    
extern void* SysCfb_GetFbPtr(s32 idx);
    asm("SysCfb_GetFbPtr = 0x800A43D8");    

typedef struct {
    /* 0x00000 */ u16 headMagic; // GFXPOOL_HEAD_MAGIC
    /* 0x00008 */ Gfx polyOpaBuffer[0x4FE0];
    /* 0x27F08 */ Gfx polyXluBuffer[0x800];
    /* 0x2BF08 */ Gfx overlayBuffer[0x400];
    /* 0x2DF08 */ Gfx workBuffer[0x80];
                  u32 emulatorIndicator;                // 0x80198948 Wii/WiiU Check
                  OSPiHandle* sISVHandle;               // 0x8019894C is64Printf
                  u32 hzChoice;                         // 0x80198950 60/50hz 
                  u32 tpakBeingUsed;                    // 0x80198954 Set to 1 when the tpak actor is accessing stuff. 
                  u32 savingGfxCounter;                 // 0x80198958 Set when saving
    /* 0x2E308 */ Gfx unusedBuffer[0x1D];               
    /* 0x2E408 */ u16 tailMagic; // GFXPOOL_TAIL_MAGIC
} MyGfxPool; // size = 0x2E410

#define GFXPOOL_HEAD_MAGIC 0x1234
#define GFXPOOL_TAIL_MAGIC 0x5678  

void Graph_InitTHGA_New(GraphicsContext* gfxCtx)
{
    MyGfxPool* gfxPools = (MyGfxPool*)0x8016A640;
    MyGfxPool* pool = &gfxPools[gfxCtx->gfxPoolIdx & 1];

    pool->headMagic = GFXPOOL_HEAD_MAGIC;
    pool->tailMagic = GFXPOOL_TAIL_MAGIC;
    
    THGA_Init_New(&gfxCtx->polyOpa, pool->polyOpaBuffer, sizeof(pool->polyOpaBuffer));
    THGA_Init_New(&gfxCtx->polyXlu, pool->polyXluBuffer, sizeof(pool->polyXluBuffer));
    THGA_Init_New(&gfxCtx->overlay, pool->overlayBuffer, sizeof(pool->overlayBuffer));
    THGA_Init_New(&gfxCtx->work, pool->workBuffer, sizeof(pool->workBuffer));

    gfxCtx->polyOpaBuffer = pool->polyOpaBuffer;
    gfxCtx->polyXluBuffer = pool->polyXluBuffer;
    gfxCtx->overlayBuffer = pool->overlayBuffer;
    gfxCtx->workBuffer = pool->workBuffer;

    gfxCtx->curFrameBuffer = SysCfb_GetFbPtr(gfxCtx->fbIdx % 2);
}