#include "global.h"
#include "message_data_static.h"
#include "sfx.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"

#define CHARSIZEX 16
#define CHARSIZEY 16

//0x80078290
void InterfaceUpdate()
{
}

//0x80078290 + 0x8
void DrawCharTexture(Gfx** gfxp, u8* texture, s32 x, s32 y, int scaleX, int scaleY, bool loadGfx, s16 alpha, 
                     Color_RGB8 Color, Color_RGB8 ShadowColor, bool drawShadow, s16 shadowAlpha, u8 shadowOffsetX, u8 shadowOffsetY, bool noWidescreenAdjust)
{
    if (texture == NULL)
        return;
    
    Gfx* gfx = *gfxp;
    
    s32 sCharTexSizeX = (CHARSIZEX * scaleX + 50) / 100;
    s32 sCharTexSizeY = (CHARSIZEY * scaleY + 50) / 100;

    float sCharTexScaleX = GET_DSD(CHARSIZEX, sCharTexSizeX); 
    float sCharTexScaleY = GET_DSD(CHARSIZEY, sCharTexSizeY); 
   
    if (SAVE_WIDESCREEN && !noWidescreenAdjust)
        sCharTexScaleX /= WIDESCREEN_SCALEX;

    if (loadGfx)
    {
        gDPLoadTextureBlock_4b(gfx++, texture, G_IM_FMT_I, CHARSIZEX, CHARSIZEY, 0, G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
                            G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
    
    if (drawShadow)
    {
        gDPPipeSync(gfx++);
        gDPSetPrimColor(gfx++, 0, 0, ShadowColor.r, ShadowColor.g, ShadowColor.b, shadowAlpha);
        gSPScisTextureRectangle(gfx++, 
                                (x + shadowOffsetX) << 2, 
                                (y + shadowOffsetY) << 2, 
                                ((x + shadowOffsetX) + sCharTexSizeX) << 2, 
                                (((y + shadowOffsetY) + sCharTexSizeY) << 2), 
                                G_TX_RENDERTILE, 
                                0, 
                                0, 
                                (s32)sCharTexScaleX, 
                                (s32)sCharTexScaleY);
    }
    
    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, Color.r, Color.g, Color.b, alpha);    
    gSPScisTextureRectangle (gfx++, x << 2, (y << 2), (x + sCharTexSizeX) << 2, ((y + sCharTexSizeY) << 2), G_TX_RENDERTILE, 0, 0, (s32)sCharTexScaleX, (s32)sCharTexScaleY);

    *gfxp = gfx;
}