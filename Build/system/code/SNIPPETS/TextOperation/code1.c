#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/1 - GameUI/include/structs.h"
#include "../../../../actor/_custom-1.0/holText.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"

#define TEXT_LINE_SPACING 16
#define TEXT_LINE_SPACING_CREDITS 6
#define JPN_TEXT_WIDTH 13

#define NOT_PAUSED 0
#define CURRENTLY_PAUSED 1
#define UNPAUSED_OR_ENDED 2  

#define NA_SE_NONE 0 
#define SOUND_HOL_EXPLOSION 0x6E3A

#define MESSAGE_NOP 0xFF
#define MESSAGE_SQUARE_TOGGLE 0xFE
#define MESSAGE_LINE_CENTER 0xFD
#define MESSAGE_FONT_TOGGLE 0xFC

/*
#define MESSAGE_TEXT_EFFECT 0xFB
#define TEXT_EFFECT_SCALEX 0x20
#define TEXT_EFFECT_SCALEY 0x21
#define TEXT_EFFECT_SCALE 0x22
#define TEXT_EFFECT_AUTOFIT 0x23
#define TEXT_EFFECT_SHAKE 0x24
*/

#define CHAR_NULL 0x0
#define CHAR_SPACE 0x20
#define CHAR_START_BUTTON 0xF8

#define gPreventTxtboxAdvanceAndSounds msgCtx->textboxBackgroundForeColorIdx
#define gTextboxPaused msgCtx->unk_E3F4

u8 GetCharWidth(u8 curChar, bool isDefault, bool widescreenAdjust);
Color_RGB8 GetTextColor(u8 textboxType, u8 colorIndex, Color_RGB8 defaultColor);
int GetNewScaleX(char* msg, int baseScaleX, int lineMaxX);

u8 lastSFXAt = 0;
u8 lastDIAt = 0;
u8 lastDCAt = 0;
u8 lastCommaAt = 0;
u8* useSquareEnd = (u8*)0x80112E40;
bool msgUnskippable = false;

static Vec3f pos = {0, 0, 1}; 

// 800756F0
void Interface_Draw(PlayState* play)
{
	return;
}

////800756F0 + 0x8
int HoL_DrawMessageTextImpl(PlayState* play, Font* font, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, 
                            s16 alpha, s16 shadowAlpha, char* msgData, 
                            int posX, int posY, u8 shadowOffsetX, u8 shadowOffsetY, 
                            Vec2s* positions, int scaleX, int scaleY, int lineMaxX, 
                            bool noWidescreenAdjust, int operation) 
{
    bool isTyper = (operation == OPERATION_TYPE_OUT);
    bool isAGraphicsOp = (operation <= OPERATION_DRAW_INDIVIDUAL_SHADOW);
    bool drawShadow = (operation == OPERATION_DRAW_SHADOW || 
                       operation == OPERATION_DRAW_CREDITS_SHADOW || 
                       operation == OPERATION_DRAW_INDIVIDUAL_SHADOW ||
                       operation == OPERATION_TYPE_OUT);    
    
    if (play == NULL && isTyper)
        return 0;
    
    MessageContext* msgCtx = NULL;
    UIStruct_Common* uiActor = NULL;
    Gfx* gfx = NULL;
    
    int i = 0;
    u8 curChar;
    
    int basePosX = posX;
    bool widescreenAdjust = SAVE_WIDESCREEN && !noWidescreenAdjust;

    if (widescreenAdjust)
    {
        posX *= WIDESCREEN_SCALEX;
        posX += WIDESCREEN_OFFSX; 
    }   
    
    int TexPosX = posX;
    int TexPosY = posY;    
    
    int baseScaleX = scaleX;
    int baseScaleY = scaleY;
    
    if (lineMaxX > 0)
        scaleX = GetNewScaleX(msgData, baseScaleX, lineMaxX); 
    
    int outYSize = TexPosY;
                    
    Color_RGB8 ActualColor = Color;                    
    u8 charLast = 0x20;
    u8 shiftText = 0;
    s8 iconDrawn = -1;
    u8 numLines = 0;
    //u8 shakeAmount = 0;
    int maxXSize = 0;
    
    bool defaultFont = false;
    bool disableSnd = false;
    bool disableSndManual = false;
    bool curCharSFXDisabled = false; 
    bool charWasDrawn = false;    
    
    if (play != NULL)
    {
        msgCtx = &play->msgCtx;
        
        if (font == NULL)
            font = &play->msgCtx.font;      
    }
    
    if (isAGraphicsOp) 
    {
        gfx = *gfxp;
        gDPPipeSync(gfx++);
        Gfx_SetupDL_39Ptr(&gfx);
    }
    
    // All of the setup for typing.
    if (isTyper)
    {
        // Failsafe
        if (gTextSpeed == 0)
            gTextSpeed = 1;
        
        // Message has begun. Messages always begin with textDrawPos being set to 1.
        if (msgCtx->textDrawPos == 1)
        {
            lastSFXAt = 0;
            lastDIAt = 0;
            lastDCAt = 0;
            lastCommaAt = 0;
            msgUnskippable = true;
            gTextboxPaused = NOT_PAUSED;
            msgCtx->textDrawPos = gTextSpeed;   // Set text draw pos to the current text speed
            msgCtx->textUnskippable = true;
        }     
        
        // Find the GUI Actor, to get access to the speaker data
        // ===============
        uiActor = (UIStruct_Common*)play->actorCtx.actorLists[ACTORCAT_PROP].head;

        while (uiActor && uiActor->actor.id != 1)
            uiActor = (UIStruct_Common*)uiActor->actor.next;     
        // ===============
       
        curChar = msgData[msgCtx->textDrawPos - 1];

        if (msgCtx->textDelayTimer == 0 && gTextboxPaused == CURRENTLY_PAUSED && 
            curChar != MESSAGE_QUICKTEXT_DISABLE && curChar != MESSAGE_TEXT_SPEED)
        {
            gTextboxPaused = UNPAUSED_OR_ENDED; 
        }
        
        // Skip textbox if C-RIGHT is pressed, and the message isn't marked as unskippable.
        if (!gPreventTxtboxAdvanceAndSounds && 
             CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_CRIGHT) && 
             !msgUnskippable)
        {
            lastCommaAt = 200;
            msgCtx->textDrawPos = 200;
        }   
        
        // gPreventTxtboxAdvanceAndSounds is used when unpausing the court record, so we set the whole message to be typed out instantly.
        if (gPreventTxtboxAdvanceAndSounds)
        {
            lastSFXAt = 200;
            lastDIAt = 200;
            lastDCAt = 200;
            lastCommaAt = 200;
            msgCtx->textDrawPos = 200;
            msgCtx->textDelayTimer = 0;
        }        
        
        *useSquareEnd = false;
        msgUnskippable = false;
    }

    for (i = 0; i < (isTyper ? MIN(200, msgCtx->textDrawPos) : 200); i++) 
    {
        curChar = (u8)msgData[i];
        curCharSFXDisabled = false;
        
        switch (curChar) 
        {
            // Unused tags
            case MESSAGE_OCARINA:
            case MESSAGE_EVENT:
            case MESSAGE_AWAIT_BUTTON_PRESS:
            case MESSAGE_NOP:
            {
                break;
            }     
            case MESSAGE_BACKGROUND:
            {
                i += 3;
                break;
            }             
            case MESSAGE_TEXTID:
            case MESSAGE_FADE2:
            {
                i += 2;
                break;
            }      
/*            
            case MESSAGE_TEXT_EFFECT:
            {
                u8 type = (u8)msgData[++i];
                u8 value = (u8)msgData[++i];
                
                switch (type)
                {
                    case TEXT_EFFECT_SCALE:
                    case TEXT_EFFECT_SCALEX: 
                    {
                        baseScaleX = value;
                        scaleX = value;
                        
                        if (type == TEXT_EFFECT_SCALEX)
                            break;
                    }
                    case TEXT_EFFECT_SCALEY: 
                    {
                        baseScaleY = value;
                        scaleY = value;
                        break;
                    }         
                    case TEXT_EFFECT_AUTOFIT:
                    {
                        if (value == 0)
                            value = 180;
                        
                        lineMaxX = value;
                        scaleX = GetNewScaleX(msgData, baseScaleX, lineMaxX); 
                        break;
                    }
                    case TEXT_EFFECT_SHAKE:
                    {
                        shakeAmount = value - 0x20;
                        break;
                    }
                }

                break;
            }
*/
            case MESSAGE_SQUARE_TOGGLE:
            {
                *useSquareEnd  = !*useSquareEnd;
                break;            
            }
            case MESSAGE_LINE_CENTER:
            { 
                int width = GetTextPxWidth(&msgData[i + 1], baseScaleX);  
               
                // In typer mode, adjust to middle of textbox, otherwise adjust to middle of screen.
                // (Which should be the same, unless the textbox is moved)
                if (isTyper)
                    TexPosX = R_TEXTBOX_X + ((R_TEXTBOX_WIDTH - width) / 2);
                else
                    TexPosX = (SCREEN_WIDTH / 2) - (width / 2);          
                
                if (widescreenAdjust)
                {
                    width *= WIDESCREEN_SCALEX;
                    
                    if (isTyper)
                    {
                        int textboxWidth = R_TEXTBOX_WIDTH * WIDESCREEN_SCALEX;
                        TexPosX = R_TEXTBOX_X + WIDESCREEN_TXBOX_OFFSX + ((textboxWidth - width) / 2); 
                    }
                    else                    
                        TexPosX = (SCREEN_WIDTH / 2) - (width / 2);        
                }
                
                break;
            }
            case MESSAGE_FONT_TOGGLE:
            {
                defaultFont = !defaultFont;
                break;
            }
            case CHAR_START_BUTTON:
            { 
                float sc = ((16 * scaleX) / 100);
                
                if (isAGraphicsOp) 
                {
                    Draw2DInternal(CI8_Setup39, &font->fontBuf[0x6C30], &font->fontBuf[0x6C00], &gfx, 
                                    basePosX + sc / 2, TexPosY + sc / 2, 24, 24, sc, sc, alpha);
                    
                    gDPPipeSync(gfx++);
                    Gfx_SetupDL_39Ptr(&gfx);
                }
                
                if (widescreenAdjust)
                {
                    sc *= WIDESCREEN_SCALEX;
                    sc += 2;
                }
                
                TexPosX += sc - 2;
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
                        TexPosY += ((TEXT_LINE_SPACING * scaleY) / 100);
                    else
                        TexPosY += TEXT_LINE_SPACING_CREDITS;
                    
                    if (lineMaxX > 0)
                        scaleX = GetNewScaleX(&msgData[i + 1], baseScaleX, lineMaxX);                               
                }
                   
                break;
            }            
            case MESSAGE_COLOR: 
            {
                u8 colorIdx = msgData[++i] & 0xF;
                // No sound for blue text (since it's used for thinking)
                disableSnd = (colorIdx == TEXT_COLOR_BLUE || colorIdx == TEXT_COLOR_LIGHTBLUE);
                
                if (isAGraphicsOp)
                    ActualColor = GetTextColor(isTyper ? msgCtx->textBoxType : TEXTBOX_TYPE_BLACK, colorIdx, Color);

                break;
            }
            case MESSAGE_BOX_BREAK:
            case MESSAGE_BOX_BREAK_DELAYED:
            {
                if (isTyper)
                {
                    if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                    {
                        gTextboxPaused = UNPAUSED_OR_ENDED;
                        
                        if (curChar == MESSAGE_BOX_BREAK_DELAYED)
                        {
                            msgCtx->stateTimer = msgData[++i];
                            msgCtx->msgMode = MSGMODE_TEXT_DELAYED_BREAK;                            
                        }
                        
                        msgCtx->msgMode = MSGMODE_TEXT_AWAIT_NEXT;
                        Font_LoadMessageBoxIcon(font, TEXTBOX_ICON_TRIANGLE);
                        charLast = 0;
                    }
                }
                
                i = 200;
                
                break;
            }
            case MESSAGE_SFX:
            {
                if (isTyper)
                {
                    u16 sfxHi = msgData[i + 1];
                    sfxHi <<= 8;
                    
                    int sndID = sfxHi | (u8)msgData[i + 2];

                    // If Sound ID is 0, toggle manual sound disabling.
                    if (sndID == NA_SE_NONE)
                        disableSndManual = !disableSndManual;
                    else
                    {
                        // Don't play sfx if disabled...
                        if (!gPreventTxtboxAdvanceAndSounds)
                        {
                            // ...Or if it has already been played.
                            if (i >= lastSFXAt)
                            {
                                Audio_PlaySfxGeneral(sndID, &pos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);     
                                lastSFXAt = i + 2;
                                
                                // If it's an EXPLOSION sfx, shake the screen too.
                                if (sndID == SOUND_HOL_EXPLOSION)
                                {
                                    s32 quakeIdx = Quake_Add(GET_ACTIVE_CAM(play), 1);
                                    Quake_SetSpeed(quakeIdx, 0x3A98);
                                    Quake_SetQuakeValues(quakeIdx, 0, 1, 0xFA, 1);
                                    Quake_SetCountdown(quakeIdx, 0xA);
                                    Rumble_Request(0, 255, 0xA, 150);
                                }                       
                            }                               
                        }
                        else
                            lastSFXAt = i + 2;
                    }
                }
                
                i += 2;
                break;
            }
            case MESSAGE_SHIFT:
            {
                u8 shift = (u8)msgData[++i];
                
                if (widescreenAdjust)
                    shift *= WIDESCREEN_SCALEX;
                
                TexPosX += shift;
                break;
            }
            case MESSAGE_QUICKTEXT_ENABLE:
            {
                if (isTyper)
                {
                    // This is way more complex than in the original code because it needs to account for multiple
                    // characters being printed per frame
                    if (i >= lastDIAt && (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING ||
                                         (msgCtx->msgMode >= MSGMODE_OCARINA_STARTING &&
                                          msgCtx->msgMode < MSGMODE_SCARECROW_LONG_RECORDING_START))) 
                    {
                                                    
                        int j = i;
                        lastDIAt = i + 1;
                        
                        // Walk through the message until we find a place to stop
                        while (true) 
                        {
                            u8 character = (u8)msgData[j];
                            
                            switch (character)
                            {
                                case MESSAGE_COLOR:
                                case MESSAGE_TEXT_SPEED:
                                case MESSAGE_ITEM_ICON:
                                case MESSAGE_SHIFT:                     j += 2; break;
                                case MESSAGE_TEXTID:
                                case MESSAGE_SFX:                       j += 3; break;
                                case MESSAGE_BACKGROUND:                j += 4; break;
                                case MESSAGE_QUICKTEXT_DISABLE:
                                case MESSAGE_PERSISTENT:
                                case MESSAGE_EVENT:
                                case MESSAGE_BOX_BREAK:
                                case MESSAGE_BOX_BREAK_DELAYED:
                                case MESSAGE_AWAIT_BUTTON_PRESS:
                                case MESSAGE_END:
                                case CHAR_NULL:
                                case MESSAGE_FADE:
                                case MESSAGE_FADE2:                     goto exit_loop;
                                default:                                j += 1; break;   
                            }
                        }
                        
                        exit_loop:
                        
                        i = j - 1;
                        lastCommaAt = i + 1;
                        msgCtx->textDrawPos = i + 1;
                        curCharSFXDisabled = true;
                        
                    }
                }
                break;  
            }                
            case MESSAGE_QUICKTEXT_DISABLE:
            {
                if (isTyper)
                {
                    curCharSFXDisabled = true;
                    
                    // If the previous character was also a QUICKTEXT_DISABLE, then we're pausing the textbox. 
                    if (msgCtx->textDrawPos >= lastDCAt)
                    {
                        if (msgCtx->textDrawPos != 0)
                        {
                            u8 character = (u8)msgData[msgCtx->textDrawPos - 1];
                            
                            if (character == MESSAGE_QUICKTEXT_DISABLE)
                                gTextboxPaused = CURRENTLY_PAUSED;
                        }
                    }
                    
                    lastDCAt = msgCtx->textDrawPos;
                }
                break;
            }
            case MESSAGE_ITEM_ICON: 
            {
                iconDrawn = (u8)msgData[i + 1];
                shiftText = 32;
                TexPosX += 32;
                i++;
                break;
            }
            case MESSAGE_TEXT_SPEED:
            {
                u8 speed = (u8)msgData[++i];
                
                if (isTyper)
                {
                    if (speed >= 101)
                        gTextSpeed = speed - 100;
                    else
                        msgCtx->textDelay = speed;
                }
                break;            
            }
            case MESSAGE_UNSKIPPABLE:
            {
                msgUnskippable = true;
                break;
            }          
            case MESSAGE_TWO_CHOICE:
            case MESSAGE_THREE_CHOICE: 
            {
                shiftText = 16;
                
                if (TexPosY != posY) 
                    TexPosX += 16;
                
                if (isTyper)
                {
                    if (msgCtx->textboxEndType != MESSAGE_THREE_CHOICE && msgCtx->textboxEndType != TEXTBOX_ENDTYPE_2_CHOICE)
                        Font_LoadMessageBoxIcon(font, TEXTBOX_ICON_ARROW);
                                        
                    msgCtx->textboxEndType = (curChar == MESSAGE_THREE_CHOICE ? TEXTBOX_ENDTYPE_3_CHOICE : TEXTBOX_ENDTYPE_2_CHOICE);
                    
                    if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                    {
                        msgCtx->choiceTextId = msgCtx->textId;
                        msgCtx->stateTimer = 4;
                        msgCtx->choiceIndex = 0;
                        charLast = 0;
                    }
                }
                
                break;
            }
            case CHAR_NULL:
            case MESSAGE_END:
            {
                if (isTyper)
                {
                    if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                    {                        
                        msgCtx->msgMode = MSGMODE_TEXT_DONE;  
                
                        if (msgCtx->textboxEndType == TEXTBOX_ENDTYPE_DEFAULT) 
                        {
                            Font_LoadMessageBoxIcon(font, *useSquareEnd ? TEXTBOX_ICON_SQUARE : TEXTBOX_ICON_TRIANGLE);
                            charLast = 0;
                        }
                        
                        gTextboxPaused = UNPAUSED_OR_ENDED;
                    }                 
                }

                i = 200;
                break;
            }
            case MESSAGE_FADE:
            {
                if (isTyper)
                {           
                    if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                    {
                        Font_LoadMessageBoxIcon(font, *useSquareEnd  ? TEXTBOX_ICON_SQUARE : TEXTBOX_ICON_TRIANGLE);
                        
                        msgCtx->msgMode = MSGMODE_TEXT_DONE;
                        msgCtx->textboxEndType = TEXTBOX_ENDTYPE_FADING;
                        msgCtx->stateTimer = (u8)msgData[++i];
                        
                        charLast = 0;
                        gTextboxPaused = UNPAUSED_OR_ENDED;
                    }
                }
                
                i = 200;
                break;
            }
            case MESSAGE_PERSISTENT:
            {
                if (isTyper)
                {
                    if (msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING) 
                    {
                        msgCtx->msgMode = MSGMODE_TEXT_DONE;
                        msgCtx->textboxEndType = TEXTBOX_ENDTYPE_PERSISTENT;
                        gTextboxPaused = UNPAUSED_OR_ENDED;
                    }
                }
                
                i = 200;
                break;
            }                
            default: 
            {
                // Walk back a pixel to fix the kerning of select characters following a ( bracket.         
                if (i != 0 && (u8)msgData[i - 1] == '(' && strchr("CGOQScdeotuvw~(@$^?<", curChar)) 
                    TexPosX -= 1;
                
                if (curChar != CHAR_SPACE) // Don't draw anything for spaces.
                {
                    int ActualX = TexPosX;
                    int ActualY = TexPosY;

                    // Re-adjust positions to the ones set in the positions array.
                    if (operation == OPERATION_DRAW_INDIVIDUAL || 
                        operation == OPERATION_DRAW_INDIVIDUAL_SHADOW) 
                    {
                        ActualX = positions[i].x;
                        ActualY = positions[i].y;
                    }

                    if (operation <= OPERATION_DRAW_INDIVIDUAL_SHADOW && font != NULL) 
                    {
                        // This line checks if the printed character is on-screen
                        // This isn't an optimization; it's how the "text explosion" effect knows it has ended.
                        // (No characters are marked as drawn once they all fall off screen, which is detected, and the game continues)
                        if (ActualX + CHARSIZEX >= 0 && ActualX - CHARSIZEX <= 320 && ActualY + CHARSIZEY >= 0 && ActualY - CHARSIZEY <= 240) 
                        {
                            gDPSetAlphaCompare(gfx++, G_AC_NONE);
                            gDPSetCombineLERP(gfx++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
                                            0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);
                                            
                            void* charGraphics = &font->fontBuf[(curChar - CHAR_SPACE) * FONT_CHAR_TEX_SIZE];

                            // Use the default font. Characters for this are preloaded on the go from the cartridge.
                            // This is necessary because other languages may sometimes have to overwrite the base font.
                            // Meant to be used for Koholink's name and the credits whenever they get localized.
                            if (defaultFont)
                                charGraphics = Font_LoadRuntimeFontChar(font, curChar);
                           
/*                           
                            u8 xShake = 0; 
                            u8 yShake = 0;
                            
                            if (shakeAmount)
                            {
                                xShake = Rand_S16Offset(0, shakeAmount);
                                yShake = Rand_S16Offset(0, shakeAmount);
                            }
*/                            
                            
                            DrawCharTexture(&gfx, charGraphics,
                                            ActualX, ActualY, scaleX, scaleY, charLast != curChar,
                                            alpha, ActualColor, ShadowColor, drawShadow, shadowAlpha,
                                            shadowOffsetX, shadowOffsetY, noWidescreenAdjust);

                            charWasDrawn = true;
                            charLast = curChar;
                            
                            if (isTyper)
                            {
                                // If the UI actor is there, and the sound isn't disabled in some manner, play a typer sound for every character.
                                if (uiActor != NULL && 
                                    !disableSnd && !disableSndManual && !curCharSFXDisabled && !gPreventTxtboxAdvanceAndSounds &&
                                    msgCtx->textboxEndType != TEXTBOX_ENDTYPE_2_CHOICE && 
                                    msgCtx->textboxEndType != TEXTBOX_ENDTYPE_3_CHOICE &&
                                    msgCtx->msgMode == MSGMODE_TEXT_DISPLAYING &&       // Only when the text is being typed out 
                                    msgCtx->textDelayTimer == msgCtx->textDelay &&      // Only when not waiting for delay
                                    msgCtx->textDrawPos == i + 1)                       // Only for the new character being typed out
                                    {
                                        // For rebuttals; overrides the SPEAKER_REBUTTAL typer sound with this one, so that the rebutted person can have their sounds.
                                        if (uiActor->curTestimonySpeakerData != NULL && (uiActor->guiSpeaker == SPEAKER_CROSS_EXAM || uiActor->guiSpeaker == SPEAKER_REBUTTAL))
                                        {
                                            Audio_PlaySfxGeneral(uiActor->curTestimonySpeakerData->sndID, &gSfxDefaultPos, 4, 
                                                                 &uiActor->curTestimonySpeakerData->sndPitch, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);  
                                        }
                                        // Else, if the data is available, play speaker sound.
                                        else if (uiActor->curSpeakerData != NULL)
                                        {
                                            Audio_PlaySfxGeneral(uiActor->curSpeakerData->sndID, &gSfxDefaultPos, 4, 
                                                                 &uiActor->curSpeakerData->sndPitch, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb); 
                                        }                                            
                                    }  

                                // Do a little pause at every comma.
                                if (i >= lastCommaAt && curChar == ',')
                                {
                                    lastCommaAt = i + 1;
                                    msgCtx->textDrawPos = lastCommaAt;
                                    msgCtx->textDelayTimer += 2;
                                }                                    
                                
                            }
                        }
                    } 
                    else if (operation == OPERATION_SET_POSITIONS || operation == OPERATION_SET_POSITIONS_CREDITS) 
                    {
                        positions[i] = (Vec2s){TexPosX, TexPosY};
                    } 
                }
                
                TexPosX += (s32)((GetCharWidth(curChar, defaultFont, widescreenAdjust) * scaleX) / 100);
            }
        }
    }
    
    if (isTyper)
    {
        if (msgCtx->textDelayTimer == 0) 
        {
            // Adjust text printing speed based on the framerate
            u8 sp = gTextSpeed + (R_UPDATE_RATE - 3);
            sp = sp < 1 ? 1 : sp; 
            msgCtx->textDrawPos = i + sp;
            msgCtx->textDelayTimer = msgCtx->textDelay;
        } 
        else 
        {
            msgCtx->textDelayTimer--;
        }        
        
    }

    // Handle icon drawing
    if (isAGraphicsOp) 
    {
        if (iconDrawn > 0 && play != NULL)
        {
            void* offs = (void*)(iconDrawn * 64 * 64 * 4);
            
            int iconPosX = widescreenAdjust ? posX - 16 : posX;
            int iconPosY = posY + 8 + (numLines * 6);
            
            if (isTyper)
            {
                iconPosX += R_TEXTBOX_ICON_XPOS + 16;
                iconPosY = R_TEXTBOX_ICON_YPOS + 16;
            }
            
            Draw2DScaled(RGBA32, 7, play, &gfx, iconPosX, iconPosY, offs, NULL, 64, 64, 32, 32, 255);
            Gfx_SetupDL_39Ptr(&gfx);
        }
        
        *gfxp = gfx;
    }

    // Return appropriate value based on operation
    switch (operation)
    {
        case OPERATION_EVALUATE_YSIZE:
            return TEXT_LINE_SPACING + TexPosY - outYSize;
        case OPERATION_EVALUATE_LINE_XSIZE:
            return TexPosX - posX;
        case OPERATION_EVALUATE_XSIZE:
            return maxXSize > TexPosX - posX ? maxXSize : TexPosX - posX;
        case OPERATION_EVALUATE_DIMENSIONS:
        {
            u16 x = maxXSize > TexPosX - posX ? maxXSize : TexPosX - posX;
            u16 y = TEXT_LINE_SPACING + TexPosY - outYSize;
            return (int)(x | y << 16);
        }
        case OPERATION_DRAW_INDIVIDUAL:
        case OPERATION_DRAW_INDIVIDUAL_SHADOW:
            return charWasDrawn;
        default:
            return 0;
    }
}

Color_RGB8 GetTextColor(u8 textboxType, u8 colorIndex, Color_RGB8 defaultColor) 
{
    switch (colorIndex) 
    {
        case TEXT_COLOR_RED:      
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_WOODEN:           return (Color_RGB8){255, 120, 0};
                default:                            return (Color_RGB8){255, 60, 60};
            }
            break;
        }
        case TEXT_COLOR_GREEN:                      return (Color_RGB8){70, 255, 80}; break;
        case TEXT_COLOR_BLUE:      
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_WOODEN:           return (Color_RGB8){80, 110, 255};
                default:                            return (Color_RGB8){80, 90, 255};
            }
            break;
        }             
        case TEXT_COLOR_LIGHTBLUE:      
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_WOODEN:           return (Color_RGB8){90, 180, 255};
                case TEXTBOX_TYPE_NONE_NO_SHADOW:   return (Color_RGB8){80, 150, 180};
                default:                            return (Color_RGB8){100, 180, 255};
            }
            break;
        }          
        case TEXT_COLOR_PURPLE:      
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_WOODEN:           return (Color_RGB8){210, 100, 255};
                default:                            return (Color_RGB8){255, 150, 180};
            }
            break;
        }            
        case TEXT_COLOR_YELLOW:      
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_WOODEN:           return (Color_RGB8){225, 225, 30};
                default:                            return (Color_RGB8){225, 225, 50};
            }
            break;
        }         
        case TEXT_COLOR_BLACK:                      return (Color_RGB8){0, 0, 0}; break;
        default:                  
        {
            switch (textboxType)
            {
                case TEXTBOX_TYPE_NONE_NO_SHADOW:   return (Color_RGB8){0, 0, 0};
                default:                            return defaultColor;
            }
            break;
        }
    }
}

int GetNewScaleX(char* msg, int baseScaleX, int lineMaxX)
{
    int dimX = GetTextPxWidth(msg, baseScaleX);
    
    if (dimX <= lineMaxX || dimX == 0)
        return baseScaleX;
    
    // Calculate initial scaled value, capped at baseScaleX
    int ret = MIN(baseScaleX, (int)(baseScaleX * (float)lineMaxX / dimX));

    // Calculate the size at that scale
    int dimXNew = GetTextPxWidth(msg, ret);
    
    // Adjust downward if still too large
    while (dimXNew > lineMaxX && ret > 0)
        dimXNew = GetTextPxWidth(msg, --ret);
    
    // Try to adjust upward if there's room
    while (ret < baseScaleX) 
    {
        int nextDim = GetTextPxWidth(msg, ret + 1);
        if (nextDim <= lineMaxX)
            ret++;
        else
            break;
    }
    
    return ret;
}

u8 GetCharWidth(u8 curChar, bool isDefault, bool widescreenAdjust)
{
    u8* fontWidths = isDefault ? fontWidthsDefault : fontWidthsLanguage;

    u8 charIdx = curChar - 0x20;
    u8 byte = fontWidths[charIdx / 2];
    
    u8 width = 13;
    
    if (charIdx & 1)
        width = byte & 0x0F;
    else
        width = (byte >> 4) & 0x0F;
    
    if (widescreenAdjust)
        width *= WIDESCREEN_SCALEX;
    
    return width;
}