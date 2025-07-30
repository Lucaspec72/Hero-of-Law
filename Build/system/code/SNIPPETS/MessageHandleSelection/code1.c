#include "global.h"
#include "vt.h"
#include "../../../../actor/_custom-1.0/common.h"

// This eats into Message_DrawTextChar
// If injection space is moved, then Font_LoadRuntimeChar also has to be moved.
void Message_HandleChoiceSelection(PlayState* play, u8 numChoices) 
{
    static s16 sAnalogStickHeld = false;
    MessageContext* msgCtx = &play->msgCtx;
    Input* input = &play->state.input[0];

    if ((input->rel.stick_y >= 30 && !sAnalogStickHeld) || (CHECK_BTN_ALL(play->state.input->press.button, BTN_DUP)))
	{
        sAnalogStickHeld = true;
        msgCtx->choiceIndex--;
        if (msgCtx->choiceIndex > 128) {
            msgCtx->choiceIndex = 0;
        } else {
            Audio_PlaySfxGeneral(NA_SE_SY_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,&gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
    } 
	else if ((input->rel.stick_y <= -30 && !sAnalogStickHeld) || (CHECK_BTN_ALL(play->state.input->press.button, BTN_DDOWN)))
	{
        sAnalogStickHeld = true;
        msgCtx->choiceIndex++;
        if (msgCtx->choiceIndex > numChoices) {
            msgCtx->choiceIndex = numChoices;
        } else {
            Audio_PlaySfxGeneral(NA_SE_SY_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale,&gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
        }
    } else if (ABS(input->rel.stick_y) < 30) {
        sAnalogStickHeld = false;
    }
    
    // Sets the position of the picker arrow.
    // R_TEXT_CHOICE_YPOS(0) is the initial position of the arrow.
    
    msgCtx->textPosX = R_TEXT_CHOICE_XPOS + (2 * sinf(play->gameplayFrames)); 
    
    if (SAVE_WIDESCREEN)
    {
        msgCtx->textPosX *= WIDESCREEN_SCALEX;
        msgCtx->textPosX += WIDESCREEN_OFFSX;
    }
    
    msgCtx->textPosY = R_TEXTBOX_Y + 20 + (12 * msgCtx->choiceIndex) + ((numChoices == 1) ? 12 : 0);
}