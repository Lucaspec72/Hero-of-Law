#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"

// =============== UNUSED STUFF
s16** sTextboxBackgroundForePrimColors = (s16**)0x80112E40;  // used one byte for useSquareEnd
s16** sTextboxBackgroundBackPrimColors = (s16**)0x80112E70;
s16* sTextboxBackgroundYOffsets = (s16*)0x80112E88;
s16* sTextIsCredits = (s16*)0x8010A934;
u8* sTextboxSkipped = (u8*)0x8010A930;
s16* sMessageHasSetSfx = (s16*)0x8010A944;
//#define gTestimonySpeakerSoundIdx msgCtx->textboxBackgroundYOffsetIdx
// =============== UNUSED STUFF


//800D8EF4
void Message_DrawText(PlayState* play, Gfx** gfxP) 
{
    MessageContext* msgCtx = &play->msgCtx;
    TextOperation(play, NULL, gfxP, 
                  msgCtx->textBoxType == TEXTBOX_TYPE_NONE_NO_SHADOW ? COLOR_BLACK : COLOR_WHITE, COLOR_BLACK,
                  msgCtx->textColorAlpha, msgCtx->textColorAlpha, 
                  msgCtx->msgBufDecoded,
                  R_TEXT_INIT_XPOS, R_TEXT_INIT_YPOS, 
                  1, 1,
                  NULL, TEXT_SCALE, TEXT_SCALE, -1, false, OPERATION_TYPE_OUT);                       
}