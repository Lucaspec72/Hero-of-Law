#include <z64hdr/oot_u10/z64hdr.h>

#include "common.h"
#include "is64Printf.h"

#ifndef HOLTEXT_DRAW_H
#define HOLTEXT_DRAW_H

#define TEXT_SCALE 75

typedef struct Vec2s
{
    s16 x;
    s16 y;
} Vec2s;

typedef enum
{
    OPERATION_DRAW = 0,
    OPERATION_DRAW_SHADOW = 1,
    OPERATION_DRAW_CREDITS = 2,
    OPERATION_DRAW_CREDITS_SHADOW = 3,
    OPERATION_DRAW_INDIVIDUAL = 4,
    OPERATION_DRAW_INDIVIDUAL_SHADOW = 5,
    OPERATION_SET_POSITIONS = 6,
    OPERATION_EVALUATE_YSIZE = 7,
    OPERATION_SET_POSITIONS_CREDITS = 8,
    OPERATION_EVALUATE_LINE_XSIZE = 9,
    OPERATION_EVALUATE_XSIZE = 10,
    OPERATION_EVALUATE_DIMENSIONS = 11,
} TextboxOperation;

typedef enum
{
    TEXT_COLOR_DEFAULT,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_LIGHTBLUE,
    TEXT_COLOR_PURPLE,
    TEXT_COLOR_YELLOW,
    TEXT_COLOR_BLACK
} TextColor;

extern int HoL_DrawMessageTextInternal(PlayState* play, u8* fontBuf, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, s16 alpha, s16 shadowAlpha, char* msgData, int posX, int posY, u8 shadowOffsetX, u8 shadowOffsetY, Vec2s* positions, float scale, int operation);
	asm("HoL_DrawMessageTextInternal = 0x80078298"); 

static inline int HoL_DrawMessageText(PlayState* play, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, s16 alpha, s16 shadowAlpha, char* msgData, int posX, int posY, u8 shadowOffsetX, u8 shadowOffsetY, Vec2s* positions, float scale, int operation)
{
    return HoL_DrawMessageTextInternal(play, NULL, gfxp, Color, ShadowColor, alpha, shadowAlpha, msgData, posX, posY, shadowOffsetX, shadowOffsetY, positions, scale, operation);
}

static inline int GetTextPxWidth(char* string, float scale)
{
    return HoL_DrawMessageTextInternal(NULL, NULL, NULL, (Color_RGB8){0,0,0}, (Color_RGB8){0,0,0}, 0, 0, string, 0, 0, 0, 0, NULL, scale, OPERATION_EVALUATE_LINE_XSIZE);
}

extern int GetStringCenterX(char* string, float scale);
	asm("GetStringCenterX = 0x800d69ec");  

extern void DrawCharTexture(PlayState* play, Gfx** gfxp, u8* texture, s32 x, s32 y, float scale, bool loadGfx,s16 alpha, Color_RGB8 Color, Color_RGB8 ShadowColor, bool drawShadow, s16 shadowAlpha, u8 shadowOffsetX, u8 shadowOffsetY);
	asm("DrawCharTexture = 0x800D7B00");    
    

#ifdef GET_DIMENSIONS_FUNCS
    Vec2s GetTextDimensions(char* string, int scale)
    {
        int res = HoL_DrawMessageTextInternal(NULL, NULL, NULL, (Color_RGB8){0,0,0}, (Color_RGB8){0,0,0}, 0, 0, string, 0, 0, 0, 0, NULL, scale, OPERATION_EVALUATE_DIMENSIONS);
        Vec2s ret;
           
        ret.x = res & 0xFFFF;
        ret.y = res >> 16;
       
        return ret;
    }

    int GetTextScaleToFit(char* string, int scaleMax, int maxXSize, int maxYSize)
    {
        Vec2s dimensions = GetTextDimensions(string, scaleMax);
       
        float scale = scaleMax;
        float scaleX = MIN(75, 75 * ((float)maxXSize / (float)dimensions.x));
        float scaleY = MIN(75, 75 * ((float)maxYSize / (float)dimensions.y));

        if (((scaleY / scale) * dimensions.x) <= maxXSize)
            scale = scaleY;
        else if (((scaleX / scale) * dimensions.y) <= maxYSize)
            scale = scaleX;
        else
            scale = scaleMax * ((scaleX / scale) * (scaleY / scale));  
        
        return scale; 
    }    
#endif

#endif