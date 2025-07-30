#include "global.h"
#include "message_data_static.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"

void Message_DrawTextBox(PlayState* play, Gfx** p) 
{
    MessageContext* msgCtx = &play->msgCtx;
    Gfx* gfx = *p;

    gDPPipeSync(gfx++);
    gDPSetPrimColor(gfx++, 0, 0, msgCtx->textboxColorRed, msgCtx->textboxColorGreen, msgCtx->textboxColorBlue,
                    msgCtx->textboxColorAlphaCurrent);
                    
    int textboxWidth = R_TEXTBOX_WIDTH;
    int textboxPosX = R_TEXTBOX_X;
    
    if (SAVE_WIDESCREEN)
    {
        textboxWidth *= WIDESCREEN_SCALEX;
        textboxPosX += WIDESCREEN_TXBOX_OFFSX;        
    }
    
    int textboxDsDx = GET_DSD(R_TEXTBOX_WIDTH, textboxWidth);     
    int textboxDsDy = GET_DSD(R_TEXTBOX_HEIGHT, R_TEXTBOX_HEIGHT);      

    if (!(msgCtx->textBoxType) || msgCtx->textBoxType == TEXTBOX_TYPE_BLUE)
    {
        gDPLoadTextureBlock_4b(gfx++, msgCtx->textboxSegment, G_IM_FMT_I, 128, 64, 0, G_TX_MIRROR, G_TX_NOMIRROR, 7, 0,
                               G_TX_NOLOD, G_TX_NOLOD);
    } 
    else 
    {
        gDPSetEnvColor(gfx++, 50, 20, 0, 255);
        gDPLoadTextureBlock_4b(gfx++, msgCtx->textboxSegment, G_IM_FMT_IA, 128, 64, 0, G_TX_MIRROR, G_TX_MIRROR, 7, 0,
                               G_TX_NOLOD, G_TX_NOLOD);
    }

    gSPTextureRectangle(gfx++, textboxPosX << 2, R_TEXTBOX_Y << 2, (textboxPosX + textboxWidth) << 2,
                        (R_TEXTBOX_Y + R_TEXTBOX_HEIGHT) << 2, G_TX_RENDERTILE, 0, 0, textboxDsDx, textboxDsDy);

    *p = gfx;
}

