#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"
#include "../../../../object/1 - gameplay_keep/zobj.h"

extern Input* D_8012D1F8;
void Play_Update(PlayState* this);
void Play_Draw(PlayState* this);
void DebugDisplay_Init(void);
void DrawSavingIcon(PlayState* play);

#define SAVE_ICON_POSX SCREEN_WIDTH - 24
#define SAVE_ICON_POSY SCREEN_HEIGHT - 16
#define SAVE_ICON_X 32
#define SAVE_ICON_Y 32

#define DOTS_POS_X_INITIAL SAVE_ICON_POSX + 12
#define DOTS_POS_Y_INITIAL SAVE_ICON_POSY - 4

void Play_Main2(PlayState* play)
{
    D_8012D1F8 = &play->state.input[0];

    RuntimeFont* f = (RuntimeFont*)&play->msgCtx.font.charTexBuf;
    
    if (f->numUsed != 0 || f->usedChar[0] != 0xFF)
    {
        Lib_MemSet(&f->usedChar[0], RUNTIME_CHAR_COUNT, 0xFF);
        f->numUsed = 0;
    }

    DebugDisplay_Init();
    Play_Update(play);
    Play_Draw(play);
    
    DrawSavingIcon(play);
        
    Screen_Adjust(&play->state, &play->view);
    return;
}

void DrawSavingIcon(PlayState* play)
{
    if (*savingGfxCounter)
    {
        GraphicsContext* __gfxCtx = play->state.gfxCtx;
        Gfx* gfx;
        Gfx* gfxRef;
        gfxRef = POLY_OPA_DISP;
        gfx = Graph_GfxPlusOne(gfxRef);
        gSPDisplayList(OVERLAY_DISP++, gfx);        
        
        int curFrame = SAVE_INDICATOR_LENGTH - *savingGfxCounter;
        
        int alpha = (curFrame <= 0) ? 0 :
                    (curFrame < SAVE_INDICATOR_LENGTH / 5) ? (curFrame * 255) / (SAVE_INDICATOR_LENGTH / 5) :
                    (curFrame <= SAVE_INDICATOR_LENGTH - 5) ? 255 :
                    (curFrame < SAVE_INDICATOR_LENGTH) ? 255 - ((curFrame - (SAVE_INDICATOR_LENGTH - 5)) * 255) / 5 : 0;
        
        (*savingGfxCounter)--;
        
        u8* offset = (u8*)OFFSET_SAVE_ICON;
        
        Draw2DScaled(CI8_Setup39, OBJECT_GAMEPLAY_KEEP, play, &gfx,    
                    SAVE_ICON_POSX + (SAVE_WIDESCREEN ? 50 : 0), SAVE_ICON_POSY,
                    offset + 0x110 + (0x400 * (curFrame % 19)), offset,                      
                    SAVE_ICON_X, SAVE_ICON_Y, 18, 18, alpha);
                        
        gSPEndDisplayList(gfx++);
        Graph_BranchDlist(gfxRef, gfx);
        POLY_OPA_DISP = gfx;    
    }  
}