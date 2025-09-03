#include "../include/voice.h"
#include "../include/voicemgr.h"
#include "../../is64Printf.h"

#define THIS ((VoiceMgr*)thisx)

void VoiceMgr_Init(Actor* thisx, PlayState* play);
void VoiceMgr_Destroy(Actor* thisx, PlayState* play);
void VoiceMgr_Update(Actor* thisx, PlayState* play);
void VoiceMgr_Draw(Actor* thisx, PlayState* play);

u8 __osContLastPoll = 0;
u8 __osMaxControllers = 4;
s32 __osPfsLastChannel = -1;

OSVoiceContext sVoiceContext;
OSVoiceHandle gVoiceHandle;
OSVoiceData D_801FD5C8; // Intermediate Voice Data during processsing?
s32 sVoiceInitStatus = VOICE_INIT_TRY;
u16 sTopScoreWordId = VOICE_WORD_ID_NONE;
u8 numErrors = 0;
u8 region = 0;
u8 numChannel = 0;

// Based mostly on the Majora's Mask decompilation
// Some stuff from https://pastebin.com/TcsfwpSM and https://pastebin.com/ajLzRLze

const ActorInitExplPad VoiceMgr_InitVars =
{
    0xDEAD,
    ACTORCAT_NPC,
    0x00000030,
    1,
    0xBEEF,
    sizeof(VoiceMgr),
    (ActorFunc)VoiceMgr_Init,
    (ActorFunc)VoiceMgr_Destroy,
    (ActorFunc)VoiceMgr_Update,
    (ActorFunc)VoiceMgr_Draw,
};

bool InitVoice(Actor* thisx)
{
    VoiceMgr* this = THIS;
    
    PadMgr_InitVoice();
    
    if (sVoiceInitStatus != VOICE_INIT_SUCCESS)
        return false;
    
    region = PadMgr_GetRegion();
    this->region = region;  
  
    AudioVoice_ResetWord();
    
    this->lastWord = VOICE_WORD_ID_NONE;
    this->showDebugTemp = 0;
    this->turnedOff = true;
    
    return true;
}

void VoiceMgr_Init(Actor* thisx, PlayState* play)
{
    VoiceMgr* this = THIS;
    
#ifdef DISABLE_VRS
        Actor_Kill(&this->actor);
        return;
#endif

    if (!(InitVoice(thisx)))
    {
        Actor_Kill(&this->actor);
        return;
    }       
    
    this->needsReinit = false;
}

void VoiceMgr_Destroy(Actor* thisx, PlayState* play)
{
 
}

void VoiceMgr_Update(Actor* thisx, PlayState* play)
{
    VoiceMgr* this = THIS;
    OSMesgQueue* serialEventQueue;
    
    if (this->showDebugTemp)
        this->showDebugTemp--;
    else
        this->lastWord = VOICE_WORD_ID_NONE;  
    
    if (this->detectionTimer)
        this->detectionTimer--;
    
    if (this->timer)
        this->timer--;
    
    this->soundStatus = 0;
    int errorCode = 0;
    
    if (this->needsReinit)
    {
        numErrors = 0;
        
        if (InitVoice(thisx))
            this->needsReinit = false;   
    }
    
    //if (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_Z))
    //    this->region = !this->region;
    
    if ((!this->disabled && !this->turnedOff && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_CDOWN)) || this->timer || sVoiceContext.mode > VOICE_STATE_LISTENING)
    {   
        switch (sVoiceContext.mode)
        {
            case VOICE_STATE_START:
            {
                if (this->needsReinit)
                    break;
                
                if (!this->listening)
                    SetMusicVolume(play, 60, 5);
                
                serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
                errorCode = osVoiceStartReadData(&gVoiceHandle);
                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);
                sVoiceContext.mode = VOICE_STATE_LISTENING;
                this->listening = true;
                this->lastWord = VOICE_WORD_ID_NONE;
                this->showDebugTemp = 0;
                this->detectionTimer = 0;
                this->timer = 0;
                break;
            }
            case VOICE_STATE_LISTENING:
            {
                if (gVoiceHandle.cmd_status == VOICE_STATUS_BUSY)
                    this->timer = 60;
                
                serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
                errorCode = osVoiceGetReadData(&gVoiceHandle, &D_801FD5C8);
                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);
                        
                this->soundStatus = gVoiceHandle.cmd_status;       
                
                if (gVoiceHandle.cmd_status == VOICE_STATUS_READY || gVoiceHandle.cmd_status == VOICE_STATUS_END)
                {
                    sVoiceContext.data = &D_801FD5C8;
                    this->showDebugTemp = 30;
                    this->detectionTimer = 5;
                    sVoiceContext.mode = VOICE_STATE_GOTWORD;
                    
                    if (!(sVoiceContext.data->answer[0] & 0xFF00) && sVoiceContext.data->distance[0] < 0x400)
                        this->lastWord = sVoiceContext.data->answer[0] % 3;
                    else
                        this->lastWord = VOICE_WORD_ID_NONE;
                }
                
                break;
                    
            }
            case VOICE_STATE_GOTWORD:
            {
                this->timer = 0;
                serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
                errorCode =  osVoiceStopReadData(&gVoiceHandle);
                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);   
                this->listening = false; 
                sVoiceContext.mode = VOICE_STATE_AWAIT_RESTART;   
                SetMusicVolume(play, 127, 5);                
                break;
            }
            case VOICE_STATE_AWAIT_RESTART:
            {
                if (this->showDebugTemp == 0)
                    sVoiceContext.mode = VOICE_STATE_START;                
            }
        }
    }
    else
    {
        switch (sVoiceContext.mode)
        {
            case VOICE_STATE_LISTENING:
            case VOICE_STATE_GOTWORD:
            case VOICE_STATE_AWAIT_RESTART:
            {
                this->timer = 0;
                serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
                errorCode = osVoiceStopReadData(&gVoiceHandle);
                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);   
                sVoiceContext.mode = VOICE_STATE_START;
                this->listening = false;
                SetMusicVolume(play, 127, 5);
            }
        }
    }

    if (AudioVoice_IsRelevantError(errorCode))
        numErrors++;
    else
        numErrors = 0;
    
    if (numErrors >= 2)
    {
        this->needsReinit = true;
        this->lastWord = VOICE_WORD_ID_NONE;
    }
}

void VoiceMgr_Draw(Actor* thisx, PlayState* play)
{
    #if DEBUGVER == 1
        VoiceMgr* this = THIS;
        
        GraphicsContext* __gfxCtx = play->state.gfxCtx;
        Gfx* gfx;
        Gfx* gfxRef;
        gfxRef = POLY_OPA_DISP;
        gfx = Graph_GfxPlusOne(gfxRef);
        gSPDisplayList(OVERLAY_DISP++, gfx);    
        
        char* buf = "DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG";
        char* statuses[] = {"READY", "NO INPUT", "", "AMBIENT NOISE", "", "SOUND DETECTED", "", "END"};
        char* words[] = {"IGIARI", "MATTA", "KURAE", "OBJECTION", "HOLD IT", "TAKE THAT"};
        char* nothing = "NONE";
        char* turnedOff = "TURNED OFF";
        
        if (!this->disabled)
        {
            if (this->needsReinit)
                sprintf(buf, "ERROR. Try re-plugging.");
            else if (this->showDebugTemp && this->lastWord >= 0 && this->lastWord <= VOICE_WORD_ID_MAX)
            {
                if (sVoiceContext.data != NULL)
                    sprintf(buf, "DETECTED WORD: %s, %x, %x", this->lastWord == VOICE_WORD_ID_NONE ? nothing : words[(region ? 3 : 0) + this->lastWord], sVoiceContext.data->answer[0], sVoiceContext.data->distance[0]);                
                else
                    sprintf(buf, "DETECTED WORD: %s", this->lastWord == VOICE_WORD_ID_NONE ? nothing : words[(region ? 3 : 0) + this->lastWord]);                
            }
            else
                sprintf(buf, "%s: %s", this->region ? "VRU" : "VRS", this->turnedOff ? turnedOff : this->listening ? statuses[gVoiceHandle.cmd_status] : "PRESS \x05""\x46""\xA6""\x05""\x40"" TO LISTEN\x02");
        
            TextOperation(play, NULL, &gfx, 
                          COLOR_WHITE, COLOR_BLACK,
                          255, 255, 
                          buf, 
                          GetStringCenterX(buf, TEXT_SCALE), 5, 
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW);                 
                            
        }
        
        
        gSPEndDisplayList(gfx++);
        Graph_BranchDlist(gfxRef, gfx);
        POLY_OPA_DISP = gfx;
    #endif
}

void SetMusicVolume(PlayState* play, float volume, float frames)
{
    SEQCMD_SET_SEQPLAYER_VOLUME(SEQ_PLAYER_BGM_MAIN, (int)frames, (int)volume);
}

s32 AudioVoice_IsRelevantError(s32 errorCode) 
{
    switch (errorCode) 
    {
        case CONT_ERR_NO_CONTROLLER:
        case CONT_ERR_CONTRFAIL:
        case CONT_ERR_INVALID:
        case CONT_ERR_DEVICE:
        case CONT_ERR_VOICE_MEMORY:
        case CONT_ERR_VOICE_WORD:
        case CONT_ERR_VOICE_NO_RESPONSE:
            return -1;
        default:
            return 0;
    }
}

s32 AudioVoice_InitDictionary(OSVoiceDictionary* dict) 
{
    OSMesgQueue* serialEventQueue;
    s32 errorCode;
    u8 numWords;
    u8 i;

    sVoiceContext.mode = VOICE_STATE_START;
    sVoiceContext.data = NULL;
    sVoiceContext.distance = 1000;
    sVoiceContext.answerNum = 5;
    sVoiceContext.warning = 0;
    sVoiceContext.dict = dict;

    numWords = dict->numWords;

    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    errorCode = osVoiceClearDictionary(&gVoiceHandle, numWords);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);

    if (errorCode != 0) 
        return errorCode;

    for (i = 0; i < numWords; i++) 
    {
        serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
        errorCode = osVoiceSetWord(&gVoiceHandle, (u8*)&dict->words[i][0], region, USAWordsFrames[i]);
        PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);
    }

    return errorCode;
}

void AudioVoice_SetVoiceLevel(u16 distance, u16 answerNum, u16 warning, u16 voiceLevel, u16 voiceRelLevel) 
{
    sVoiceContext.distance = distance;
    sVoiceContext.answerNum = answerNum;
    sVoiceContext.warning = warning;
    sVoiceContext.voiceLevel = voiceLevel;
    sVoiceContext.voiceRelLevel = voiceRelLevel;
}

void AudioVoice_ResetWord(void)
{
    s32 errorCode;
    OSMesgQueue* serialEventQueue;

    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    osVoiceStopReadData(&gVoiceHandle);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);

    errorCode = AudioVoice_InitDictionary(region ? &sVoiceDictionaryEnglish : &sVoiceDictionary);

    if (errorCode == 0) 
        AudioVoice_SetVoiceLevel(800, 2, VOICE_WARN_TOO_SMALL, 500, 2000);
}

// Returns 0 if JPN VRS, 1 if US VRU
bool PadMgr_GetRegion(void)
{
    OSMesgQueue* serialEventQueue;
    bool ret;    
    
    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);

    ret = IsVRU(serialEventQueue, &gVoiceHandle, numChannel);

    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);
    
    return ret;
}

void PadMgr_InitVoice(void) 
{
    s32 i;
    OSMesgQueue* serialEventQueue;
    s32 errorCode;
    
    sVoiceInitStatus = VOICE_INIT_TRY;

    // Only polling controllers 2, 3, 4.
    for (i = 1; i < MAXCONTROLLERS; i++) 
    {
        if (gPadMgr.padStatus[i].errno == 0)
        {
            if (gPadMgr.padStatus[i].type == CONT_TYPE_VOICE)
            {
                gPadMgr.ctrlrIsConnected[i] = PADMGR_CONT_VOICE_PLUGGED;

                serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);

                errorCode = osVoiceInit(serialEventQueue, &gVoiceHandle, i);

                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);

                if (errorCode == 0)
                {
                    numChannel = i;
                    gPadMgr.ctrlrIsConnected[i] = PADMGR_CONT_VOICE;
                    sVoiceInitStatus = VOICE_INIT_SUCCESS;
                    break;
                }
            }
        }
    }
    
    // If sVoiceInitStatus is not successful after the first attempt to initialize a VRU, don't try again
    if (sVoiceInitStatus != VOICE_INIT_SUCCESS)
        sVoiceInitStatus = VOICE_INIT_FAILED;
}