#ifndef HOLTEXT_DRAW_H
#define HOLTEXT_DRAW_H

#include <z64hdr/oot_u10/z64hdr.h>

#include "common.h"
#include "is64Printf.h"
#define gTextSpeed play->msgCtx.textboxBackgroundBackColorIdx

#define CHARSIZEX 16
#define CHARSIZEY 16
#define TEXT_SCALE 75

// 0x270 of extra free memory for stuff here
// Takes place of sFontWidths which are 0x370, and the new font widths only take up 0x100
u8* fontWidthsDefault = (u8*)0x80112F40;
u8* fontWidthsLanguage = (u8*)0x80112FC0;

typedef struct Vec2s
{
    s16 x;
    s16 y;
} Vec2s;

typedef enum
{
    OPERATION_TYPE_OUT = 0,
    OPERATION_DRAW = 1,
    OPERATION_DRAW_SHADOW = 2,
    OPERATION_DRAW_CREDITS = 3,
    OPERATION_DRAW_CREDITS_SHADOW = 4,
    OPERATION_DRAW_INDIVIDUAL = 5,
    OPERATION_DRAW_INDIVIDUAL_SHADOW = 6,
    OPERATION_SET_POSITIONS = 7,
    OPERATION_EVALUATE_YSIZE = 8,
    OPERATION_SET_POSITIONS_CREDITS = 9,
    OPERATION_EVALUATE_LINE_XSIZE = 10,
    OPERATION_EVALUATE_XSIZE = 11,
    OPERATION_EVALUATE_DIMENSIONS = 12,
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

#define RUNTIME_CHAR_COUNT 118

typedef struct RuntimeFont
{
    u8 charTexBuf[RUNTIME_CHAR_COUNT * FONT_CHAR_TEX_SIZE];
    u8 usedChar[RUNTIME_CHAR_COUNT];
    u8 storedChar[RUNTIME_CHAR_COUNT];
    u8 numUsed;
    u8 numStored;
    // 18 bytes of free space here
} RuntimeFont;

extern void DrawCharTexture(Gfx** gfxp, u8* texture, s32 x, s32 y, int scaleX, int scaleY, 
                            bool loadGfx,s16 alpha, Color_RGB8 Color, Color_RGB8 ShadowColor, bool drawShadow, 
                            s16 shadowAlpha, u8 shadowOffsetX, u8 shadowOffsetY, bool noWidescreenAdjust);
	asm("DrawCharTexture = 0x80078298");  

extern int TextOperation(PlayState* play, Font* font, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, 
                         s16 alpha, s16 shadowAlpha, char* msgData, int posX, int posY, u8 shadowOffsetX, 
                         u8 shadowOffsetY, Vec2s* positions, int scaleX, int scaleY, int lineMaxX, 
                         bool noWidescreenAdjust, int operation);
    asm("TextOperation = 0x800756F0 + 0x8"); 
    
extern void* Font_LoadRuntimeFontChar(Font* font, u8 character);
    asm("Font_LoadRuntimeFontChar = 0x800D6590"); 
    
extern void Font_LoadMessageBoxIcon(Font* font, u16 icon);
        asm("Font_LoadMessageBoxIcon = 0x8005BD34");     
    
__attribute__((always_inline)) inline int GetTextPxWidth(char* string, int scale)
{
    return TextOperation(NULL, NULL, NULL, COLOR_BLACK, COLOR_BLACK, 0, 0, string, 
                        0, 0, 0, 0, NULL, scale, scale, 0, true, OPERATION_EVALUATE_LINE_XSIZE);
} 

__attribute__((always_inline)) inline int GetTextPxWidthWidescreenAdjust(char* string, int scale)
{
    return TextOperation(NULL, NULL, NULL, COLOR_BLACK, COLOR_BLACK, 0, 0, string, 
                         0, 0, 0, 0, NULL, scale, scale, 0, false, OPERATION_EVALUATE_LINE_XSIZE);
} 
    
#ifdef GET_DIMENSIONS_FUNCS
    Vec2s GetTextDimensions(char* string, int scale, bool noWidescreenAdjust)
    {
        int res = TextOperation(NULL, NULL, NULL, COLOR_BLACK, COLOR_BLACK, 0, 0, string, 
                                0, 0, 0, 0, NULL, scale, scale, 0, noWidescreenAdjust, OPERATION_EVALUATE_DIMENSIONS);
        Vec2s ret;
           
        ret.x = res & 0xFFFF;
        ret.y = res >> 16;

        return ret;
    }
    
    int GetStringCenterX(char* string, float scale)
    {
        int len = GetTextPxWidth(string, scale);
        return (SCREEN_WIDTH / 2) - len / 2;
    }    
      
    int GetTextScaleToFitX(char* msg, int scaleMax, int maxXSize)
    {
        int dimX = GetTextPxWidth(msg, scaleMax);
        
        if (dimX <= maxXSize || dimX == 0)
            return scaleMax;
        
        // Calculate initial scaled value, capped at scaleMax
        int ret = MIN(scaleMax, (int)(scaleMax * (float)maxXSize / dimX));

        // Calculate the size at that scale
        int dimXNew = GetTextPxWidth(msg, ret);
        
        // Adjust downward if still too large
        while (dimXNew > maxXSize && ret > 0)
            dimXNew = GetTextPxWidth(msg, --ret);
        
        // Try to adjust upward if there's room
        while (ret < scaleMax) 
        {
            int nextDim = GetTextPxWidth(msg, ret + 1);
            if (nextDim <= maxXSize)
            {
                dimXNew = nextDim;
                ret++;
            }
            else
                break;
        }
        
        return ret;
    }
    
    int GetTextScaleToFitYFromHeight(int dimY, int scaleMax, int maxXSize)
    {
        return MIN(scaleMax, scaleMax * ((float)maxXSize / (float)dimY));
    }        

    int GetTextScaleToFitY(char* string, int scaleMax, int maxYSize)
    {
        int dimY = TextOperation(NULL, NULL, NULL, COLOR_BLACK, COLOR_BLACK, 0, 0, string, 
                                 0, 0, 0, 0, NULL, scaleMax, scaleMax, 0, false, OPERATION_EVALUATE_YSIZE);
                                 
        return GetTextScaleToFitYFromHeight(dimY, scaleMax, maxYSize);                                        
    }      
#endif

#endif