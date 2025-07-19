#include "../include/title.h"
#include "../../is64Printf.h"

#define THIS ((TitleLogo*)thisx)

void TitleLogo_Init(Actor* thisx, PlayState* play);
void TitleLogo_Destroy(Actor* thisx, PlayState* play);
void TitleLogo_Update(Actor* thisx, PlayState* play);
void TitleLogo_Draw(Actor* thisx, PlayState* play);

const ActorInitExplPad Title_InitVars =
{
    0xDEAD,
    ACTORCAT_PROP,
    FLAGS,
    OBJECT_MAG,
    0xBEEF,
    sizeof(TitleLogo),
    (ActorFunc)TitleLogo_Init,
    (ActorFunc)TitleLogo_Destroy,
    (ActorFunc)TitleLogo_Update,
    (ActorFunc)TitleLogo_Draw,
};

void TitleLogo_Init(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    
    YREG(1) = 63;
    YREG(7) = 119;
    YREG(8) = 7;
    YREG(9) = 5;
    YREG(10) = 3;
    VREG(9) = 99;
    VREG(19) = 99;
    VREG(21) = 9;
    VREG(23) = 10;

    this->textAlphaFull = 0;
    this->inputPauseTimer = 0;
    this->initialTimer = 30;
    
    int curCutsceneIndex = gSaveContext.cutsceneIndex;
    
    int res = LoadSaveAndVerify(0);  // Slot 1.
    int res2 = SAVE_NOT_HOL;
    
    if (res)
        res2 = LoadSaveAndVerify(3); // Backup for slot 1.
    
    if (res == SAVE_OK || res2 == SAVE_OK)
    {
        this->hasFile = (SAVE_PROGRESS != 0xFFFFFFFF);
        
        if (this->hasFile)
            this->highlightedOption = OPTION_CONTINUE;       
    }
    else if (res == SAVE_NOT_HOL && res2 == SAVE_NOT_HOL)
    {
        this->hasFile = false;
        TitleLogo_InitNewSave();           
    }
    else
    {
        this->saveCorrupted = true;
        this->hasFile = false;
        TitleLogo_InitNewSave();
    }

    #if DEBUGVER == 1
        
        int wasWide = SAVE_WIDESCREEN;
        
        TitleLogo_InitNewSave();
        TitleLogo_InvalidateMsgLogChecksum();
        
        SAVE_WIDESCREEN = wasWide;
        
        this->globalState = TITLESCREEN_STATE_DISPLAY;
        this->saveCorrupted = false;
        
        this->mainAlpha = LOGO_ALPHA_TARGET;
        this->copyrightAlpha = LOGO_ALPHA_TARGET;
        
        SAVE_LANAME = 0xFF;
        SAVE_LAKILLEDBYSHOPKEEPER = 1;
        bcopy("DEBUG", &SAVE_LANAME, 5);
        
    #endif
    
    SAVE_DEBUGMODE = 0;
    SAVE_LAUNCHSCENE = 0;
    SAVE_LAUNCHLEVEL = 0;
    SAVE_LANGUAGE = 1;
    
    gSaveContext.audioSetting = SAVE_AUDIOSETTING;
    gSaveContext.cutsceneIndex = curCutsceneIndex;
    gSaveContext.dayTime = CLOCK_TIME(12, 00);
    gSaveContext.skyboxTime = CLOCK_TIME(12, 00);
    gSaveContext.nextDayTime  = 0xFFFF;
    gSaveContext.nightFlag = 0;
    gSaveContext.gameMode = GAMEMODE_TITLE_SCREEN;
    gSaveContext.forcedSeqId = 0;
}

void TitleLogo_Destroy(Actor* thisx, PlayState* play)
{
    gSaveContext.gameMode = GAMEMODE_NORMAL;
}

void SetTrsPakDisableStatus(Actor* thisx, bool status)
{
    TitleLogo* this = THIS;
    
    if (this->trsPak != NULL)
        this->trsPak->disabled = status;    
}

void HaltCutscene(PlayState* play)
{
    play->csCtx.frames = 30;
}

void TitleLogo_Update_Settings(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);
    
    if (this->aux->actor.update != NULL)
        return;
    
    this->actor.update = &TitleLogo_Update;
    this->actor.draw = &TitleLogo_Draw;   

    SetTrsPakDisableStatus(thisx, false);
}

void TitleLogo_Update_SaveCorrupted(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);
    
    if (this->aux->actor.update != NULL)
        return;
    
    this->actor.update = &TitleLogo_Update;
    this->actor.draw = &TitleLogo_Draw;    
    this->saveCorrupted = false;
    
    SetTrsPakDisableStatus(thisx, false);
}

void TitleLogo_Update_Overwrite(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);
    
    if (this->aux->scriptVars[0] == 0)
        return;
    
    SetTrsPakDisableStatus(thisx, false);
    
    if (this->aux->scriptVars[0] == 1)
    {
        Audio_SetCutsceneFlag(0);
        TitleLogo_NewGame();
        TitleLogo_InvalidateMsgLogChecksum();
        this->globalState = TITLESCREEN_STATE_STARTING_GAME;
        
        SetTrsPakDisableStatus(thisx, true);     
    }
    
    this->actor.update = &TitleLogo_Update;
    this->actor.draw = &TitleLogo_Draw;    
}

void TitleLogo_Update(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);

    ProcessTransferPak(thisx, play);
    
    if (this->initialTimer)
        this->initialTimer--;
    
    if (this->inputPauseTimer)
        this->inputPauseTimer--;
    
    if (this->globalState >= TITLESCREEN_STATE_DISPLAY && this->globalState < TITLESCREEN_STATE_FADE_OUT)
    {
        Math_ApproachS(&this->textAlphaFull, TEXT_ALPHA_TARGET_FULL, 1, TEXT_FADE_SPEED);
        
        if (this->stopTextAlphaCounter)
            this->stopTextAlphaCounter--;
        else
        {
            if (this->textFadeDirection)
                Math_ApproachS(&this->textAlpha, TEXT_ALPHA_TARGET_FULL, 1, TEXT_FADE_SPEED);
            else
                Math_ApproachS(&this->textAlpha, TEXT_ALPHA_TARGET_NONE, 1, TEXT_FADE_SPEED);
            
            if (this->textAlpha == TEXT_ALPHA_TARGET_FULL || this->textAlpha == TEXT_ALPHA_TARGET_NONE)
                this->textFadeDirection = !this->textFadeDirection;
        }
    }

    if (this->globalState < TITLESCREEN_STATE_DISPLAY && !this->inputPauseTimer)
    {
        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_START) ||
            CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A) ||
            CHECK_BTN_ALL(play->state.input[0].press.button, BTN_B))
        {
            Audio_PlaySfxGeneral(NA_SE_SY_PIECE_OF_HEART, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);

            this->mainAlpha = LOGO_ALPHA_TARGET;
            this->copyrightAlpha = LOGO_ALPHA_TARGET;
            this->textAlpha = TEXT_ALPHA_TARGET_FULL;
            this->textAlphaFull = TEXT_ALPHA_TARGET_FULL; 

            this->globalState = TITLESCREEN_STATE_DISPLAY;
            this->inputPauseTimer = 10;
            this->stopTextAlphaCounter = 10;
        }
    }
    
    if (this->globalState == TITLESCREEN_STATE_DISPLAY)
    {
        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_R) && !Message_GetState(&play->msgCtx) && !this->inputPauseTimer)
        {
            Audio_PlaySfxGeneral(NA_SE_SY_WIN_OPEN, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            this->aux = (NpcMaker*)Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_SETTINGS, NPCMAKER_FILE);
            this->actor.update = &TitleLogo_Update_Settings;
            this->actor.draw = &TitleLogo_Draw_Settings;    

            SetTrsPakDisableStatus(thisx, true);

            return;
        }      
    }
    
    
#if DEBUGVER == 1
     // DEBUG MODE ON
    SAVE_DEBUGMODE = 1;
    
    if (this->trsPak == NULL)
        this->trsPak = (TrsPakMgr*)Actor_Spawn(&play->actorCtx, play, 5, 0, 0, 0, 0, 0, 0, TRSPAKMGR_REINITIALIZABLE);
    
    if (this->bioSnsr == NULL)
        this->bioSnsr = (BioSnsrMgr*)Actor_Spawn(&play->actorCtx, play, 6, 0, 0, 0, 0, 0, 0, 0);
    
    if (!this->inputPauseTimer)
    {
        if (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DUP) || play->state.input[0].cur.stick_y > 20)
        {
            Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            this->inputPauseTimer = INPUT_COOLDOWN_DURATION;
            this->selectedScene--;

            if (this->selectedScene < 0)
                this->selectedScene = ARRAY_COUNT(sScenes) - 1;
        }

        if (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DDOWN) || play->state.input[0].cur.stick_y < -20)
        {
            Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            this->inputPauseTimer = INPUT_COOLDOWN_DURATION;
            this->selectedScene++;

            if (this->selectedScene >= ARRAY_COUNT(sScenes))
                this->selectedScene = 0;
        }

        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A))
        {
            Audio_PlaySfxGeneral(NA_SE_SY_PIECE_OF_HEART, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            this->inputPauseTimer = 255;
         
            
            play->nextEntranceIndex = sScenes[this->selectedScene].entranceIndex;
            play->transitionTrigger = TRANS_TRIGGER_START;
            play->transitionType = TRANS_TYPE_FADE_BLACK;
            gSaveContext.nextCutsceneIndex = sScenes[this->selectedScene].cutsceneIndex;
            this->globalState = TITLESCREEN_STATE_FADE_OUT;
            Audio_StopBGMAndFanfares(20);
        }
    }       
    else if (this->globalState == TITLESCREEN_STATE_FADE_OUT)
    {
        Math_ApproachS(&this->mainAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->copyrightAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->textAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->textAlphaFull, 0, 1, FADEOUT_SPEED);
       
        if (this->copyrightAlpha == 0)
            this->globalState = TITLESCREEN_STATE_POST_DISPLAY;
    }        
#else
    if (this->globalState == TITLESCREEN_STATE_DISPLAY)
    {        
        // DEBUG MODE OFF
        SAVE_DEBUGMODE = 0;
        
        if (this->saveCorrupted)
        {
            this->aux = (NpcMaker*)Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_CORRUPTED, NPCMAKER_FILE);
            this->actor.update = &TitleLogo_Update_SaveCorrupted;      
            return;
        }
        
        if (this->trsPak == NULL)
            this->trsPak = (TrsPakMgr*)Actor_Spawn(&play->actorCtx, play, 5, 0, 0, 0, 0, 0, 0, TRSPAKMGR_REINITIALIZABLE);        
        
        if (!this->inputPauseTimer)
        {
            if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_START) ||
                CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A))
            {
                if (this->highlightedOption == OPTION_STAGESELECT)
                {
                    this->numScenesVisible = 0;
                    bzero(this->scenesDisplayed, 30);
                    
                    // Count the number of scenes that will be shown...
                    for (int i = 0; i < ARRAY_COUNT(SceneSelectData); i++)
                    {
                        if ((SAVE_HASBEATENGAME && SceneSelectData[i].extraScene == 0) ||
                            (SAVE_HASBEATENGAME && SceneSelectData[i].extraScene == (1 << 0)) ||
                            (SAVE_EXTRASCENES & SceneSelectData[i].extraScene))
                            {
                                if (this->numScenesVisible == 0)
                                    this->selectedScene = SceneSelectData[i].idx;
                                
                                this->scenesDisplayed[this->numScenesVisible] = SceneSelectData[i].idx;
                                this->numScenesVisible++;
                            }
                    }
                    
                    this->sceneSelectRed = 0;
                    this->textAlpha = 255;
                    this->actor.draw = &TitleLogo_DrawSceneSelect;
                    this->globalState = TITLESCREEN_STATE_SCENE_SELECT;
                    SetTrsPakDisableStatus(thisx, true);
                    
                    play->envCtx.fillScreen = 1;
                    play->envCtx.screenFillColor[0] = 0;
                    play->envCtx.screenFillColor[1] = 0;
                    play->envCtx.screenFillColor[2] = 0;
                    play->envCtx.screenFillColor[3] = 150;  
                    
                    Audio_PlaySfxGeneral(NA_SE_SY_LOCK_ON_HUMAN, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                }
                else
                { 
                    if (play->transitionTrigger != TRANS_TRIGGER_START)
                    {
                        Audio_PlaySfxGeneral(NA_SE_SY_PIECE_OF_HEART, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        
                        // If NEW SAVE chosen, or no new save, make a new savefile and invalidate msglog.
                        if (this->highlightedOption == OPTION_NEWGAME || !this->hasFile)
                        {
                            if (this->hasFile)
                            {
                                this->textAlpha = 255;
                                this->aux = (NpcMaker*)Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_OVERWRITEQ, NPCMAKER_FILE);
                                this->actor.update = &TitleLogo_Update_Overwrite;
                                //this->actor.draw = &TitleLogo_Draw_Settings;    
                                SetTrsPakDisableStatus(thisx, true);  
                                return;
                            }
                            else
                            {
                                Audio_SetCutsceneFlag(0);
                                TitleLogo_NewGame();
                                TitleLogo_InvalidateMsgLogChecksum();
                                this->globalState = TITLESCREEN_STATE_STARTING_GAME;
                                SetTrsPakDisableStatus(thisx, true);   
                                return;
                            }
                        }
                        else
                        {
                            Audio_SetCutsceneFlag(0);
                            this->globalState = TITLESCREEN_STATE_STARTING_GAME;
                            SetTrsPakDisableStatus(thisx, true);  
                            return;
                        }
                    }
                }
            }
            else if (this->hasFile && (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DRIGHT) || play->state.input[0].cur.stick_x >= 20))
            {
                this->highlightedOption = (this->highlightedOption == OPTION_CONTINUE ? OPTION_NEWGAME : OPTION_CONTINUE);
                goto common;
            }
            else if (this->hasFile  && (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_DLEFT) || play->state.input[0].cur.stick_x <= -20))
            {
                this->highlightedOption = (this->highlightedOption == OPTION_NEWGAME ? OPTION_CONTINUE : OPTION_NEWGAME);
                goto common;  
            }                
            else if ((SAVE_HASBEATENGAME || SAVE_EXTRASCENES) && (CHECK_BTN_ANY(play->state.input[0].press.button, BTN_DDOWN | BTN_DUP) || (ABS(play->state.input[0].cur.stick_y) >= 30)))
            {
                this->highlightedOption = this->highlightedOption == OPTION_STAGESELECT ? OPTION_NEWGAME : OPTION_STAGESELECT;
                
                common:
                this->stopTextAlphaCounter = TEXT_FADE_SPEED_PAUSE_DURATION;
                this->textAlpha = TEXT_ALPHA_TARGET_FULL;
                this->inputPauseTimer = INPUT_COOLDOWN_DURATION;

                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);   
            }                           
        }
    }     
    else if (this->globalState == TITLESCREEN_STATE_STARTING_GAME)
    {
        //if more cases are implemented, this will spawn different controller actors
        Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_CONTROLLER, NPCMAKER_FILE);
        this->globalState = TITLESCREEN_STATE_FADE_OUT;        
        SetTrsPakDisableStatus(thisx, true);   
    }
    else if (this->globalState == TITLESCREEN_STATE_SCENE_SELECT)
    {
        bool scrollInputMade = false;
        
        if (this->inputPauseTimer)
            this->firstInput = false;
        else
            this->firstInput = true;
        
        if (ABS(play->state.input[0].cur.stick_y) > 20 || CHECK_BTN_ANY(play->state.input[0].cur.button, BTN_DUP | BTN_DDOWN))
            scrollInputMade = true;

        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_START) ||
            CHECK_BTN_ALL(play->state.input[0].press.button, BTN_A))
        {
            sSelectEntry* selectedEntry = &SceneSelectData[0];
            
            for (int i = 0; i < ARRAY_COUNT(SceneSelectData); i++)
            {
                if (SceneSelectData[i].idx == this->selectedScene)
                    selectedEntry = &SceneSelectData[i];
            }
            
            Audio_PlaySfxGeneral(NA_SE_SY_PIECE_OF_HEART, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            
            switch (selectedEntry->idx)
            {
                case EXTRA_TALONGAME:
                {
                    TitleLogo_GoTalonGame(thisx, play); 
                    break;
                }
                case EXTRA_ZELDA:
                {
                    TitleLogo_GoIngoCasino(thisx, play); 
                    break;                    
                }
                default:
                {
                    SAVE_PROGRESS = selectedEntry->progress;   
                    SAVE_HEALTH = 5;
                    TitleLogo_InvalidateMsgLogChecksum();
                    this->globalState = TITLESCREEN_STATE_STARTING_GAME;
                }
            }
        }                
        else if (!this->inputPauseTimer && (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DDOWN) || play->state.input[0].cur.stick_y <= -20))
        {        
            u8 before = this->selectedSceneId;
            this->selectedSceneId++;
            
            if (this->selectedSceneId >= this->numScenesVisible)
                this->selectedSceneId = 0;
            
            this->selectedScene = this->scenesDisplayed[this->selectedSceneId];
            
            if (before != this->selectedSceneId)
                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb); 
        }
        else if (!this->inputPauseTimer && (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DUP) || play->state.input[0].cur.stick_y >= 20))
        {        
            u8 before = this->selectedSceneId;
            this->selectedSceneId--;
            
            if (this->selectedSceneId < 0)
                this->selectedSceneId = this->numScenesVisible - 1;
            
            this->selectedScene = this->scenesDisplayed[this->selectedSceneId];
            
            if (before != this->selectedSceneId)
                Audio_PlaySfxGeneral(NA_SE_SY_FSEL_CURSOR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb); 
        } 
        else if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_B))
        {
            this->stopTextAlphaCounter = TEXT_FADE_SPEED_PAUSE_DURATION;
            this->textAlpha = TEXT_ALPHA_TARGET_FULL;            
            this->globalState = TITLESCREEN_STATE_DISPLAY;
            this->actor.draw = &TitleLogo_Draw;
            play->envCtx.fillScreen = 0;
            Audio_PlaySfxGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            SetTrsPakDisableStatus(thisx, false);  
        } 
        
        if (scrollInputMade)
        {
            if (!this->inputPauseTimer)
            {
                this->inputPauseTimer = 2;
                
                if (this->firstInput)
                    this->inputPauseTimer *= 3;
            }
        }
        else
        {
            this->firstInput = true;
            this->inputPauseTimer = 0;
        }
    }      
 #endif

    if (this->globalState == TITLESCREEN_STATE_FADE_IN)
    {
        Math_ApproachS(&this->mainAlpha, LOGO_ALPHA_TARGET, 1, FADEIN_SPEED);

        if (this->mainAlpha >= LOGO_ALPHA_TARGET)
        {                                    
            Math_ApproachS(&this->copyrightAlpha, LOGO_ALPHA_TARGET, 1, FADEIN_SPEED);
        }
        
        if (this->copyrightAlpha >= LOGO_ALPHA_TARGET)
        {            
            this->globalState = TITLESCREEN_STATE_DISPLAY;
            this->inputPauseTimer = 20;
        }
    }
    else if (this->globalState == TITLESCREEN_STATE_FADE_OUT)
    {
        Math_ApproachS(&this->mainAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->copyrightAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->textAlpha, 0, 1, FADEOUT_SPEED);
        Math_ApproachS(&this->textAlphaFull, 0, 1, FADEOUT_SPEED);
       
        if (this->copyrightAlpha == 0)
            this->globalState = TITLESCREEN_STATE_POST_DISPLAY;
    }

    if (this->globalState == TITLESCREEN_STATE_INITIAL && !this->initialTimer)
        this->globalState = TITLESCREEN_STATE_FADE_IN;
    else if (this->globalState == TITLESCREEN_STATE_DISPLAY && Flags_GetEnv(play, 4))
        this->globalState = TITLESCREEN_STATE_FADE_OUT;

}

void TitleLogo_Draw_Settings(Actor* thisx, PlayState* play)
{
}

void TitleLogo_Draw(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    Gfx* gfx;
    Gfx* gfxRef;

    GraphicsContext* __gfxCtx = play->state.gfxCtx;

    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);

    Color_RGB8 cOutline = (Color_RGB8){0, 0, 0};
    Color_RGB8 cEnabled = (Color_RGB8){0xFF, 0xFF, 0x43};
    Color_RGB8 cDisabled = (Color_RGB8){0xA6, 0xA6, 0xA6};

  
#if DEBUGVER == 1

    TitleLogo_DrawLogo(thisx, play, &gfx);
    
    Gfx_SetupDL_39Ptr(&gfx);
    gDPPipeSync(gfx++);    

    if (this->globalState < TITLESCREEN_STATE_FADE_OUT)
    {       
        //TitleLogo_DrawText(thisx, &gfx, BUILDUSERSTRING, GetStringCenterX(BUILDUSERSTRING), 10, cDisabled, cOutline, 255);
        HoL_DrawMessageText(play, 
                            &gfx, 
                            cDisabled, 
                            cOutline, 
                            255, 
                            255, 
                            sDebugVersion, 
                            GetStringCenterX(sDebugVersion, TEXT_SCALE_TITLE), 
                            10, 
                            1, 
                            1, 
                            NULL, 
                            TEXT_SCALE_TITLE, 
                            OPERATION_DRAW_SHADOW); 
        
        sprintf(BUILDUSERSTRING, "BUILD %s %s", __DATE__, __TIME__);
        HoL_DrawMessageText(play, 
                            &gfx, 
                            cDisabled, 
                            cOutline, 
                            255, 
                            255, 
                            BUILDUSERSTRING, 
                            GetStringCenterX(BUILDUSERSTRING, TEXT_SCALE_TITLE), 
                            220, 
                            1, 
                            1, 
                            NULL, 
                            TEXT_SCALE_TITLE, 
                            OPERATION_DRAW_SHADOW); 

        for (int i = 0; i < ARRAY_COUNT(sScenes); i++)
            HoL_DrawMessageText(play, 
                                &gfx, 
                                i == this->selectedScene ? cEnabled : cDisabled, 
                                cOutline, 
                                255, 
                                255, 
                                sScenes[i].name, 
                                GetStringCenterX(sScenes[i].name, TEXT_SCALE_TITLE), 
                                85 + (i * 16), 
                                1, 
                                1, 
                                NULL, 
                                TEXT_SCALE_TITLE, 
                                OPERATION_DRAW_SHADOW);      
    }

#else
    
    TitleLogo_DrawLogo(thisx, play, &gfx);
    Color_RGB8 cRed = (Color_RGB8){0xFF, 0x00, 0x00};

    if (this->mainAlpha > 0)
    {
        HoL_DrawMessageText(play, 
                           &gfx, 
                           cDisabled, 
                           cOutline, 
                           this->mainAlpha, 
                           this->mainAlpha, 
                           StringVERSION, 
                           285 + (SAVE_WIDESCREEN ? 50 : 0), 
                           220, 
                           1, 
                           1, 
                           NULL, 
                           65, 
                           OPERATION_DRAW_SHADOW);  
                           
                                   
        if (this->globalState == TITLESCREEN_STATE_DISPLAY)
        {
            HoL_DrawMessageText(play, 
                               &gfx, 
                               cDisabled, 
                               cOutline, 
                               this->mainAlpha, 
                               this->mainAlpha, 
                               StringSETTINGS, 
                               265 + (SAVE_WIDESCREEN ? 50 : 0), 
                               20, 
                               1, 
                               1, 
                               NULL, 
                               65, 
                               OPERATION_DRAW_SHADOW);  
        }
    }


    if (this->globalState >= TITLESCREEN_STATE_DISPLAY)
    {         
        int stageSelectOffset = ((SAVE_HASBEATENGAME || SAVE_EXTRASCENES) ? 12 : 0);

        if (!this->hasFile)
        {
            HoL_DrawMessageText(play, 
                                &gfx, 
                                this->highlightedOption == 0 ? cEnabled : cDisabled, 
                                cOutline, 
                                this->highlightedOption == 0 ? this->textAlpha : this->textAlphaFull, 
                                this->highlightedOption == 0 ? this->textAlpha : this->textAlphaFull, 
                                NewGameString, 
                                GetStringCenterX(NewGameString, TEXT_SCALE_TITLE), 
                                170 - stageSelectOffset, 
                                1, 
                                1, 
                                NULL, 
                                TEXT_SCALE_TITLE, 
                                OPERATION_DRAW_SHADOW);
                                
            if (SAVE_EXTRASCENES)
            {
                HoL_DrawMessageText(play, 
                                    &gfx, 
                                    this->highlightedOption == 2 ? cEnabled : this->sceneSelectRed ? cRed : cDisabled, 
                                    cOutline, 
                                    this->highlightedOption == 2 ? this->textAlpha : this->textAlphaFull, 
                                    this->highlightedOption == 2 ? this->textAlpha : this->textAlphaFull, 
                                    SceneSelectString, 
                                    GetStringCenterX(SceneSelectString, TEXT_SCALE_TITLE) + (SAVE_WIDESCREEN ? 5 : 0), 
                                    186 - stageSelectOffset, 
                                    1, 
                                    1, 
                                    NULL, 
                                    TEXT_SCALE_TITLE, 
                                    OPERATION_DRAW_SHADOW);
            }                                 
        }
        else
        {
            HoL_DrawMessageText(play, 
                                &gfx, 
                                this->highlightedOption == 0 ? cEnabled : cDisabled, 
                                cOutline, 
                                this->highlightedOption == 0 ? this->textAlpha : this->textAlphaFull, 
                                this->highlightedOption == 0 ? this->textAlpha : this->textAlphaFull, 
                                NewGameString, 
                                GetStringCenterX(NewGameString, TEXT_SCALE_TITLE) - 50, 
                                170 - stageSelectOffset, 
                                1, 
                                1, 
                                NULL, 
                                TEXT_SCALE_TITLE, 
                                OPERATION_DRAW_SHADOW);
            
            HoL_DrawMessageText(play, 
                                &gfx, 
                                this->highlightedOption == 1 ? cEnabled : cDisabled, 
                                cOutline, 
                                this->highlightedOption == 1 ? this->textAlpha : this->textAlphaFull, 
                                this->highlightedOption == 1 ? this->textAlpha : this->textAlphaFull, 
                                ContinueString, 
                                GetStringCenterX(ContinueString, TEXT_SCALE_TITLE) + 50 + (SAVE_WIDESCREEN ? 5 : 2), 
                                170 - stageSelectOffset, 
                                1, 
                                1, 
                                NULL, 
                                TEXT_SCALE_TITLE, 
                                OPERATION_DRAW_SHADOW);
                                
                                
            if (SAVE_HASBEATENGAME || SAVE_EXTRASCENES)
            {
                HoL_DrawMessageText(play, 
                                    &gfx, 
                                    this->highlightedOption == 2 ? cEnabled : this->sceneSelectRed ? cRed : cDisabled,  
                                    cOutline, 
                                    this->highlightedOption == 2 ? this->textAlpha : this->textAlphaFull, 
                                    this->highlightedOption == 2 ? this->textAlpha : this->textAlphaFull, 
                                    SceneSelectString, 
                                    GetStringCenterX(SceneSelectString, TEXT_SCALE_TITLE) + (SAVE_WIDESCREEN ? 5 : 0), 
                                    186 - stageSelectOffset, 
                                    1, 
                                    1, 
                                    NULL, 
                                    TEXT_SCALE_TITLE, 
                                    OPERATION_DRAW_SHADOW);
            }                
        }
        
    }

#endif


    if (this->drawGbCamEasterEgg > 1 && play->msgCtx.msgMode == MSGMODE_NONE && this->trsPak != NULL)
    {
        GBCameraIndividual* in = (GBCameraIndividual*)this->trsPak->individualData;
        
        if (in->status == FUNCTION_OK && in->gbCameraPic != NULL)
        {
            if (this->drawGbCamEasterEgg == GBCAMERA_EASTEREGG_LENGTH)
            {
                this->stringSlot = Rand_S16Offset(0, 5);
                this->gbCamEasterEggAlpha = 255;
                Audio_PlaySfxGeneral(NA_SE_EN_GANON_LAUGH, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            }
            
            if (this->drawGbCamEasterEgg < 50)
                Math_ApproachS(&this->gbCamEasterEggAlpha, 0, 1, 5);
            
            if (this->gbCamEasterEggAlpha > 0)
            {
                Draw2DInternal(IA8, (u8*)in->gbCameraPic, NULL, &gfx, 160, 120, 128, 112, 320, 240, this->gbCamEasterEggAlpha);
                
                Color_RGB8 clRed = (Color_RGB8){0xFF, 0x00, 0x00};
                HoL_DrawMessageText(play, 
                                    &gfx, 
                                    clRed,  
                                    cOutline, 
                                    this->gbCamEasterEggAlpha, 
                                    this->gbCamEasterEggAlpha, 
                                    gbCamEasterEggStrings[this->stringSlot], 
                                    GetStringCenterX(gbCamEasterEggStrings[this->stringSlot], 140), 
                                    48, 
                                    1, 
                                    1, 
                                    NULL, 
                                    140, 
                                    OPERATION_DRAW_SHADOW);          
            }                                
        }
        else
            this->drawGbCamEasterEgg = 1;
        
        this->drawGbCamEasterEgg--;
    }


    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;

}

void TitleLogo_GoTalonGame(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;    
    SAVE_LASTDIEDONSCENE = 0;
    gSaveContext.dayTime = CLOCK_TIME(16,00);
   
    play->nextEntranceIndex = ROUTE_TALONGAME;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_BLACK;
    gSaveContext.nextCutsceneIndex = 0xFFF0;
    this->globalState = TITLESCREEN_STATE_FADE_OUT;
    Audio_StopBGMAndFanfares(20);      
}

void TitleLogo_GoIngoCasino(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;    
    SAVE_LASTDIEDONSCENE = 0;
    gSaveContext.dayTime = CLOCK_TIME(16,00);
   
    play->nextEntranceIndex = ROUTE_CASINO;
    play->transitionTrigger = TRANS_TRIGGER_START;
    play->transitionType = TRANS_TYPE_FADE_BLACK;
    gSaveContext.nextCutsceneIndex = 0xFFF0;
    this->globalState = TITLESCREEN_STATE_FADE_OUT;
    Audio_StopBGMAndFanfares(20);      
}

void ProcessTransferPak(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;  
    
    if (this->globalState == TITLESCREEN_STATE_DISPLAY)
    {
        // Undo the antipiracy gag save so it can be retriggered if the player removes the transfer pak or inits another game.
        if (gPadMgr.pakType[0] != CONT_PAK_OTHER ||
           (this->trsPak != NULL && this->trsPak->init == INIT_DONE && this->trsPak->insertedGame != GBGAME_EVERDRIVE))
        {
            if (SAVE_ANTIPIRACYGAG)
            {
                SAVE_ANTIPIRACYGAG = 0;
                Sram_WriteSave(&play->sramCtx); 
            }        
        }
    
        if (this->trsPak != NULL && this->trsPak->init == INIT_DONE)
        {
            switch (this->trsPak->insertedGame)
            {
                case GBGAME_GBCAMERA:
                case GBGAME_POCKETCAMERA:
                {                
                    // Randomly choose a picture from the available ones in the save, then show it full-screen
                    // Can be triggered only once per titlescreen viewing
                    if (this->drawGbCamEasterEgg == 0)
                    {
                        GBCameraIndividual* data = (GBCameraIndividual*)this->trsPak->individualData;
                        
                        bool doEasterEgg = false;
                        
                        for (int i = 0; i < GBCAMERA_NUMSLOTS; i++)
                            if (data->slots[i] != GBCAMERA_SLOT_EMPTY)
                                doEasterEgg = true;
                              
                        if (doEasterEgg)
                        {
                            u8 slot = Rand_S16Offset(0, GBCAMERA_NUMSLOTS);
                            
                            while (data->slots[slot] == GBCAMERA_SLOT_EMPTY)
                                slot = Rand_S16Offset(0, GBCAMERA_NUMSLOTS - 1);
                            
                            u16 param = 2;
                            param |= (data->slots[slot] << 8);
                        
                            this->trsPak->gameFuncFunc((Actor*)this->trsPak, param);
                            this->drawGbCamEasterEgg = GBCAMERA_EASTEREGG_LENGTH;
                        }
                        else
                            this->drawGbCamEasterEgg = 1;
                    }        
                    break; 
                }
                /*
                // Unlock Talongame Early
                case GBGAME_SUPERMARIOLAND:
                case GBGAME_SUPERMARIOLAND2:
                case GBGAME_WARIOLAND1:
                case GBGAME_WARIOLAND2:
                case GBGAME_CGBWARIOLAND2:
                case GBGAME_WARIOLAND3:
                {
                    if ((SAVE_EXTRASCENES & (1 << 0)) == 0)
                    {
                        Audio_PlaySfxGeneral(NA_SE_VO_TA_SURPRISE, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        SAVE_EXTRASCENES |= (1 << 0);
                        this->sceneSelectRed = 1;

                        Sram_WriteSave(&play->sramCtx);    
                    }        
                    break;
                }
                */
                // Unlock Ingo's Casino:
                case GBGAME_ORACLEOFAGES:
                case GBGAME_ORACLEOFSEASONS:
                {
                    if ((SAVE_EXTRASCENES & (1 << 1)) == 0)
                    {
                        Audio_PlaySfxGeneral(NA_SE_SY_CORRECT_CHIME, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        SAVE_EXTRASCENES |= (1 << 1);

                        Sram_WriteSave(&play->sramCtx); 
                        this->sceneSelectRed = 1;
                    }          

                    break;
                }
                // Unlock Thief Case
                case GBGAME_LINKSAWAKENING:
                {
                    if ((SAVE_EXTRASCENES & (1 << 2)) == 0)
                    {
                        Audio_PlaySfxGeneral(0x28DB, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        SAVE_EXTRASCENES |= (1 << 2);
                        
                        Sram_WriteSave(&play->sramCtx); 
                        this->sceneSelectRed = 1;
                    }
                 
                    break;
                }
                case GBGAME_EVERDRIVE:
                {
                    if (!SAVE_ANTIPIRACYGAG)
                    {
                        Audio_PlaySfxGeneral(0x683A, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        SAVE_ANTIPIRACYGAG = 1;
                        
                        Sram_WriteSave(&play->sramCtx); 
                        
                        SAVE_LAUNCHLEVEL = SCENE_ID_COURT;
                        SAVE_LAUNCHSCENE = 96;
                        
                        this->globalState = TITLESCREEN_STATE_STARTING_GAME;
                    }        
                    break;
                }
                case GBGAME_EZGB:
                {
                    // No idea how to initialize the EZGB, so the scene should not mention that you can.
                     if (!SAVE_ANTIPIRACYGAG)
                    {
                        Audio_PlaySfxGeneral(0x683A, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                        SAVE_ANTIPIRACYGAG = 1;
                        
                        Sram_WriteSave(&play->sramCtx); 
                        
                        SAVE_LAUNCHLEVEL = SCENE_ID_COURT;
                        SAVE_LAUNCHSCENE = 99;
                        
                        this->globalState = TITLESCREEN_STATE_STARTING_GAME;
                    }        
                    break;                   
                }
                default:
                    break;
            }
        }
    }    
}


void TitleLogo_InvalidateMsgLogChecksum()
{
    #ifdef SAVE_MSGLOG  
        // Invalidate message log checksum so that you don't see random messages from elsewhere in game when using the scene select.
        u32 header[4];
        header[0] = 0;
        header[1] = 0;
        header[2] = 0;
        header[3] = 0;
    
        SsSram_ReadWrite(OS_K1_TO_PHYSICAL(0xA8000000) + SLOT_OFFSET(1), &header, 16, OS_WRITE);
    #endif
}

void TitleLogo_InitNewSave()
{
    Sram_InitNewSave();
    
    // Why is this info not cleared in Init New Save???
    bzero(&gSaveContext, 0x18);

    gSaveContext.fileNum = 0;
    gSaveContext.audioSetting = 0;    
    
    SAVE_PROGRESS = 0xFFFFFFFF;
    SAVE_HEALTH = 5;
    SAVE_EVIDENCE = 0;
    SAVE_DEBUG_DEPRECATED = 0;
    SAVE_CREDITSDEBUGCOUNTER = 0;
    SAVE_DEBUGMODE = 0;
    SAVE_LASTDIEDONSCENE = 0;
    SAVE_ANTIPIRACYGAG = 0;
    SAVE_LAUNCHSCENE = 0;
    SAVE_LAUNCHLEVEL = 0;
    SAVE_LAKILLEDBYSHOPKEEPER = 0;
    SAVE_NUMTHEFTS = 0;
    SAVE_EXTRASCENES = 0;
    *SAVE_CASINORUPEES = 20;
    SAVE_INGOHINT = 0;
    SAVE_SHOWNTALONGAMECONTROLS = 0;
    SAVE_LATUNIC = 0;
    SAVE_SHOWNUNLOCK = 0;
    
    SAVE_SCREENXPOS = 0;
    SAVE_SCREENYPOS = 0;
    SAVE_SCREENSIZEX = 225;
    SAVE_SCREENSIZEY = 225;
    SAVE_WIDESCREEN = 0;
    SAVE_ANTIALIASOFF = 0;
    
    SAVE_LANGUAGE = 0;
    
    bcopy("ZELDA", &SAVE_LANAME, 5);
    bcopy(&sHeroOfLawMagic, &gSaveContext.playerName, ARRAY_COUNTU(sHeroOfLawMagic));
}

void TitleLogo_NewGame()
{
    gSaveContext.fileNum = 0;
    SAVE_PROGRESS = 0;
    SAVE_HEALTH = 5;
    SAVE_EVIDENCE = 0;
    SAVE_DEBUG_DEPRECATED = 0;
    SAVE_CREDITSDEBUGCOUNTER = 0;
    SAVE_DEBUGMODE = 0;
    SAVE_LASTDIEDONSCENE = 0;
    SAVE_LAUNCHSCENE = 0;
    SAVE_LAUNCHLEVEL = 0;
}

int mStrlen(const char *str)
{
    const char *s;
    for (s = str; *s; ++s) {}
    return (s - str);
}

void TitleLogo_DrawLogo(Actor* thisx, PlayState* play, Gfx** gfxp)
{
    TitleLogo* this = THIS;
    Gfx* gfx = *gfxp;

    int mA = this->mainAlpha;
    int cA = this->copyrightAlpha;
    
#if DEBUGVER == 1
    mA = MIN(40, mA);
    cA = MIN(40, cA);
#endif

    if ((s16)this->mainAlpha != 0)
        Draw2D(RGBA32, OBJECT_MAG, play, &gfx, 160, 90, (u8*)LOGO_OFFSET, NULL, 180, 140, mA);  
        
    if ((s16)this->copyrightAlpha != 0)
        Draw2D(RGBA32, OBJECT_MAG, play, &gfx, 160, 210, (u8*)COPYRIGHT_OFFSET, NULL, 200, 16, cA); 

    *gfxp = gfx;
}

int GetScreenCenterY(int Size)
{
    return (SCREEN_HEIGHT / 2) - Size / 2;
}

void TitleLogo_DrawSceneSelect(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    Gfx* gfx;
    Gfx* gfxRef;

    GraphicsContext* __gfxCtx = play->state.gfxCtx;

    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);
    
    Gfx_SetupDL_39Ptr(&gfx);
    gDPPipeSync(gfx++);      
       
    Color_RGB8 cOutline = (Color_RGB8){0, 0, 0};
    Color_RGB8 cEnabled = (Color_RGB8){0xFF, 0xFF, 0x43};
    Color_RGB8 cDisabled = (Color_RGB8){0xA6, 0xA6, 0xA6};    
    Color_RGB8 cHeader = (Color_RGB8){0xFF, 0x00, 0x00}; 

    HoL_DrawMessageText(play, 
                        &gfx, 
                        cHeader, 
                        cOutline, 
                        this->textAlpha, 
                        this->textAlpha, 
                        SceneSelectString, 
                        GetStringCenterX(SceneSelectString, TEXT_SCALE_TITLE), 
                        20, 
                        1, 
                        1, 
                        NULL, 
                        TEXT_SCALE_TITLE, 
                        OPERATION_DRAW_SHADOW);     
                        
                        
    int posY = 48;        
    int start = (this->selectedSceneId / 13) * 13;
    
    for (int i = start; i < ARRAY_COUNT(SceneSelectData); i++)
    {
        if ((SceneSelectData[i].extraScene == 0 && SAVE_HASBEATENGAME) || 
            (SAVE_EXTRASCENES & SceneSelectData[i].extraScene) || 
            (SAVE_HASBEATENGAME && SceneSelectData[i].extraScene == (1 << 0))) 
        {
            if (posY + 16 < 240)
            {
                HoL_DrawMessageText(play, 
                                    &gfx, 
                                    this->selectedScene == SceneSelectData[i].idx ? cEnabled : cDisabled, 
                                    cOutline, 
                                    this->textAlpha, 
                                    this->textAlpha, 
                                    SceneSelectData[i].name, 
                                    GetStringCenterX(SceneSelectData[i].name, 85), 
                                    posY, 
                                    1, 
                                    1, 
                                    NULL, 
                                    85, 
                                    OPERATION_DRAW_SHADOW); 
            }

            posY += 14;
        }
    }  

    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;    
}