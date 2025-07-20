#include "global.h"
#include "message_data_static.h"
#include "sfx.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"

#define FONT_CHAR_NUM 139
#define NES_FONT_STATIC 0x928000
#define CHARSIZEX 16
#define CHARSIZEY 16
#define TEXT_LINE_SPACING 16
#define TEXT_LINE_SPACING_CREDITS 6

static Color_RGB8 GetTextColor(u8 colorIndex, Color_RGB8 defaultColor);
f32* fontWidths = (f32*)0x80112F40;
u8 lastChar;

void InterfaceUpdate()
{
}

//800D7B00 + 0x8
int HoL_DrawMessageTextInternal(PlayState* play, u8* fontBuf, Gfx** gfxp, Color_RGB8 Color, 
                               Color_RGB8 ShadowColor, s16 alpha, s16 shadowAlpha, char* msgData, 
                               int posX, int posY, u8 shadowOffsetX, u8 shadowOffsetY, 
                               Vec2s* positions, float scale, int operation) 
{
    bool isAGraphicsOp = operation <= OPERATION_DRAW_INDIVIDUAL_SHADOW;
    Gfx* gfx = NULL;
    
    if (isAGraphicsOp) 
    {
        gfx = *gfxp;
        gDPPipeSync(gfx++);
        Gfx_SetupDL_39Ptr(&gfx);
        
        if (fontBuf == NULL && play != NULL)
            fontBuf = play->msgCtx.font.fontBuf;
    }
    
    int basePosX = posX;

    if (SAVE_WIDESCREEN)
    {
  
            posX *= WIDESCREEN_SCALEX;
            posX += WIDESCREEN_OFFSX; 
        
    }   
    
    int TexPosX = posX;
    int TexPosY = posY;    
    
    
    int outYSize = TexPosY;
    bool charWasDrawn = false;
    bool drawShadow = (operation == OPERATION_DRAW_SHADOW || 
                      operation == OPERATION_DRAW_CREDITS_SHADOW || 
                      operation == OPERATION_DRAW_INDIVIDUAL_SHADOW);
                      
    u8 charLast = 0x20;
    u8 shiftText = 0;
    s8 iconDrawn = -1;
    u8 numLines = 0;
    Color_RGB8 ActualColor = Color;
    int maxXSize = 0;

    for (int i = 0; i < 200; i++) 
    {
        u8 curChar = msgData[i];
        
        switch (curChar) 
        {
            case MESSAGE_BACKGROUND:
            case MESSAGE_QUICKTEXT_ENABLE:
            case MESSAGE_QUICKTEXT_DISABLE:
            case MESSAGE_AWAIT_BUTTON_PRESS:
            case 0xFF:
            case 0xFE:            
                break;           
            case MESSAGE_FADE:
            case MESSAGE_PERSISTENT:
            case MESSAGE_FADE2:
            case MESSAGE_EVENT:
            case MESSAGE_BOX_BREAK:
            case MESSAGE_BOX_BREAK_DELAYED:
            case MESSAGE_END:
            case 0x0:            
            {
                i = 200;
                break;
            }
            case 0xFD: // MESSAGE CENTER
            { 
                int width = GetTextPxWidth(&msgData[i + 1], scale);
                TexPosX = R_TEXTBOX_X + (R_TEXTBOX_WIDTH / 2) - (width / 2);
                
                if (SAVE_WIDESCREEN)
                {
                    int textboxWidth = R_TEXTBOX_WIDTH * 73 / 100;
                    width *= WIDESCREEN_SCALEX;
                    TexPosX = R_TEXTBOX_X + WIDESCREEN_TXBOX_OFFSX + (textboxWidth / 2) - (width / 2);                   
                }
                
                break;
            }
            case MESSAGE_TEXT_SPEED:
            {
                i++;
                break;
            }
            case MESSAGE_ITEM_ICON: 
            {
                iconDrawn = msgData[i + 1];
                shiftText = 32;
                TexPosX += 32;
                i++;
                break;
            }
            case MESSAGE_COLOR: 
            {
                ActualColor = GetTextColor(msgData[++i] & 0xF, Color);
                break;
            }
            case MESSAGE_SFX:
            {
                i += 2;
                break;
            }
            case MESSAGE_TWO_CHOICE:
            case MESSAGE_THREE_CHOICE: 
            {
                shiftText = 16;
                
                if (TexPosY != posY) 
                    TexPosX += 16;
                
                break;
            }
            case MESSAGE_SHIFT:
            {
                u8 shift = (u8)msgData[++i];
                
                if (SAVE_WIDESCREEN)
                    shift *= WIDESCREEN_SCALEX;
                
                TexPosX += shift;
                break;
            }
            case MESSAGE_NEWLINE: 
            {
                if (operation == OPERATION_EVALUATE_LINE_XSIZE) 
                    i = 200;
                else
                {
                    if (operation == OPERATION_EVALUATE_XSIZE || operation == OPERATION_EVALUATE_DIMENSIONS)
                    {
                        if (maxXSize < TexPosX - posX)
                            maxXSize = TexPosX - posX;
                    }                    
                                       
                    TexPosX = posX + shiftText;
                    numLines++;

                    if (operation != OPERATION_DRAW_CREDITS && 
                        operation != OPERATION_DRAW_CREDITS_SHADOW && 
                        operation != OPERATION_SET_POSITIONS_CREDITS)
                        TexPosY += TEXT_LINE_SPACING * (scale / 100.0f);
                    else
                        TexPosY += TEXT_LINE_SPACING_CREDITS;
                }
                   
                break;
            }
            case 0x20: // Space
            {
                float width = fontWidths[0];
                
                if (SAVE_WIDESCREEN)
                    width *= WIDESCREEN_SCALEX;

                TexPosX += (s32)(width * (scale / 100.0f));
                break;
            }
            case 0xF8: // START button
            { 
                float sc = 16 * (scale / 100.0f);
                
                if (isAGraphicsOp) 
                {
                    Draw2DInternal(CI8_Setup39, &fontBuf[0x6C30], &fontBuf[0x6C00], &gfx, 
                                    basePosX + sc / 2, TexPosY + sc / 2, 24, 24, sc, sc, alpha);
                    
                    gDPPipeSync(gfx++);
                    Gfx_SetupDL_39Ptr(&gfx);
                }
                
                if (SAVE_WIDESCREEN)
                {
                    sc *= WIDESCREEN_SCALEX;
                    sc += 2;
                }
                
                TexPosX += sc - 2;
                break;
            }
            default: 
            {
                // Walk back a pixel to fix the kerning of select characters following a ( bracket.
                // This is WMC's fault. Blame HIM.                 
                if (i != 0 && msgData[i - 1] == '(' && strchr("CGOQScdeotuvw~(@$^?<", curChar)) 
                    TexPosX -= 1;

                int ActualX = TexPosX;
                int ActualY = TexPosY;

                if (operation == OPERATION_DRAW_INDIVIDUAL || 
                    operation == OPERATION_DRAW_INDIVIDUAL_SHADOW) 
                {
                    ActualX = positions[i].x;
                    ActualY = positions[i].y;
                }

                if (operation <= OPERATION_DRAW_INDIVIDUAL_SHADOW) 
                {
                    if (ActualX + CHARSIZEX >= 0 && ActualX - CHARSIZEX <= 320 && ActualY + CHARSIZEY >= 0 && ActualY - CHARSIZEY <= 240) 
                    {
                        
                        gDPSetAlphaCompare(gfx++, G_AC_NONE);
                        gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
                                        0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

                        DrawCharTexture(NULL, &gfx, &fontBuf[(curChar - ' ') * FONT_CHAR_TEX_SIZE],
                                      ActualX, ActualY, scale, charLast != curChar,
                                      alpha, ActualColor, ShadowColor, drawShadow, shadowAlpha,
                                      shadowOffsetX, shadowOffsetY);

                        float width = (curChar > 0xAB) ? 13 : fontWidths[curChar - ' '];
                        
                        if (SAVE_WIDESCREEN)
                            width *= WIDESCREEN_SCALEX;                        
                        
                        TexPosX += (s32)(width * (scale / 100.0f));
                        charWasDrawn = true;
                        charLast = curChar;
                    }
                } 
                else if (operation == OPERATION_SET_POSITIONS || operation == OPERATION_SET_POSITIONS_CREDITS) 
                {
                    positions[i] = (Vec2s){TexPosX, TexPosY};
                    float width = (curChar > 0xAB) ? 13 : fontWidths[curChar - ' '];   

                        if (SAVE_WIDESCREEN)
                            width *= WIDESCREEN_SCALEX;                     
                    
                    TexPosX += (s32)(width * (scale / 100.0f));
                } 
                else if (operation == OPERATION_EVALUATE_LINE_XSIZE || operation == OPERATION_EVALUATE_XSIZE || operation == OPERATION_EVALUATE_DIMENSIONS) 
                {
                    float width = (curChar > 0xAB) ? 13 : fontWidths[curChar - ' ']; 
                    TexPosX += (s32)(width * (scale / 100.0f));
                }
            }
        }
    }

    // Handle icon drawing
    if (isAGraphicsOp && iconDrawn > 0 && play != NULL) 
    {
        void* offs = (void*)(iconDrawn * 64 * 64 * 4);
        
        int pos = SAVE_WIDESCREEN ? (posX + 8 - 16) : (posX + 8);
        
        Draw2DScaled(RGBA32, 7, play, &gfx, pos, posY + 8 + (numLines * 6), offs, NULL, 64, 64, 32, 32, 255);
        Gfx_SetupDL_39Ptr(&gfx);
    }

    if (isAGraphicsOp)
        *gfxp = gfx;

    // Return appropriate value based on operation
    switch (operation)
    {
        case OPERATION_EVALUATE_YSIZE:
            return R_TEXT_LINE_SPACING + TexPosY - outYSize;
        case OPERATION_EVALUATE_LINE_XSIZE:
            return TexPosX - posX;
        case OPERATION_EVALUATE_XSIZE:
            return maxXSize > TexPosX - posX ? maxXSize : TexPosX - posX;
        case OPERATION_EVALUATE_DIMENSIONS:
        {
            u16 x = maxXSize > TexPosX - posX ? maxXSize : TexPosX - posX;
            u16 y = R_TEXT_LINE_SPACING + TexPosY - outYSize;
            
            return (int)(x | y << 16);
        }
        case OPERATION_DRAW_INDIVIDUAL:
        case OPERATION_DRAW_INDIVIDUAL_SHADOW:
            return charWasDrawn;
        default:
            return 0;
    }
}

static Color_RGB8 GetTextColor(u8 colorIndex, Color_RGB8 defaultColor) 
{
    switch (colorIndex) 
    {
        case TEXT_COLOR_RED:      return (Color_RGB8){255, 60, 60};
        case TEXT_COLOR_GREEN:    return (Color_RGB8){70, 255, 80};
        case TEXT_COLOR_BLUE:     return (Color_RGB8){80, 90, 255};
        case TEXT_COLOR_LIGHTBLUE:return (Color_RGB8){100, 180, 255};
        case TEXT_COLOR_PURPLE:   return (Color_RGB8){100, 150, 180};
        case TEXT_COLOR_YELLOW:   return (Color_RGB8){225, 225, 50};
        case TEXT_COLOR_BLACK:    return (Color_RGB8){0, 0, 0};
        default:                  return defaultColor;
    }
}