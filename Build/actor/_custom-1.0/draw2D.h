#ifndef RGBA_DRAW_H
#define RGBA_DRAW_H

#include <z64hdr/oot_u10/z64hdr.h>

#define RGBA16 0
#define RGBA16_Setup39 1
#define RGBA32 2
#define RGBA32_Setup39 3
#define I8 4
#define I8_Setup39 5
#define IA8 6
#define IA8_Setup39 7
#define IA16 8
#define IA16_Setup39 9
#define I4 10
#define I4_Setup39 11
#define CI4 12
#define CI4_Setup39 13
#define CI8 14
#define CI8_Setup39 15
#define IA4 16
#define IA4_Setup39 17

#define SCALE_DEFAULT 1.0

#define GET_DSD(textureSize, scaledSize) (1024 * (textureSize) / (scaledSize))

extern void Draw2DScaled(u8 RGBAType, int object, PlayState* playState, Gfx** gfxp, s16 centerX, s16 centerY, u8* source, u8* sourcePal, u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha);
	asm("Draw2DScaled = 0x800755C8"); 
    
extern void Draw2DInternal(u8 RGBAType, u8* texture, u8* palette, Gfx** gfxp, s16 centerX, s16 centerY, u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha);
    asm("Draw2DInternal = 0x800D7B00");

static inline void Draw2D(u8 RGBAType, int object, PlayState* playState, Gfx** gfxp, s16 centerX, s16 centerY, u8* source, u8* sourcePal, u32 width, u32 height, s16 alpha)
{
	Draw2DScaled(RGBAType, object, playState, gfxp, centerX, centerY, source, sourcePal, width, height, width, height, alpha);
}
	
#endif