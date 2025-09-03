#include "../include/GameUI.h"

#define THIS ((UIStruct*)thisx)

void GameUI_Init(Actor* thisx, PlayState* play);
void GameUI_Destroy(Actor* thisx, PlayState* play);
void GameUI_Update(Actor* thisx, PlayState* play);
void GameUI_Draw(Actor* thisx, PlayState* play);

const ActorInitExplPad UI_InitVars =
{
    0xDEAD,
    ACTORCAT_PROP,
    0x00000030,
    3,
    0xBEEF,
    sizeof(UIStruct),
    (ActorFunc)GameUI_Init,
    (ActorFunc)GameUI_Destroy,
    (ActorFunc)GameUI_Update,
    (ActorFunc)GameUI_Draw,
};

void GameUI_Init(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;
    this->guiSpeaker = SPEAKER_NONE;
    this->speakerLastFrame = SPEAKER_NONE;
    this->curSpeakerTextAlpha = 0;
    this->curSpeakerTextboxAlpha = 0;
    this->guiSubtitle = 0;
    this->guiShowHearts = HEARTS_NONE;
    this->heartsPos = HEARTS_POS_DEFAULT;
    this->guiDrawIcon = -1;
    this->guiMsgLogCurrentScene = -1;
    this->msgLogPosition = 0;
    this->msgLogSpeed = 0.0f;
    this->CurScene = NULL;
    this->msgLogYSize = 0;
    this->guiCourtRecordStatus = UIACTOR_CR_IDLE;
    this->guiCourtRecordPlayerPresented = -1;
    this->guiCourtRecordMode = COURTRECORD_MODE_NORMAL;
    this->guiCourtRecordShowingList = LIST_EVIDENCE;
    this->crSelectedIdLast = -1;
    this->msgLogRingBufferPos = 0;
    this->guiCourtRecordListOnly = -1;
    this->guiSpeakerForce = -1;
    this->guiLogSubtitle = -1;
    this->forceMicShow = -1;
    this->guiTestimonySpeaker = -1;
    this->curSpeakerData = NULL;

    this->guiAlphaDir = DIR_NONE;
    this->guiAlpha = GUI_ALPHA_NONE;
    this->crAlpha = CR_ALPHA_NONE;
    
    this->fullscreenGraphicId = -1;
    this->fullscreenGraphicBuf = NULL;
    
    this->msgBufCR = ZeldaArena_Malloc(200);
    this->arrowGraphic = ZeldaArena_Malloc(FONT_CHAR_TEX_SIZE);
    
    // Load the arrow graphic.
    DmaMgr_SendRequest1(this->arrowGraphic, (uintptr_t)MESSAGE_STATIC_VROM + 4 * MESSAGE_STATIC_TEX_SIZE + TEXTBOX_ICON_ARROW * FONT_CHAR_TEX_SIZE, FONT_CHAR_TEX_SIZE);    
    
    // Load the UI graphics
    this->uiGraphics = LoadFromHeaderObject(OBJECT_UI, SAVE_LANGUAGE + BUTTONS_FILEID);
    
    if (this->uiGraphics == NULL)
        this->uiGraphics = LoadFromHeaderObject(OBJECT_UI, BUTTONS_FILEID);
    
    this->msgLog = ZeldaArena_Malloc(MSG_LOG_SIZE * sizeof(LoggedMsg));
    
#ifdef SAVE_MSGLOG    
    // Can't read less than 16 bytes, it seems.
    u8 buf[16];    
    u32 checksum = MSGLOG_MAGIC;
    u32 saved_checksum = 0;
    u32 msgLogPos;
    char* msglogd = (char*)this->msgLog;
    
    // Load message log from savefile
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT_MSGLOG), buf, 16, OS_READ);
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT_MSGLOG) + 16, this->msgLog, MSG_LOG_SIZE * sizeof(LoggedMsg), OS_READ);
    
    bcopy(&buf, &saved_checksum, 4);
    bcopy(&buf[4], &msgLogPos, 4);    
    
    // Calculate checksum
    checksum += msgLogPos;
    this->msgLogRingBufferPos = msgLogPos;
    
    for (int i = 0; i < MSG_LOG_SIZE * sizeof(LoggedMsg); i++)
        checksum += (int)(msglogd[i]);         
    
    // If msglog checksum does not match, then blank everything. 
    // Saved checksum being 0 means the message log has been invalidated.
    if (checksum != saved_checksum || saved_checksum == 0)
    {
        bzero(this->msgLog, MSG_LOG_SIZE * sizeof(LoggedMsg));
        this->msgLogRingBufferPos = 0;
        
        for (int i = 0; i < MSG_LOG_SIZE; i++)
        {
            this->msgLog[i].msgId = -1;
            this->msgLog[i].speakerId = -1;
        }
    }
#else
    bzero(this->msgLog, MSG_LOG_SIZE * sizeof(LoggedMsg));

    for (int i = 0; i < MSG_LOG_SIZE; i++)
    {
        this->msgLog[i].msgId = -1;
        this->msgLog[i].speakerId = -1;
    }
#endif
}

void GameUI_Destroy(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;
    
#ifdef SAVE_MSGLOG        
    
    // Store scene linebreak.
    if (this->msgLog[this->msgLogRingBufferPos].speakerId != SPEAKER_NOLOG && this->msgLog[this->msgLogRingBufferPos].speakerId != -1)
        StoreMessageInLog(thisx, play, sceneBreakMsg, strlen(sceneBreakMsg), 0xFFFF, SPEAKER_NOLOG);

    u32 checksum = MSGLOG_MAGIC;
    checksum += this->msgLogRingBufferPos;
    
    char* msglogd = (char*)this->msgLog;
    
    for (int i = 0; i < MSG_LOG_SIZE * sizeof(LoggedMsg); i++)
        checksum += (int)(msglogd[i]);    
    
    u32 header[4];
   
    header[0] = checksum;
    header[1] = this->msgLogRingBufferPos;
    header[2] = 0;
    header[3] = 0;
    
    // Write message log to save.
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT_MSGLOG), &header, 16, OS_WRITE);
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT_MSGLOG) + 16, this->msgLog, MSG_LOG_SIZE * sizeof(LoggedMsg), OS_WRITE);
    
#endif    
    
    ZeldaArena_Free(this->msgLog);
    ZeldaArena_Free(this->msgBufCR);
    ZeldaArena_Free(this->arrowGraphic);

    if (this->guiExplodeTextboxStatus != UIACTOR_EXPLOSION_IDLE)
    {
        this->guiExplodeTextboxStatus = UIACTOR_EXPLOSION_IDLE;

        if (this->textDrawPositions)
            ZeldaArena_Free(this->textDrawPositions);
        if (this->textMovementVectors)
            ZeldaArena_Free(this->textMovementVectors);
        if (this->msgBufDecodedCopy)
            ZeldaArena_Free(this->msgBufDecodedCopy);
    }
    
    if (this->fullscreenGraphicBuf)
        ZeldaArena_Free(this->fullscreenGraphicBuf);
    
    if (this->uiGraphics)
        ZeldaArena_Free(this->uiGraphics);
}

void GameUI_Update(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;
    
    if (this->VoiceM == NULL)
        this->VoiceM = (VoiceMgr*)GetActorByID(play, ACTOR_VRS);
    else if (this->VoiceM && this->VoiceM != (VoiceMgr*)0xDEADBEEF && this->VoiceM->actor.update == NULL)
        this->VoiceM = (VoiceMgr*)0xDEADBEEF;
    
    // Get Court Record Data NPCMaker address 
    if (this->CrDataNpcMaker == NULL)
        GetCRDataNpcMaker(thisx, play);
    else if (this->CrDataNpcMaker->actor.update == NULL)
    {
        this->CrDataNpcMaker = NULL;
        GetCRDataNpcMaker(thisx, play);
    }

    // Get speaker entry if it has changed since last frame.
    if (this->guiSpeaker != this->speakerLastFrame)
        this->curSpeakerData = GetSpeakerEntry(thisx, play, this->guiSpeaker);
    
    // Get testimony speaker
    if (this->guiTestimonySpeaker >= 0)
        this->curTestimonySpeakerData = GetSpeakerEntry(thisx, play, this->guiTestimonySpeaker);
    else
        this->curTestimonySpeakerData = NULL;
        
    // Create and destroy the data for message explosion
    if (this->guiExplodeTextboxStatus == UIACTOR_EXPLOSION_START)
    {
        this->textDrawPositions = ZeldaArena_Malloc(sizeof(Vec2s) * play->msgCtx.decodedTextLen);
        this->textMovementVectors = ZeldaArena_Malloc(sizeof(Vec2s) * play->msgCtx.decodedTextLen);
        this->msgBufDecodedCopy = ZeldaArena_Malloc(200);
        this->explodedTextboxLength = play->msgCtx.decodedTextLen;

        bcopy(&play->msgCtx.msgBufDecoded, this->msgBufDecodedCopy, 200);
        
        // Set first char to MESSAGE_PERSISTENT so that the message box doesn't disappear, but the text drawn by the normal game text drawing routine does.
        play->msgCtx.msgBufDecoded[0] = 0x0A;
        play->msgCtx.msgMode = MSGMODE_TEXT_CLOSING;

        GetMessageTextPositions(thisx, play, this->msgBufDecodedCopy, R_TEXT_INIT_XPOS, R_TEXT_INIT_YPOS);

        this->guiExplodeTextboxStatus = UIACTOR_EXPLOSION_ONGOING;
    }
    else if (this->guiExplodeTextboxStatus == UIACTOR_EXPLOSION_COMPLETE)
    {
        if (this->textDrawPositions)
            ZeldaArena_Free(this->textDrawPositions);
        if (this->textMovementVectors)
            ZeldaArena_Free(this->textMovementVectors);
        if (this->msgBufDecodedCopy)
            ZeldaArena_Free(this->msgBufDecodedCopy);

        this->guiExplodeTextboxStatus = UIACTOR_EXPLOSION_IDLE;
    }

    // Message Log
    TryLogMessage(thisx, play);

    if (this->guiMsgLogStatus == UIACTOR_MSGLOG_OPEN)
        MsgLogControls(thisx, play);
    else if (this->guiMsgLogStatus == UIACTOR_MSGLOG_OPENED)
    {
        play->envCtx.fillScreen = 1;
        play->envCtx.screenFillColor[0] = msgLogBackgroundColor.r;
        play->envCtx.screenFillColor[1] = msgLogBackgroundColor.g;
        play->envCtx.screenFillColor[2] = msgLogBackgroundColor.b;
        play->envCtx.screenFillColor[3] = msgLogBackgroundColor.a;

        this->msgLogPosition = 0;
        this->msgLogSpeed = 0.0f;
        this->msgLogYSize = CalculateMsgLogHeight(thisx, play);
        this->guiMsgLogStatus = UIACTOR_MSGLOG_OPEN;
    }

    // Update the picker hand position based on input.
    if (this->guiPickerEnabled)
    {
        u8 mouseSlot = 0;
        
        for (int i = 1; i < MAXCONTROLLERS; i++)
        {
            if (gPadMgr.padStatus[i].errno == 0 && gPadMgr.padStatus[i].type == CONT_TYPE_MOUSE)
            {
                mouseSlot = i;
                break;
            }
        }
        
        if (mouseSlot != 0) 
        {
            if (gPadMgr.padStatus[mouseSlot].type == CONT_TYPE_MOUSE) 
            {
                this->guiPickerPosX += gPadMgr.inputs[mouseSlot].cur.stick_x;
                this->guiPickerPosY -= gPadMgr.inputs[mouseSlot].cur.stick_y;      
            }
        }
      
        if (ABS(play->state.input->cur.stick_x) > STICK_DEADZONE)
        {
            this->guiPickerPosX += (play->state.input->cur.stick_x / STICK_PICKER_DIVIDER);
        }
        else
        {
            if (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DRIGHT))
                this->guiPickerPosX += DPAD_PICKER_SPEED;

            if (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DLEFT))
                this->guiPickerPosX -= DPAD_PICKER_SPEED;            
        }
        
        if (ABS(play->state.input->cur.stick_y) > STICK_DEADZONE)
        {
            this->guiPickerPosY -= (play->state.input->cur.stick_y / STICK_PICKER_DIVIDER);
        }
        else
        {
            if (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DUP))
                this->guiPickerPosY -= DPAD_PICKER_SPEED;

            if (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DDOWN))
                this->guiPickerPosY += DPAD_PICKER_SPEED;            
        }
        
        this->guiPickerPosX = MAX(MIN(this->guiPickerPosX, 320 - 32), 0);
        this->guiPickerPosY = MAX(MIN(this->guiPickerPosY, 240 - 32), 0);             
    }


    // Update speaker indicator Alpha
    switch (play->msgCtx.msgMode)
    {
        case MSGMODE_NONE:
        case MSGMODE_TEXT_START:
        case MSGMODE_TEXT_BOX_GROWING:
        case MSGMODE_TEXT_STARTING:
        case MSGMODE_TEXT_CLOSING:
            {
                Math_SmoothStepToS(&this->curSpeakerTextboxAlpha, SPEAKER_INDICATOR_ALPHA_NONE, 2, 100, 5);
                Math_SmoothStepToS(&this->curSpeakerTextAlpha, SPEAKER_INDICATOR_TEXT_ALPHA_NONE, 2, 100, 5);
                break;
            }
        default:
            {
                if (this->guiCourtRecordStatus != UIACTOR_CR_CHECKING)
                {
                    Math_SmoothStepToS(&this->curSpeakerTextboxAlpha, SPEAKER_INDICATOR_ALPHA_FULL, 1, 100, 5);
                    Math_SmoothStepToS(&this->curSpeakerTextAlpha, SPEAKER_INDICATOR_TEXT_ALPHA_FULL, 1, 100, 5);
                }
                break;
            }
    }

    if (this->guiSpeaker == SPEAKER_NONE)
    {
        this->curSpeakerTextboxAlpha = 0;
        this->curSpeakerTextAlpha = 0;
    }

    // Update UI Alpha

    if (this->guiToEnable != this->guiLastEffectiveGui)
        this->guiAlphaDir = DIR_OUT;

    switch (this->guiAlphaDir)
    {
        case DIR_NONE: break;
        case DIR_OUT:
        {
            Math_ApproachS(&this->guiAlpha, GUI_ALPHA_NONE, 1, GUI_ALPHA_DELTA);

            if (this->guiAlpha <= GUI_ALPHA_NONE)
            {
                this->guiAlpha = GUI_ALPHA_NONE;
                this->guiEffective = this->guiToEnable;
                this->guiAlphaDir = DIR_IN;
            }

            break;
        }
        case DIR_IN:
        {
            Math_ApproachS(&this->guiAlpha, GUI_ALPHA_FULL, 1, GUI_ALPHA_DELTA);
            
            if (this->guiAlpha >= GUI_ALPHA_FULL)
            {
                this->guiAlphaDir = DIR_NONE;
                this->guiAlpha = GUI_ALPHA_FULL;
            }

            break;
        }
    }
    
    int heartsEndPos = (SAVE_WIDESCREEN ? HEARTS_POS_Y : HEARTS_POS_SHOWING);

    // Update the hearts
    switch (this->guiShowHearts)
    {
        case HEARTS_IN:
        {
            Math_SmoothStepToS(&this->heartsPos, heartsEndPos, 4, HEARTS_MOVE_RATE, 4);

            if (this->heartsPos >= heartsEndPos)
            {
                this->heartsPos = heartsEndPos;
                this->guiShowHearts = HEARTS_SHOWING;
            }

            break;
        }
        case HEARTS_OUT:
        {
            Math_SmoothStepToS(&this->heartsPos, HEARTS_POS_DEFAULT, 4, HEARTS_MOVE_RATE, 4);

            if (this->heartsPos <= HEARTS_POS_DEFAULT)
            {
                this->heartsPos = HEARTS_POS_DEFAULT;
                this->guiShowHearts = HEARTS_NONE;
            }

            break;
        }
        case HEARTS_SHOWING:
        case HEARTS_DAMAGE:
        {
            this->heartsPos = heartsEndPos;
            break;
        }
        case HEARTS_NONE:
        {
            this->heartsPos = HEARTS_POS_DEFAULT;
            break;
        }
    }

    // the CR can change the gui, so this has to be here.
    this->guiLastEffectiveGui = this->guiToEnable;

    // Update Court Record
    UpdateCourtRecord(thisx, play);
    
    this->speakerLastFrame = this->guiSpeaker;
}

void GameUI_Draw(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;

    GraphicsContext* __gfxCtx = play->state.gfxCtx;
    Gfx* gfxRef = POLY_OPA_DISP;
    Gfx* gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx); 

    if (this->guiCourtRecordStatus != UIACTOR_CR_IDLE)
        DrawCourtRecord(thisx, play, &gfx);
    
    DrawUIElements(thisx, play, &gfx);
    
    if (this->CrDataNpcMaker != NULL && this->guiSpeaker != SPEAKER_NONE && this->curSpeakerData != NULL)
        DrawSpeakerIndicator(thisx, play, &gfx);

    if (this->guiSubtitle >= 3)
        DrawSubtitle(thisx, play, &gfx);

    if (this->guiExplodeTextboxStatus == UIACTOR_EXPLOSION_ONGOING)
    {
        bool exploding = TextOperation(play, NULL, &gfx, 
                                       explosionTextColor, colorBlack, 
                                       255, 255, 
                                       this->msgBufDecodedCopy, 
                                       0, 0, 0, 0, 
                                       this->textDrawPositions, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_INDIVIDUAL_SHADOW);
        
        if (!exploding)
            this->guiExplodeTextboxStatus = UIACTOR_EXPLOSION_COMPLETE;
        else
        {
            for (int i = 0; i < this->explodedTextboxLength; i++)
            {
                this->textDrawPositions[i].x += this->textMovementVectors[i].x;
                this->textDrawPositions[i].y -= this->textMovementVectors[i].y;
                this->textMovementVectors[i].y += EXPLOSION_CHAR_GRAVITY;
            }
        }
    }

    if (this->guiMsgLogStatus == UIACTOR_MSGLOG_OPEN)
        DrawMsgLog(thisx, play, &gfx);
   
    if (this->guiPickerEnabled)
        Draw2D(CI4, OBJ_GRAPHICS_COMMON, play, &gfx, this->guiPickerPosX + (POINTERHANDX / 2),  this->guiPickerPosY + (POINTERHANDY / 2), (u8*)POINTERHAND + 0x20, (u8*)POINTERHAND, POINTERHANDX, POINTERHANDY, this->guiAlpha);
    
    //=============================== Evidence Icon Window ===============================
    if (this->guiDrawIcon > EVIDENCE_INVALID)
        this->guiDrawIconEffective = this->guiDrawIcon;

    Math_ApproachS(&this->evidenceAlpha, this->guiDrawIcon < 0 ? EVIDENCE_ALPHA_NONE : EVIDENCE_ALPHA_FULL, 1, EVIDENCE_ALPHA_DELTA);

    if (this->evidenceAlpha > EVIDENCE_ALPHA_NONE)
    {
        Draw2D((SAVE_WIDESCREEN ? CI4_Setup39 : CI4), OBJ_GRAPHICS_COMMON, play, &gfx, EVIDENCE_FRAME_POSX - (SAVE_WIDESCREEN ? 30 : 0), EVIDENCE_FRAME_POSY, (u8*)EVIDENCE_FRAME_OFFSET + 0x20, (u8*)EVIDENCE_FRAME_OFFSET, EVIDENCE_FRAME_HEIGHT, EVIDENCE_FRAME_WIDTH, this->evidenceAlpha);
        void* offs = (void*)(this->guiDrawIconEffective * EVIDENCE_ICON_X * EVIDENCE_ICON_Y * 4);
        Draw2DScaled((SAVE_WIDESCREEN ? RGBA32_Setup39 : RGBA32), 7, play, &gfx, EVIDENCE_ICON_POSX + 16 - (SAVE_WIDESCREEN ? 30 : 0), EVIDENCE_ICON_POSY + 16, offs, NULL, 64, 64, 32, 32, this->evidenceAlpha);
    }
    //====================================================================================
    
    if (this->guiShowHearts)
    {
        int bankCurrent = thisx->objBankIndex;
        thisx->objBankIndex = Object_GetIndex(&play->objectCtx, 1);

        for (int i = 0; i < gSaveContext.healthCapacity; i++)
        {
            int XPos = (20 + (i * (HEARTS_WIDTH + 1)));
            
            if (SAVE_WIDESCREEN)
                XPos += HEARTS_POS_SHOWING - 50;
            else
                XPos += this->heartsPos;
            
            if (this->guiShowHearts == HEARTS_DAMAGE && i == gSaveContext.healthCapacity - 1)
            {
                Draw2D(CI8_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, XPos, (SAVE_WIDESCREEN ? this->heartsPos : HEARTS_POS_Y), (u8*)HEARTS_DAMAGE_OFFSET + 0x1F8 + (HEARTS_DAMAGE_SIZE * this->guiShowHeartsDamageFrame), (u8*)HEARTS_DAMAGE_OFFSET, HEARTS_WIDTH, HEARTS_HEIGHT, 255);

                this->guiShowHeartsDamageFrame++;

                if (this->guiShowHeartsDamageFrame > HEARTS_DAMAGE_FRAMES_COUNT - 1)
                {
                    gSaveContext.healthCapacity--;
                    this->guiShowHeartsDamageFrame = 0;
                    this->guiShowHearts = HEARTS_SHOWING;
                }
            }
            else
                Draw2D(CI8_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, XPos, (SAVE_WIDESCREEN ? this->heartsPos : HEARTS_POS_Y), (u8*)HEARTS_OFFSET + 0x78, (u8*)HEARTS_OFFSET, HEARTS_WIDTH, HEARTS_HEIGHT, 255);
        }

        thisx->objBankIndex = bankCurrent;
    }
    //====================================================================================
        

    if (this->guiEffective > 0 && 
        (this->guiEffective <= GUI_PRESENT || 
         this->guiEffective == GUI_CROSS_EXAMINATION))
    {
        if (this->VoiceM && this->VoiceM != (VoiceMgr*)0xDEADBEEF && !this->VoiceM->disabled && !this->VoiceM->needsReinit)
        {
            char* listeningMsg = NULL;
            this->VoiceM->turnedOff = true;
            
            if (this->guiEffective == GUI_CROSS_EXAMINATION)
                listeningMsg = this->VoiceM->region ? micMsgEng[VOICE_WORD_ID_MATTA] : micMsgJpn[VOICE_WORD_ID_MATTA];
            
            if (this->guiCourtRecordStatus == UIACTOR_CR_OPEN && this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT)
                listeningMsg = this->VoiceM->region ? 
                    micMsgEng[this->forcedPresent ? VOICE_WORD_ID_KURAE : VOICE_WORD_ID_IGIARI] : micMsgJpn[this->forcedPresent ? VOICE_WORD_ID_KURAE : VOICE_WORD_ID_IGIARI];
            
            if (this->forceMicShow >= VOICE_WORD_ID_IGIARI && this->forceMicShow < VOICE_WORD_ID_MAX)
                listeningMsg = this->VoiceM->region ? micMsgEng[this->forceMicShow] : micMsgJpn[this->forceMicShow];
            else
                this->forceMicShow = -1;
            
            if (listeningMsg != NULL)
            {
                this->VoiceM->turnedOff = false;
                
                char* msg = this->VoiceM->listening ? listeningMsg : micMsgs[LANG_INDEX];
                int scale = 60;
                int xpos = ROUNDBUTTON_R_POS_X + (SAVE_WIDESCREEN ? 50 : 0) + 12 + ((ROUNDBUTTON_XSIZE - GetTextPxWidth(msg, scale)) / 2) - ROUNDBUTTON_XSIZE / 2;
                
                TextOperation(play, NULL, &gfx, 
                              this->VoiceM->soundStatus == VOICE_STATUS_BUSY ? colorRed : colorWhite, colorBlack, 
                              255, 255, 
                              msg, 
                              xpos, this->forcedPresent ? MIC_POS_Y - 20 : MIC_POS_Y, 
                              0, 1, NULL, scale, scale, 0, false, OPERATION_DRAW_SHADOW);
            }
        }
    }     
    
    

/*
    char* s = "TESTINGTESTING";
    gDPPipeSync(gfx++);
    sprintf(s, "%x", (u8*)gfxRef + GRAPHICS_BUF_SIZE - (u8*)gfx);
    Gfx_SetupDL_39Ptr(&gfx);
    DrawMessageText(thisx, play, &gfx, (Color_RGB8){255,255,255}, 255, s, 20, 20);
*/

    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;    
}

Actor* GetActorByID(PlayState* playState, u16 ID)
{
    Actor* a = (Actor*)playState->actorCtx.actorLists[ACTORCAT_NPC].head;

    while (a)
    {
        if (a->id == ID)
            return a;
        
        a = a->next;
    }

    return NULL;
}

NpcMaker* GetNpcMakerByID(PlayState* playState, u16 ID)
{
    NpcMaker* npc = (NpcMaker*)playState->actorCtx.actorLists[ACTORCAT_NPC].head;
    bool found = false;

    while (npc)
    {
        if (npc->actor.id == NPCMAKER_ACTORID)
        {
            if (npc->npcId == ID)
            {
                found = true;
                break;
            }
        }

        npc = (NpcMaker*)npc->actor.next;
    }

    if (!found)
        return NULL;
    else
        return npc;
}

SpeakerEntry* GetSpeakerEntry(Actor* thisx, PlayState* play, int entry)
{
    UIStruct* this = THIS;
    
    if (entry < 0 || entry == SPEAKER_NONE)
        return NULL;
    else
    {
        if (this->CrDataNpcMaker == NULL || speakerData == NULL)
            return NULL;
        else
        {
            for (int i = 0; i < speakerNum; i++)
            {
                if (speakerData[i].id == entry)
                    return &speakerData[i]; 
            }            
        }    
    }
   
   return NULL;
}

void GetCRDataNpcMaker(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;
    
    this->CrDataNpcMaker = GetNpcMakerByID(play, NPCMAKER_COURT_RECORD_DATA);
    
    if (this->CrDataNpcMaker != NULL)
    {
        if (this->CrDataNpcMaker->scriptVars != NULL)
        {
            if (crMagic != COURT_RECORD_MAGIC && crMagicEnd != COURT_RECORD_MAGIC)
                this->CrDataNpcMaker = NULL;   
        }
        else
            this->CrDataNpcMaker = NULL;  
    }
}

void TryLogMessage(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;

    if (this->guiMsgLogCurrentScene < 0)
        return;

    if (this->CurScene == NULL || this->guiMsgLogCurrentScene != this->CurScene->npcId)
        this->CurScene = (NpcMaker*)GetNpcMakerByID(play, this->guiMsgLogCurrentScene);

    if (this->CurScene == NULL)
        return;
    
    if (Message_GetState(&play->msgCtx) == TEXT_STATE_CHOICE && Message_ShouldAdvance(play) && this->msgLog[this->msgLogRingBufferPos].msgId == this->CurScene->customMsgId)
        this->msgLog[this->msgLogRingBufferPos].msgChoice = play->msgCtx.choiceIndex + play->msgCtx.textboxEndType;

    // If the speaker is the same as last frame's
    // and speaker is one of the non-system ones
    // and logging isn't disabled
    // and message has been fully typed out...
    if (this->guiSpeaker == this->speakerLastFrame &&
        (this->guiSpeaker <= SPEAKER_TYPEWRITER || this->guiSpeaker == SPEAKER_NONE || this->guiSpeakerForce >= 0) &&
        !this->guiMsgLoggingDisabled &&
        play->msgCtx.textDrawPos >= (play->msgCtx.decodedTextLen - gTextSpeed) &&
        (play->msgCtx.msgMode == MSGMODE_TEXT_AWAIT_NEXT || play->msgCtx.msgMode == MSGMODE_TEXT_CLOSING || play->msgCtx.msgMode == MSGMODE_TEXT_DONE || this->guiSpeakerForce >= 0))
    {
        bool isNewMsg = false;

        // Check the first 16 characters for mis-matching.
        // This was the main basis for detecting a new message once upon a time, but it's probably not necessary now.
        for (int i = 0; i < MIN(16, play->msgCtx.decodedTextLen); i++)
        {
            if ((char)play->msgCtx.msgBufDecoded[i] != (char)this->msgLog[this->msgLogRingBufferPos].message[i])
                isNewMsg = true;
        }
        
        bool curNoSpeaker = false;
        
        if (this->curSpeakerData != NULL)
            curNoSpeaker = this->curSpeakerData->disableIndicator;
        
        int speaker = curNoSpeaker ? SPEAKER_NONE : this->guiSpeaker;

        // If the message is physically different,
        // or the message ID is different (excluding if the last stored message is a subtitle, since CurScene->customMsgId is always different then),
        // or the speaker ID is different...
        if (isNewMsg ||
           (this->msgLog[this->msgLogRingBufferPos].msgId != this->CurScene->customMsgId && this->CurScene->customMsgId >= 0 && this->msgLog[this->msgLogRingBufferPos].msgId != MSGLOG_SUBTITLE_MAGIC) ||
           (this->msgLog[this->msgLogRingBufferPos].speakerId != speaker))
        {
            StoreMessageInLog(thisx, play, (char*)play->msgCtx.msgBufDecoded, 200, this->CurScene->customMsgId, this->guiSpeakerForce >= 0 ? this->guiSpeakerForce : speaker);
            this->guiSpeakerForce = -1;
        }
    }
    
    // Store subtitle in log with magic ID of ABCDDCBA.
    if (this->guiLogSubtitle >= 0 && this->guiSubtitle)
    {
        if (this->CrDataNpcMaker != NULL)
        {
            void* msg = this->CrDataNpcMaker->GetInternalMsgPtrFunc(this->CrDataNpcMaker, play, this->guiSubtitle - 3 + msgSubtitlesIDStart);
        
            StoreMessageInLog(thisx, play, msg, MAX(200, strlen(msg)), MSGLOG_SUBTITLE_MAGIC, this->guiLogSubtitle);
            this->guiLogSubtitle = -1;
        }
    }
}

void StoreMessageInLog(Actor* thisx, PlayState* play, char* data, int size, int msgId, int speaker)
{
    UIStruct* this = THIS;    
    
    // Only increment if the first message was already used.
    if (this->msgLog[0].speakerId >= 0)
        this->msgLogRingBufferPos++;

    // Loop ring buffer around.
    if (this->msgLogRingBufferPos >= MSG_LOG_SIZE)
        this->msgLogRingBufferPos = 0;

    // Copy the message into the buffer
    bzero(&this->msgLog[this->msgLogRingBufferPos].message, 200);
    
    if (size > 200)
        size = 200;
    
    bcopy(data, &this->msgLog[this->msgLogRingBufferPos].message, size);

    this->msgLog[this->msgLogRingBufferPos].msgYSize = GetMessageTextYSize(thisx, play, this->msgLog[this->msgLogRingBufferPos].message);
    this->msgLog[this->msgLogRingBufferPos].speakerId = speaker;
    this->msgLog[this->msgLogRingBufferPos].msgId = msgId;
    this->msgLog[this->msgLogRingBufferPos].msgChoice = -1;
}

void GetMessageTextPositions(Actor* thisx, PlayState* play, char* msgData, int startPosX, int startPosY)
{
    UIStruct* this = THIS;
    TextOperation(play, NULL, NULL, 
                  colorBlack, colorBlack, 
                  0, 0, 
                  msgData, 
                  startPosX, startPosY, 
                  0, 0, 
                  this->textDrawPositions, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_SET_POSITIONS);

    for (int i = 0; i < play->msgCtx.decodedTextLen; i++)
        this->textMovementVectors[i] = (Vec2s){Rand_S16Offset(-5, 10), Rand_S16Offset(2, 10)};
}

int GetMessageTextYSize(Actor* thisx, PlayState* play, char* msgData)
{
    return TextOperation(play, NULL, NULL, 
                         colorBlack, colorBlack, 
                         0, 0, 
                         msgData, 
                         0, 0, 
                         0, 0, 
                         NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_EVALUATE_YSIZE);
}

void DrawSpeakerIndicator(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    UIStruct* this = THIS;
    Gfx* gfx = *gfxp;

    Gfx_SetupDL_39Ptr(&gfx);
    gDPPipeSync(gfx++);
    
    int textboxPosX = R_TEXTBOX_X + 8;
    int textboxPosY = (R_TEXTBOX_Y < 128) ? R_TEXTBOX_Y + 64 : R_TEXTBOX_Y - 18;    
    
    int textboxPosXActual = textboxPosX;

    if (SAVE_WIDESCREEN)
    {
        textboxPosXActual *= WIDESCREEN_SCALEX;
        textboxPosXActual += WIDESCREEN_OFFSX;    
    }    
    
    if (!this->curSpeakerData->disableIndicator)
    {
        if (this->speakerLastFrame == this->guiSpeaker)
        {        
            void* msg = this->CrDataNpcMaker->GetInternalMsgPtrFunc(this->CrDataNpcMaker, play, this->curSpeakerData->textID);

            switch (this->guiSpeaker)
            {
                case SPEAKER_CROSS_EXAM:
                    sprintf(this->speakerBuf, "%d / %d", this->guiCECurStatement, this->guiCEStatementCount); break;
                case SPEAKER_REBUTTAL:
                    sprintf(this->speakerBuf, "%s %d", msg, this->guiCECurStatement); break;
                default:
                    bcopy(msg, this->speakerBuf, MAX(SPEAKERBUF_SIZE, strlen(msg)));
            }
        }            
        
        int textboxHeight = 16;
        
        int textWidth = GetTextPxWidth(this->speakerBuf, TEXT_SCALE);
        int textboxWidth = MIN(SPEAKER_INDICATOR_MAX_XSIZE, MAX(SPEAKER_INDICATOR_MIN_XSIZE, textWidth));
        int textboxWidthActual = textboxWidth * (SAVE_WIDESCREEN ? WIDESCREEN_SCALEX : 1);
        
        int textboxDsDx = GET_DSD(R_TEXTBOX_WIDTH, textboxWidthActual);
        int textboxDsDy = GET_DSD(R_TEXTBOX_HEIGHT, textboxHeight); 
           
        Color_RGB8 txtBoxColor = this->curSpeakerData->textboxColor;
        gDPSetPrimColor(gfx++, 0, 0, txtBoxColor.r, txtBoxColor.g, txtBoxColor.b, this->curSpeakerTextboxAlpha);

        u8 textureType = G_IM_FMT_IA;

        if (play->msgCtx.textBoxType == TEXTBOX_TYPE_BLACK || play->msgCtx.textBoxType == TEXTBOX_TYPE_BLUE)
            textureType = G_IM_FMT_I;

        gDPLoadTextureBlock_4b(gfx++, play->msgCtx.textboxSegment, textureType, 128, 64, 0, G_TX_MIRROR, G_TX_MIRROR, 7, 0, G_TX_NOLOD, G_TX_NOLOD);
        gSPTextureRectangle(gfx++, textboxPosXActual << 2, textboxPosY << 2, (textboxPosXActual + textboxWidthActual) << 2, (textboxPosY + textboxHeight) << 2, G_TX_RENDERTILE, 0, 0, textboxDsDx, textboxDsDy);

        if (this->guiSpeaker == SPEAKER_CROSS_EXAM && this->guiDrawCheckmark)
            Draw2D(CI4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, textboxPosX + textboxWidth + 3, textboxPosY, (u8*)CHECKMARK_OFFSET + 0x20, (u8*)CHECKMARK_OFFSET, CHECKMARK_XSIZE, CHECKMARK_YSIZE, this->curSpeakerTextboxAlpha);            
        gDPPipeSync(gfx++);  
        int scaleX = GetTextScaleToFitX(this->speakerBuf, TEXT_SCALE, textboxWidth);
        
        if (SAVE_WIDESCREEN)
            scaleX *= WIDESCREEN_SCALEX;
       
        s32 textPosX = textboxPosXActual + ((textboxWidthActual - GetTextPxWidth(this->speakerBuf, scaleX)) / 2);
        s32 textPosY = textboxPosY + textboxHeight - 14;
       
        TextOperation(play, NULL, &gfx, 
                      this->curSpeakerData->textColor, this->curSpeakerData->textShadowColor, 
                      this->curSpeakerTextAlpha, this->curSpeakerTextAlpha, 
                      this->speakerBuf, 
                      textPosX, textPosY, 
                      0, 1, 
                      NULL, 
                      scaleX, TEXT_SCALE, 0,
                      true, OPERATION_DRAW_SHADOW);
        
    }
    
    textboxPosY = (R_TEXTBOX_Y < 128) ? (R_TEXTBOX_Y + 64 - 10) : (R_TEXTBOX_Y - 18 + 10);
    
    char* logMsg = logMsgs[LANG_INDEX];
    int logPosX = R_TEXTBOX_X + 256 - GetTextPxWidth(logMsg, TEXT_SCALE) - 8;
  
    s16 historyBtnAlpha = 0;
    
    // If forced present is enabled, then alpha is 0
    if (this->guiCourtRecordStatus == UIACTOR_CR_IDLE && !this->forcedPresent)
        historyBtnAlpha = MIN(this->guiAlpha, this->curSpeakerTextboxAlpha);

    if (this->guiEffective > 0 && (this->guiEffective <= GUI_PRESENT || this->guiEffective == GUI_CROSS_EXAMINATION))
    {
        TextOperation(play, NULL, &gfx, 
                      colorWhite, colorBlack, 
                      historyBtnAlpha, historyBtnAlpha, 
                      logMsg, 
                      logPosX, textboxPosY - 6, 
                      1, 1, 
                      NULL, TEXT_SCALE, TEXT_SCALE, 0, 
                      false, OPERATION_DRAW_SHADOW);            
        
    }  

    *gfxp = gfx;

}

int GetMaxEvidenceID(Actor* thisx)
{
    UIStruct* this = THIS;
    int ret = 0;

    for (int i = 0; i < evidenceNum; i++)
    {
        CourtRecordEntry crE = evidenceData[i];

        if (crE.list == this->guiCourtRecordShowingList && crE.showOnProgress <= evidenceProgress)
            ret++;
    }

    return ret;

}

void DrawCourtRecord(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    UIStruct* this = THIS;
    Gfx* gfx = *gfxp;

    gDPPipeSync(gfx++);
    
    // Draw a black rectangle so that models get properly covered up.
    if (this->crAlpha == CR_ALPHA_FULL)
    {
        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_FILL);
        gDPSetRenderMode(gfx++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gfx++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
        
        int xSz = CR_BASE_X;
        int xPos = CR_BASE_POSX;
        
        if (SAVE_WIDESCREEN)
        {
            xSz *= WIDESCREEN_SCALEX;
            xPos *= WIDESCREEN_SCALEX;
            xPos += WIDESCREEN_OFFSX;
        }
        
        gDPFillRectangle(gfx++, xPos - (xSz / 2), CR_BASE_POSY - (CR_BASE_Y / 2), xPos + (xSz / 2) - 1, CR_BASE_POSY + (CR_BASE_Y / 2) - 1);
    }    

    // The base graphic
    Draw2D(CI4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, CR_BASE_POSX, CR_BASE_POSY, (u8*)CR_BASE_OFFSET + 0x20, (u8*)CR_BASE_OFFSET, CR_BASE_X, CR_BASE_Y, this->crAlpha);

    int listPos = 0;
    this->selectedCREntry = evidenceData;

    // Calculates the first index of evidence to show in case we're showing another page
    int toSkip = this->crPickPage * CR_MAX_EVIDENCE_PER_PAGE;

    for (int i = 0; i < evidenceNum; i++)
    {
        CourtRecordEntry crE = evidenceData[i];

        // Only draw items belonging to the current shown list (evidence/profiles) and which have been obtained
        if (crE.list == this->guiCourtRecordShowingList && crE.showOnProgress <= evidenceProgress)
        {
            if (toSkip)
            {
                // Skip items until the first shown index
                toSkip--;
                continue;
            }

            int posX = CR_DARKSLOT_FIRSTPOSX + listPos * (CR_DARKSLOT_X + 2);

            // Draw highlighted selection slot
            if (listPos == this->crPickPos)
            {
                this->selectedCREntry = &evidenceData[i];
                Draw2D(CI4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, posX, CR_LIGHTSLOT_POSY, (u8*)CR_LIGHTSLOT_OFFSET + 0x10, (u8*)CR_LIGHTSLOT_OFFSET, CR_LIGHTSLOT_X, CR_LIGHTSLOT_Y, this->crAlpha);
            }

            // Draw evidence icon and then a border above it
            void* offs = (void*)(crE.id * EVIDENCE_ICON_X * EVIDENCE_ICON_Y * 4);
            Draw2DScaled(RGBA32, OBJ_GRAPHICS_ICONS, play, &gfx, posX, CR_DARKSLOT_POSY, offs, NULL, EVIDENCE_ICON_X, EVIDENCE_ICON_Y, 16, 16, this->crAlpha);
            Draw2D(IA4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, posX, CR_BORDER_FIRSTPOSY, (u8*)CR_BORDER_OFFSET, NULL, CR_BORDER_X, CR_BORDER_Y, this->crAlpha);

            listPos++;

            if (listPos == CR_MAX_EVIDENCE_PER_PAGE)
                break;
        }
    }

    // Draw dark frames and borders for the slots that aren't filled.
    for (int i = listPos; i < CR_MAX_EVIDENCE_PER_PAGE; i++)
    {
        int posX = CR_DARKSLOT_FIRSTPOSX + i * (CR_DARKSLOT_X + 2);
        Draw2D(CI4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, posX, CR_DARKSLOT_POSY + 1, (u8*)CR_DARKSLOT_OFFSET + 0x10, (u8*)CR_DARKSLOT_OFFSET, CR_DARKSLOT_X, CR_DARKSLOT_Y, this->crAlpha);
        Draw2D(IA4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, posX, CR_BORDER_FIRSTPOSY, (u8*)CR_BORDER_OFFSET, NULL, CR_BORDER_X, CR_BORDER_Y, this->crAlpha);
    }

    void* offs = (void*)(this->selectedCREntry->id * EVIDENCE_ICON_X * EVIDENCE_ICON_Y * 4);

    // Draw the big evidence icon and then a border above it
    Draw2DScaled(RGBA32, 7, play, &gfx, CR_BIG_BORDER_POSX, CR_BIG_BORDER_POSY, offs, NULL, EVIDENCE_ICON_X, EVIDENCE_ICON_Y, 64, 64, this->crAlpha);
    Draw2D(IA4_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, CR_BIG_BORDER_POSX, CR_BIG_BORDER_POSY, (u8*)CR_BIG_BORDER_OFFSET, NULL, CR_BIG_BORDER_X, CR_BIG_BORDER_Y, this->crAlpha);

    // Get the text
    if (this->CrDataNpcMaker == NULL)
        sprintf(this->msgBufCR, "DATA ACTOR NULL");
    else
    {
        if (this->crSelectedIdLast != this->selectedCREntry->id)
        {
            this->CrDataNpcMaker->GetInternalMsgFunc(this->CrDataNpcMaker, play, this->selectedCREntry->msgId, this->msgBufCR);
            this->crSelectedIdLast = this->selectedCREntry->id;
        }
    }
    
    TextOperation(play, NULL, &gfx,
                  colorWhite, colorBlack, 
                  this->crAlpha, this->crAlpha, 
                  this->msgBufCR, 
                  CR_TEXT_X, CR_TEXT_Y, 
                  1, 1, NULL, 
                  TEXT_SCALE, GetTextScaleToFitY(this->msgBufCR, TEXT_SCALE, CR_TEXT_MAX_YSIZE), CR_TEXT_MAX_XSIZE, 
                  false, OPERATION_DRAW_SHADOW);
                        
                        
    if (this->guiCourtRecordShowingList)
    {
        if (this->guiCourtRecordListOnly != LIST_PROFILES)
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_R_EVIDENCE_OFFSET, this->uiGraphics, &gfx, 
                            BARBUTTON_R_POS_X, BARBUTTON_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->crAlpha);                 
    }
    else
    {
        if (this->guiCourtRecordListOnly != LIST_EVIDENCE)
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_R_PROFILES_OFFSET, this->uiGraphics, &gfx, 
                            BARBUTTON_R_POS_X, BARBUTTON_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->crAlpha);                 
    }
    
    if (this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT)
        Draw2DInternal(CI8, this->uiGraphics + BUTTON_CUP_PRESENT_OFFSET, this->uiGraphics, &gfx, 
                        CUP_PRESENT_POS_X, CUP_PRESENT_POS_Y, CUP_PRESENT_XSIZE, CUP_PRESENT_YSIZE, CUP_PRESENT_XSIZE, CUP_PRESENT_YSIZE, this->crAlpha);            
         
    if (!this->forcedPresent)
        Draw2DInternal(CI8, this->uiGraphics + BUTTON_B_CLOSE_OFFSET, this->uiGraphics, &gfx, 
                       BARBUTTON_L_POS_X, BARBUTTON_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->crAlpha);
                                 

    *gfxp = gfx;
}

void DrawSubtitle(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    UIStruct* this = THIS;
    
    if (this->CrDataNpcMaker == NULL || this->guiSubtitle < 3)
        return;
    
    void* msg = this->CrDataNpcMaker->GetInternalMsgPtrFunc(this->CrDataNpcMaker, play, this->guiSubtitle - 3 + msgSubtitlesIDStart);

    s32 TEXT_POS_X = 0;
    s32 TEXT_POS_Y = 215;
    
    Gfx* gfx = *gfxp;
    Gfx_SetupDL_39Ptr(&gfx);
    gDPPipeSync(gfx++);
  
    TextOperation(play, NULL, &gfx, 
                  colorWhite, colorBlack, 
                  255, 255, 
                  msg, 
                  TEXT_POS_X, TEXT_POS_Y, 
                  0, 1, 
                  NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);

    *gfxp = gfx;
}

bool checkLoadFullScreenGraphic(Actor* thisx, int fileId, bool localized)
{
    UIStruct* this = THIS;
    
    int localizedFileId = fileId + (localized ? LANG_INDEX : 0);
    
    if (this->fullscreenGraphicBuf == NULL || this->fullscreenGraphicId != localizedFileId)
    {
        if (this->fullscreenGraphicBuf != NULL)
            ZeldaArena_Free(this->fullscreenGraphicBuf);
        
        this->fullscreenGraphicId = localizedFileId;
        this->fullscreenGraphicBuf = LoadFromHeaderObject(OBJECT_UI, localizedFileId);     

        // If the localized file ID is not present, load the english one (which should always be there!!)
        if (localized && this->fullscreenGraphicBuf == NULL)
            this->fullscreenGraphicBuf = LoadFromHeaderObject(OBJECT_UI, fileId); 
        
        return false;
    }
    else
        return true;
}

void DrawUIElements(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    UIStruct* this = THIS;
    Gfx* gfx = *gfxp;

    switch (this->guiEffective)
    {
        case GUI_NONE: break;
        case GUI_INVESTIGATION:
        {
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_R_COURTRECORD_OFFSET, this->uiGraphics, &gfx, 
                            ROUNDBUTTON_R_POS_X + (SAVE_WIDESCREEN ? 50 : 0), ROUNDBUTTON_R_POS_Y, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, this->guiAlpha);               
            break;
        }
        case GUI_PHOTO_EVIDENCE:
        {
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_B_CLOSE_IND_OFFSET, this->uiGraphics, &gfx, 
                           BARBUTTON_PHOTO_POS_X, BARBUTTON_PHOTO_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->guiAlpha);             
            FALLTHROUGH;
        }
        case GUI_PHOTO:
        {
            if (checkLoadFullScreenGraphic(thisx, MALON_PICT_FILEID, false))
            {
                // Drawn twice on purpose.
                Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->fullscreenGraphicBuf + 0x200, this->fullscreenGraphicBuf, &gfx, 
                               PHOTO_POS_X, PHOTO_POS_Y, PHOTO_XSIZE, PHOTO_YSIZE, PHOTO_XSIZE, PHOTO_YSIZE, this->guiAlpha);   
                Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->fullscreenGraphicBuf + 0x200, this->fullscreenGraphicBuf, &gfx, 
                               PHOTO_POS_X, PHOTO_POS_Y, PHOTO_XSIZE, PHOTO_YSIZE, PHOTO_XSIZE, PHOTO_YSIZE, this->guiAlpha);                   
            }                
            
            break;
        }
        case GUI_CROSS_EXAMINATION:
        {
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_R_COURTRECORD_OFFSET, this->uiGraphics, &gfx, 
                            ROUNDBUTTON_R_POS_X + (SAVE_WIDESCREEN ? 50 : 0), ROUNDBUTTON_R_POS_Y, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, this->guiAlpha);            
            
            Draw2DInternal(CI8, this->uiGraphics + BUTTON_L_PRESS_OFFSET, this->uiGraphics, &gfx, 
                            ROUNDBUTTON_L_POS_X - (SAVE_WIDESCREEN ? 50 : 0), ROUNDBUTTON_L_POS_Y, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, ROUNDBUTTON_XSIZE, ROUNDBUTTON_YSIZE, this->guiAlpha);
            
            
            if (this->guiShowConsult)
                Draw2DInternal(CI8, this->uiGraphics + BUTTON_CONSULT_OFFSET, this->uiGraphics, &gfx, 
                                CONSULT_BUTTON_POSX, CONSULT_BUTTON_POSY, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->guiAlpha);                

            break;
        }
        case GUI_CASE2PIC:
        {
            Environment_FillScreen(play->state.gfxCtx, 0, 0, 0, (s16)this->guiAlpha, FILL_SCREEN_XLU);
            Draw2D((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), OBJ_GRAPHICS_COMMON, play, &gfx, CASE2_PICPART1_XPOS, CASE2_PICPART1_YPOS, (u8*)CASE2_PICPART1 + 0x88, (u8*)CASE2_PICPART1, CASE2_PICPART1X, CASE2_PICPART1Y, this->guiAlpha);
            Draw2D((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), OBJ_GRAPHICS_COMMON, play, &gfx, CASE2_PICPART2_XPOS, CASE2_PICPART2_YPOS, (u8*)CASE2_PICPART2 + 0x88, (u8*)CASE2_PICPART2, CASE2_PICPART2X, CASE2_PICPART2Y, this->guiAlpha);
            Draw2D((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), OBJ_GRAPHICS_COMMON, play, &gfx, CASE2_PICPART3_XPOS, CASE2_PICPART3_YPOS, (u8*)CASE2_PICPART3 + 0x88, (u8*)CASE2_PICPART3, CASE2_PICPART3X, CASE2_PICPART3Y, this->guiAlpha);
            break;
        }
        case GUI_INVENTORY:
        case GUI_INVENTORY_EVIDENCE:
        {
            Environment_FillScreen(play->state.gfxCtx, 0, 0, 0, (s16)this->guiAlpha, FILL_SCREEN_XLU);
            
            if (checkLoadFullScreenGraphic(thisx, INVENTORY_FILEID, true))
            {            
                Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->fullscreenGraphicBuf + 0x200, this->fullscreenGraphicBuf, &gfx, 
                               SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, this->guiAlpha);

                if (this->guiEffective == GUI_INVENTORY_EVIDENCE)
                    Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->uiGraphics + BUTTON_B_CLOSE_IND_OFFSET, this->uiGraphics, &gfx, 
                                    BARBUTTON_INVENTORY_POS_X, BARBUTTON_INVENTORY_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->guiAlpha);   
            }                                
                                
            break;
        }
        case GUI_CRATES:
        {
            Environment_FillScreen(play->state.gfxCtx, 0, 0, 0, (s16)this->guiAlpha, FILL_SCREEN_XLU);
            
            if (checkLoadFullScreenGraphic(thisx, SHIPMENT_FILEID, false))
            {            
                Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->fullscreenGraphicBuf + 0x200, this->fullscreenGraphicBuf, &gfx, 
                               SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT, this->guiAlpha);

                if (this->guiEffective == GUI_INVENTORY_EVIDENCE)
                    Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->uiGraphics + BUTTON_B_CLOSE_IND_OFFSET, this->uiGraphics, &gfx, 
                                    BARBUTTON_INVENTORY_POS_X, BARBUTTON_INVENTORY_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->guiAlpha);   
            }              
            break;
        }
        case GUI_SHOPLIST:
        case GUI_SHOPLIST_EVIDENCE:
        {
            if (checkLoadFullScreenGraphic(thisx, TOOLSHOPLIST_FILEID, true))
            {            
                Draw2DInternal((SAVE_WIDESCREEN ? CI8_Setup39 : CI8), this->fullscreenGraphicBuf + 0x200, this->fullscreenGraphicBuf, &gfx, 
                               SHOP_LIST_POS_X, SHOP_LIST_POS_Y, SHOP_LIST_XSIZE, SHOP_LIST_YSIZE, SHOP_LIST_XSIZE, SHOP_LIST_YSIZE, this->guiAlpha);

                if (this->guiEffective == GUI_SHOPLIST_EVIDENCE)
                    Draw2DInternal(CI8, this->uiGraphics + BUTTON_B_CLOSE_IND_OFFSET, this->uiGraphics, &gfx, 
                                    BARBUTTON_INVENTORY_POS_X, BARBUTTON_INVENTORY_POS_Y, BARBUTTON_XSIZE, BARBUTTON_YSIZE, BARBUTTON_XSIZE, BARBUTTON_YSIZE, this->guiAlpha);    
            }
            
            break;
        }
    }

    int ARROW_R_XPOS = (this->guiEffective == GUI_CROSS_EXAMINATION ? ARROW_R_POS_X : ARROW_R_POS_X_COURTRECORD);
    int ARROW_L_XPOS = (this->guiEffective == GUI_CROSS_EXAMINATION ? ARROW_L_POS_X : ARROW_L_POS_X_COURTRECORD);
    int ARROW_YPOS = (this->guiEffective == GUI_CROSS_EXAMINATION ? ARROW_POS_Y : ARROW_POS_Y_COURTRECORD);

    f32 SinOffset = sinf(play->state.frames);
    
    s16 arrowAlpha = (this->guiEffective == GUI_CROSS_EXAMINATION ? this->guiAlpha : this->crAlpha);
    u8 wideScreenRArrowOffs = SAVE_WIDESCREEN ? 1 : 0;

    if (this->guiEffective >= GUI_COURTRECORD_EVIDENCE && this->guiEffective <= GUI_CROSS_EXAMINATION && this->guiAlphaDir != DIR_OUT)
    {
        switch (this->guiArrows)
        {
            case ARROWS_NONE: break;
            case ARROWS_LEFT:
            {
                Draw2D(CI4, OBJ_GRAPHICS_COMMON, play, &gfx, ARROW_L_XPOS - SinOffset, ARROW_YPOS, (u8*)LARROW_OFFSET + 0x20, (u8*)LARROW_OFFSET, ARROW_XSIZE, ARROW_YSIZE, arrowAlpha);
                break;
            }
            case ARROWS_RIGHT:
            {
                Draw2D(CI4, OBJ_GRAPHICS_COMMON, play, &gfx, ARROW_R_XPOS + SinOffset + wideScreenRArrowOffs, ARROW_YPOS, (u8*)RARROW_OFFSET + 0x20, (u8*)RARROW_OFFSET, ARROW_XSIZE, ARROW_YSIZE, arrowAlpha);
                break;
            }
            case ARROWS_BOTH:
            {
                Draw2D(CI4, OBJ_GRAPHICS_COMMON, play, &gfx, ARROW_L_XPOS - SinOffset, ARROW_YPOS, (u8*)LARROW_OFFSET + 0x20, (u8*)LARROW_OFFSET, ARROW_XSIZE, ARROW_YSIZE, arrowAlpha);
                Draw2D(CI4, OBJ_GRAPHICS_COMMON, play, &gfx, ARROW_R_XPOS + SinOffset + wideScreenRArrowOffs, ARROW_YPOS, (u8*)RARROW_OFFSET + 0x20, (u8*)RARROW_OFFSET, ARROW_XSIZE, ARROW_YSIZE, arrowAlpha);
                break;
            }
        }
    }

    *gfxp = gfx;
}

int GetFirstMsgInitialPos(Actor* thisx, PlayState* play)
{
    // Get starting position for the first text - this is so that the message you see after opening the msg log is in the same place
    // as in the textbox.
    // Note: the vanilla third y pos is "+ 16", but we patched it to "+ 14" to be more correct (see patch/code.txt)
    UIStruct* this = THIS;
    int InitialTexPosYs[] = {TEXTBOX_POS_Y_BOTTOM + 26, TEXTBOX_POS_Y_BOTTOM + 20, TEXTBOX_POS_Y_BOTTOM + 14, TEXTBOX_POS_Y_BOTTOM + 8};

    int numLinesIdx = (this->msgLog[this->msgLogRingBufferPos].msgYSize / R_TEXT_LINE_SPACING) - 1;

    return InitialTexPosYs[MIN(3, MAX(0, numLinesIdx))];
}

bool MsgLogShouldDisplaySpeaker(Actor* thisx, PlayState* play, int bufferPos)
{
    UIStruct* this = THIS;

    int f = bufferPos;

    int thisSpeaker = this->msgLog[f].speakerId;

    if (thisSpeaker == SPEAKER_NONE || thisSpeaker == SPEAKER_NOLOG)
        return false;

    // Also skip if it's the same as the previous message
    if (--f < 0)
        f = MSG_LOG_SIZE - 1;
    if (f == this->msgLogRingBufferPos)
        return true;

    return this->msgLog[f].speakerId != thisSpeaker;
}

int CalculateMsgLogHeight(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;

    int height = 0;

    int f = this->msgLogRingBufferPos;

    while (this->msgLog[f].speakerId >= 0)
    {
        height += this->msgLog[f].msgYSize + 2 * R_TEXT_LINE_SPACING;
        if (MsgLogShouldDisplaySpeaker(thisx, play, f))
            height += R_TEXT_LINE_SPACING;

        if (--f < 0)
            f = MSG_LOG_SIZE - 1;

        if (f == this->msgLogRingBufferPos)
            break;
    }

    return height;
}

void DrawMsgLogTriforce(PlayState* play, Gfx** gfxp, int yPos)
{
    Gfx* gfx = *gfxp;    
    
    Draw2DScaled(CI8_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, SCREEN_WIDTH / 2, yPos + 8, (u8*)MSGLOG_TRIFORCE_OFFSET + 0x130, (u8*)MSGLOG_TRIFORCE_OFFSET, MSGLOG_TRIFORCE_X, MSGLOG_TRIFORCE_Y, 18, 18, 255);
    TextOperation(play, NULL, &gfx, 
                  msgLogEndColor, colorBlack, 
                  255, 255, 
                  msgLogEnd, 
                  GetStringCenterX(msgLogEnd, TEXT_SCALE), 
                  yPos, 
                  0, 1, 
                  NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
    
    *gfxp = gfx;
}

void DrawMsgLog(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    UIStruct* this = THIS;
    Gfx* gfx = *gfxp;

    Gfx_SetupDL_39Ptr(&gfx);

    int TexPosX = R_TEXT_INIT_XPOS;
    int TexPosY = R_TEXT_INIT_YPOS;
    
    // Index to start drawing from
    int f = this->msgLogRingBufferPos;

    if (this->msgLog[f].speakerId >= 0)
    {
        // Get the Y size of the first textbox, so that it always fits on-screen.
        TexPosY = GetFirstMsgInitialPos(thisx, play);

        while(true)
        {
            bool displaySpeaker = MsgLogShouldDisplaySpeaker(thisx, play, f);
            
            if (TexPosY + this->msgLogPosition + this->msgLog[f].msgYSize + R_TEXT_LINE_SPACING >= 0 &&
                TexPosY + this->msgLogPosition - this->msgLog[f].msgYSize - R_TEXT_LINE_SPACING <= 240)
            {

                int TextPosXName = TexPosX - MSGLOG_SPEAKERNAME_OFFSET;
                int TextPosYName = TexPosY - R_TEXT_LINE_SPACING;
                
                if (displaySpeaker)
                {
                    SpeakerEntry* entry = GetSpeakerEntry(thisx, play, this->msgLog[f].speakerId);
                    int z = 0;

                    if (entry != NULL)
                    {
                        char* msg = this->CrDataNpcMaker->GetInternalMsgPtrFunc(this->CrDataNpcMaker, play, entry->textID);
                        static char nameBuffer[SPEAKERBUF_SIZE + 1];         

                        while (msg[z] != 0x0 && msg[z] != 0x2)
                        {
                            nameBuffer[z] = msg[z];
                            z++;
                        }

                        // Append a colon after the speaker name
                        nameBuffer[z] = ':';
                        nameBuffer[z + 1] = 0x0;

                        // Draw speaker name
                        TextOperation(play, NULL, &gfx, 
                                      msgLogSpeakerNameColor, colorBlack, 
                                      255, 255, 
                                      nameBuffer, 
                                      TextPosXName, 
                                      TextPosYName + this->msgLogPosition, 
                                      0, 1, 
                                      NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
                    }
                    else
                        displaySpeaker = false;
                }
                
                if (this->msgLog[f].speakerId == SPEAKER_NOLOG)
                    DrawMsgLogTriforce(play, &gfx, TexPosY + this->msgLogPosition);     
                else
                {
                    // Draw message text
                    TextOperation(play, NULL, &gfx, 
                                  colorWhite, colorBlack, 
                                  255, 255, 
                                  this->msgLog[f].message, 
                                  TexPosX, 
                                  TexPosY + this->msgLogPosition, 
                                  0, 1, 
                                  NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
                    
                    // Print arrow at selected message 
                    int msgChoice = this->msgLog[f].msgChoice;
                    
                    if (msgChoice != -1)
                    {
                        int index = msgChoice & 0xF;
                        int posOffset = (msgChoice >= TEXTBOX_ENDTYPE_3_CHOICE ? 0 : R_TEXT_LINE_SPACING) + (index * R_TEXT_LINE_SPACING) + R_TEXT_LINE_SPACING;
                        
                        DrawCharTexture(&gfx, 
                                        this->arrowGraphic, 
                                        TexPosX + (SAVE_WIDESCREEN ? 20 : 0), 
                                        TexPosY + this->msgLogPosition + posOffset, 
                                        TEXT_SCALE, 
                                        TEXT_SCALE, 
                                        true, 
                                        255, 
                                        msgLogPickedChoiceArrowColor, 
                                        colorBlack, 
                                        true, 
                                        255, 
                                        0, 
                                        1,
                                        false);
                    }
                }
            }
            
            if (displaySpeaker)
                TexPosY -= R_TEXT_LINE_SPACING;

            TexPosY -= R_TEXT_LINE_SPACING * 2;

            if (--f < 0)
                f = MSG_LOG_SIZE - 1;

            if (f == this->msgLogRingBufferPos || this->msgLog[f].speakerId < 0)
                break;

            TexPosY -= this->msgLog[f].msgYSize;
            TexPosX = R_TEXT_INIT_XPOS;
        }
    }
    
    int posYEnd = TexPosY - R_TEXT_LINE_SPACING + this->msgLogPosition;
    
    // Print end log message and icon
    if (posYEnd > -32 && posYEnd < 272)
    {
        DrawMsgLogTriforce(play, &gfx, posYEnd);
    }

    *gfxp = gfx;
}

void MsgLogControls(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;

    int Offset = GetFirstMsgInitialPos(thisx, play) - TEXTBOX_POS_Y_BOTTOM - 200;

    if (CHECK_BTN_ANY(play->state.input->press.button, BTN_START | BTN_B))
    {
        play->envCtx.fillScreen = 0;
        play->envCtx.screenFillColor[3] = 0;
        this->guiMsgLogStatus = UIACTOR_MSGLOG_IDLE;
        return;
    }

    float mag = sqrtf(SQ(play->state.input->cur.stick_x) + SQ(play->state.input->cur.stick_y));

    if (mag > STICK_DEADZONE)
    {
        if (mag > MSGLOG_STICK_OUTER_RING_MIN && ABS(play->state.input->cur.stick_y) < ABS(play->state.input->cur.stick_x))
            this->msgLogSpeed = (play->state.input->cur.stick_x < 0)
                ? MSGLOG_SCROLL_SPEED_DPAD_LR
                : -MSGLOG_SCROLL_SPEED_DPAD_LR;
        else
        {
            s8 stick_y = play->state.input->cur.stick_y;
            if (stick_y < -MSGLOG_STICK_MAX) stick_y = -MSGLOG_STICK_MAX;
            if (stick_y > MSGLOG_STICK_MAX) stick_y = MSGLOG_STICK_MAX;
            this->msgLogSpeed = stick_y * MSGLOG_SCROLL_SPEED_STICK;
        }
        this->msgLogAccel = __builtin_copysignf(MSGLOG_SCROLL_DECEL_STICK, -this->msgLogSpeed);
    }
    else
    {
        // goal here is to make it so that nothing happens if more than one dpad button is held
        u32 dpadButtons = play->state.input->cur.button & (BTN_CUP | BTN_CDOWN | BTN_CLEFT | BTN_CRIGHT | BTN_DUP | BTN_DDOWN | BTN_DLEFT | BTN_DRIGHT);
        if (dpadButtons == BTN_DUP || dpadButtons == BTN_CUP)
        {
            this->msgLogSpeed = MSGLOG_SCROLL_SPEED_DPAD_UD;
            this->msgLogAccel = -MSGLOG_SCROLL_DECEL_DPAD_UD;
        }
        else if (dpadButtons == BTN_DDOWN || dpadButtons == BTN_CDOWN)
        {
            this->msgLogSpeed = -MSGLOG_SCROLL_SPEED_DPAD_UD;
            this->msgLogAccel = MSGLOG_SCROLL_DECEL_DPAD_UD;
        }
        else if (dpadButtons == BTN_DLEFT || dpadButtons == BTN_CLEFT)
        {
            this->msgLogSpeed = MSGLOG_SCROLL_SPEED_DPAD_LR;
            this->msgLogAccel = -MSGLOG_SCROLL_DECEL_DPAD_LR;
        }
        else if (dpadButtons == BTN_DRIGHT || dpadButtons == BTN_CRIGHT)
        {
            this->msgLogSpeed = -MSGLOG_SCROLL_SPEED_DPAD_LR;
            this->msgLogAccel = MSGLOG_SCROLL_DECEL_DPAD_LR;
        }
    }

    this->msgLogPosition += this->msgLogSpeed;

    if (this->msgLogSpeed != 0.0f)
    {
        float newSpeed = this->msgLogSpeed + this->msgLogAccel;
        if ((bool)__builtin_signbitf(this->msgLogSpeed) == (bool)__builtin_signbitf(newSpeed))
            this->msgLogSpeed = newSpeed;
        else
            this->msgLogSpeed = 0.0f;
    }

    if (this->msgLogPosition >= this->msgLogYSize + Offset)
        this->msgLogPosition = this->msgLogYSize + Offset;

    if (this->msgLogPosition < 0)
        this->msgLogPosition = 0;

}

void UpdateCourtRecord(Actor* thisx, PlayState* play)
{
    UIStruct* this = THIS;

    bool scrollInputMade = false;

    if (this->crCooldown)
    {
        this->crCooldown--;
        this->firstInput = false;
    }
    else
        this->firstInput = true;
    
    if (ABS(play->state.input->cur.stick_y) > 20 || ABS(play->state.input->cur.stick_x) > 20 || CHECK_BTN_ANY(play->state.input->cur.button, BTN_DRIGHT | BTN_DLEFT | BTN_DUP | BTN_DDOWN))
        scrollInputMade = true;
    
    switch (this->guiCourtRecordStatus)
    {
        case UIACTOR_CR_OPENING:
        {
            bool onlyOneList = this->guiCourtRecordListOnly >= 0;
            this->guiArrows = ARROWS_NONE;

            if (this->crAlpha == CR_ALPHA_NONE)
            {
                if (this->CrDataNpcMaker == NULL || evidenceData == NULL)
                {
                    this->guiCourtRecordStatus = UIACTOR_CR_IDLE;
                    break;
                }
                else
                {
                    this->guiCourtRecordShowingList = onlyOneList ? this->guiCourtRecordListOnly : LIST_EVIDENCE;
                }
            }
            
            Math_ApproachS(&this->crAlpha, CR_ALPHA_FULL, 1, CR_ALPHA_DELTA);

            if (this->crAlpha == CR_ALPHA_FULL)
            {
                this->guiCourtRecordStatus = UIACTOR_CR_OPEN;
                this->crPickEvidenceMax = GetMaxEvidenceID(thisx);
                this->crPickPageMax = (this->crPickEvidenceMax + CR_MAX_EVIDENCE_PER_PAGE - 1) / CR_MAX_EVIDENCE_PER_PAGE;

                if (this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT)
                    this->guiToEnable = onlyOneList ? GUI_CROSS_EXAMINATION_EVIDENCE + this->guiCourtRecordListOnly : GUI_CROSS_EXAMINATION_EVIDENCE;
                else
                    this->guiToEnable = onlyOneList ? GUI_COURTRECORD_EVIDENCE + this->guiCourtRecordListOnly : GUI_COURTRECORD_EVIDENCE;
            }

            break;
        }
        case UIACTOR_CR_OPEN:
        {
            if (this->crPickEvidenceMax <= 1)
                this->guiArrows = ARROWS_NONE;
            else
                this->guiArrows = ARROWS_BOTH;


            if (CHECK_BTN_ALL(play->state.input->press.button, BTN_B) && !this->forcedPresent)
            {
                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CLOSE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                this->guiToEnable = GUI_NONE;
                this->guiArrows = ARROWS_NONE;                
                this->guiCourtRecordStatus = UIACTOR_CR_CLOSING;

                break;
            }

            if (CHECK_BTN_ALL(play->state.input->press.button, BTN_R) && this->guiCourtRecordListOnly < 0)
            {
                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_DECIDE_L, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                this->guiCourtRecordStatus = UIACTOR_CR_SWITCHING_OUT;
                break;
            }

            if (this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT)
            {
                bool CUPPressed = CHECK_BTN_ALL(play->state.input->press.button, BTN_CUP);
                
                if (!CUPPressed && this->VoiceM != NULL && this->VoiceM != (VoiceMgr*)0xDEADBEEF)
                {
                    if ((!this->forcedPresent && this->VoiceM->lastWord == VOICE_WORD_ID_IGIARI && this->VoiceM->detectionTimer > 0) || 
                        (this->forcedPresent && this->VoiceM->lastWord == VOICE_WORD_ID_KURAE && this->VoiceM->detectionTimer > 0))
                        CUPPressed = true;
                }
  
                if (CUPPressed)
                {
                    this->guiCourtRecordPlayerPresented = this->selectedCREntry->id;
                    this->guiToEnable = GUI_NONE;
                    this->guiArrows = ARROWS_NONE;
                    this->crAlpha = 0;
                    this->guiAlpha = 0;
                    this->guiCourtRecordStatus = UIACTOR_CR_IDLE;
                }
            }

            if (this->selectedCREntry->guiOnA)
            {
                if (CHECK_BTN_ALL(play->state.input->press.button, BTN_A))
                {
                    Audio_PlaySfxGeneral(NA_SE_SY_FSEL_DECIDE_L, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                    this->guiCourtRecordStatus = UIACTOR_CR_CHECKING_OPEN;
                    break;
                }
            }

            s8 prevCrPickPos = this->crPickPos;
            u8 prevCrPickPage = this->crPickPage;

            if (!this->crCooldown && (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DRIGHT) || play->state.input->cur.stick_x > 20))
            {
                this->crPickPos++;

                if (this->crPickPos + (this->crPickPage * CR_MAX_EVIDENCE_PER_PAGE) >= this->crPickEvidenceMax)
                {
                    this->crPickPage = 0;
                    this->crPickPos = 0;
                }
                else if (this->crPickPos > CR_MAX_EVIDENCE_PER_PAGE - 1)
                {
                    this->crPickPage++;
                    this->crPickPos = 0;
                }
            }
            else if (!this->crCooldown && (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DLEFT) || play->state.input->cur.stick_x < -20))
            {
                this->crPickPos--;

                if (this->crPickPos < 0)
                {
                    if (this->crPickPage > 0)
                        this->crPickPage--;
                    else
                        this->crPickPage = this->crPickPageMax - 1;

                    this->crPickPos = CR_MAX_EVIDENCE_PER_PAGE - 1;
                }
            }
            else if (!this->crCooldown && (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DUP) || play->state.input->cur.stick_y > 20))
            {
                this->crPickPos = CR_MAX_EVIDENCE_PER_PAGE - 1;

                if (this->crPickPage > 0)
                    this->crPickPage--;
                else
                    this->crPickPage = this->crPickPageMax - 1;
            }
            else if (!this->crCooldown && (CHECK_BTN_ALL(play->state.input->cur.button, BTN_DDOWN) || play->state.input->cur.stick_y < -20))
            {
                this->crPickPos = 0;

                this->crPickPage++;
                if (this->crPickPage >= this->crPickPageMax)
                    this->crPickPage = 0;
            }

            if (this->crPickPage == this->crPickPageMax - 1)
            {
                if (this->crPickPos > (this->crPickEvidenceMax - 1) % CR_MAX_EVIDENCE_PER_PAGE)
                    this->crPickPos = (this->crPickEvidenceMax - 1) % CR_MAX_EVIDENCE_PER_PAGE;
            }
            else
            {
                if (this->crPickPos > CR_MAX_EVIDENCE_PER_PAGE - 1)
                    this->crPickPos = CR_MAX_EVIDENCE_PER_PAGE - 1;
            }
            
            if (scrollInputMade)
            {
                if (this->crCooldown == 0)
                {
                    this->crCooldown = CR_STICK_COOLDOWN;
                    
                    if (this->firstInput)
                        this->crCooldown *= 3;
                }
            }
            else
            {
                this->firstInput = true;
                this->crCooldown = 0;
            }
            
            u16 sfxId = 0xffff;
            if (this->crPickPage < prevCrPickPage)
                sfxId = NA_SE_SY_WIN_SCROLL_LEFT;
            else if (this->crPickPage > prevCrPickPage)
                sfxId = NA_SE_SY_WIN_SCROLL_RIGHT;
            else if (this->crPickPos != prevCrPickPos)
                sfxId = NA_SE_SY_FSEL_CURSOR;

            if (sfxId != 0xffff)
                Audio_PlaySfxGeneral(sfxId, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);

            break;
        }
        case UIACTOR_CR_CHECKING_OPEN:
        {
            Math_ApproachS(&this->crAlpha, CR_ALPHA_NONE, 1, CR_ALPHA_DELTA);
            
            if (this->crAlpha == CR_ALPHA_NONE)
            {
                this->guiToEnable = this->selectedCREntry->guiOnA;
                this->guiArrows = ARROWS_NONE;
                this->guiCourtRecordStatus = UIACTOR_CR_CHECKING;
                this->wasShowingHearts = this->guiShowHearts;

                if (this->wasShowingHearts)
                    this->guiShowHearts = HEARTS_OUT;
            }

            break;
        }
        case UIACTOR_CR_CHECKING:
        {
            if (this->forcedPresent)
            {
                Math_ApproachS(&play->msgCtx.textboxColorAlphaCurrent, CR_ALPHA_NONE, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&play->msgCtx.textColorAlpha, CR_ALPHA_NONE, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&this->curSpeakerTextboxAlpha, SPEAKER_INDICATOR_ALPHA_NONE, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&this->curSpeakerTextAlpha, SPEAKER_INDICATOR_TEXT_ALPHA_NONE, 1, CR_ALPHA_DELTA);
            }
            
            if (CHECK_BTN_ALL(play->state.input->press.button, BTN_B))
            {
                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CLOSE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                this->guiToEnable = this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT ? GUI_CROSS_EXAMINATION_EVIDENCE : GUI_COURTRECORD_EVIDENCE;

                this->guiCourtRecordStatus = UIACTOR_CR_CHECKING_CLOSE;
            }

            break;
        }
        case UIACTOR_CR_CHECKING_CLOSE:
        {
            if (this->forcedPresent)
            {
                Math_ApproachS(&play->msgCtx.textboxColorAlphaCurrent, play->msgCtx.textboxColorAlphaTarget, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&play->msgCtx.textColorAlpha, CR_ALPHA_FULL, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&this->curSpeakerTextboxAlpha, SPEAKER_INDICATOR_ALPHA_FULL, 1, CR_ALPHA_DELTA);
                Math_ApproachS(&this->curSpeakerTextAlpha, SPEAKER_INDICATOR_TEXT_ALPHA_FULL, 1, CR_ALPHA_DELTA);
            }
            
            Math_ApproachS(&this->crAlpha, CR_ALPHA_FULL, 1, CR_ALPHA_DELTA);
            
            if (this->crAlpha == CR_ALPHA_FULL)
            {
                this->guiCourtRecordStatus = UIACTOR_CR_OPEN;

                if (this->wasShowingHearts)
                    this->guiShowHearts = HEARTS_IN;
            }

            break;
        }
        case UIACTOR_CR_SWITCHING_OUT:
        {
            Math_ApproachS(&this->crAlpha, CR_ALPHA_NONE, 1, CR_ALPHA_DELTA);

            if (this->crAlpha == CR_ALPHA_NONE)
            {
                this->guiCourtRecordShowingList = !this->guiCourtRecordShowingList;
                this->guiCourtRecordStatus = UIACTOR_CR_OPEN;
                this->crPickEvidenceMax = GetMaxEvidenceID(thisx);
                this->crPickPage = 0;
                this->crPickPos = 0;


                if (this->guiCourtRecordShowingList == LIST_EVIDENCE)
                    this->guiToEnable = this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT ? GUI_CROSS_EXAMINATION_EVIDENCE : GUI_COURTRECORD_EVIDENCE;
                else
                    this->guiToEnable = this->guiCourtRecordMode == COURTRECORD_MODE_PRESENT ? GUI_CROSS_EXAMINATION_PROFILES : GUI_COURTRECORD_PROFILES;


                this->guiCourtRecordStatus = UIACTOR_CR_SWITCHING_IN;
            }

            break;
        }
        case UIACTOR_CR_SWITCHING_IN:
        {
            Math_ApproachS(&this->crAlpha, CR_ALPHA_FULL, 1, CR_ALPHA_DELTA);
            
            if (this->crAlpha == CR_ALPHA_FULL)
                this->guiCourtRecordStatus = UIACTOR_CR_OPEN;

            break;
        }

        case UIACTOR_CR_CLOSING:
        {
            Math_ApproachS(&this->crAlpha, CR_ALPHA_NONE, 1, CR_ALPHA_DELTA);

            if (this->crAlpha == CR_ALPHA_NONE)
                this->guiCourtRecordStatus = UIACTOR_CR_IDLE;

            break;

        }
        default:
        {
            this->guiCourtRecordShowingList = LIST_EVIDENCE;
            this->crPickPage = 0;
            this->crPickPos = 0;
        }
    }
}