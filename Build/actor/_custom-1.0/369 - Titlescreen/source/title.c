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
  
    this->textAlphaFull = 0;
    this->inputPauseTimer = 0;
    this->initialTimer = 30;
    
    int curCutsceneIndex = gSaveContext.cutsceneIndex;
    int Language = SAVE_LANGUAGE;
 
    int res = LoadSaveAndVerify(SAVE_SLOT);  // Slot 1.
    int res2 = SAVE_NOT_HOL;
    
    if (res)
        res2 = LoadSaveAndVerify(SAVE_SLOT_BACKUP); // Backup for slot 1.
    
    // Clear save file if L + R + Z held    
    if (CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_L | BTN_R | BTN_Z))
    {
        res = SAVE_NOT_HOL;
        res2 = SAVE_NOT_HOL;
    }
    
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
        InvalidateMsgLogChecksum();
        
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
    
    SAVE_LANGUAGE = Language;
    
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

void TitleLogo_Update(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);

    ProcessTransferPak(thisx, play);
    
    // Start reloading the font on the frame we stopped drawing
    if (this->fontReloadTimer == FONT_RELOAD_LENGTH - 1)
        Font_LoadFont(&play->msgCtx.font);    
    
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
    
    if (this->globalState == TITLESCREEN_STATE_DISPLAY && !Message_GetState(&play->msgCtx) && this->drawGbCamEasterEgg <= 1 && !this->fontReloadTimer)
    {
        if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_R))
        {
            Audio_PlaySfxGeneral(NA_SE_SY_WIN_OPEN, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
            this->aux = (NpcMaker*)Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_SETTINGS, NPCMAKER_FILE);
            this->actor.update = &TitleLogo_Update_Settings;
            this->actor.draw = &TitleLogo_Draw_Settings;    

            SetTrsPakDisableStatus(thisx, true);

            return;
        }      
        
        #ifdef LANGUAGE_PICKER
        
            if (CHECK_BTN_ALL(play->state.input[0].press.button, BTN_L))
            {
                Audio_PlaySfxGeneral(0x6E73, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                
                SAVE_LANGUAGE++;
                
                if (SAVE_LANGUAGE > HOL_LANGUAGE_MAX - 1)
                    SAVE_LANGUAGE = 0;
                
                this->fontReloadTimer = FONT_RELOAD_LENGTH;
                
                // The message log needs to be erased when the language is changed.
                InvalidateMsgLogChecksum();
            }       

        #endif
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
                    this->sceneSelectRed = 0;
                    Audio_PlaySfxGeneral(NA_SE_SY_LOCK_ON_HUMAN, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                    this->aux = (NpcMaker*)Actor_Spawn(&play->actorCtx, play, 3, 0, 0, 0, 0, 0, NPCMAKER_ACTOR_SCENE_SELECT, NPCMAKER_FILE);
                    this->actor.update = &TitleLogo_Update_SceneSelect;
                    this->actor.draw = &TitleLogo_DrawSceneSelect;    

                    SetTrsPakDisableStatus(thisx, true);
                    return;                    
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
                                InvalidateMsgLogChecksum();
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

void TitleLogo_Update_SceneSelect(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    HaltCutscene(play);
    
    if (this->aux->scriptVars[0] < 0)
        return;
    
    switch (this->aux->scriptVars[0])
    {
        case SCENE_SELECT_CHOICE_CANCEL:
        {
            this->stopTextAlphaCounter = TEXT_FADE_SPEED_PAUSE_DURATION;
            this->textAlpha = TEXT_ALPHA_TARGET_FULL;            
            this->globalState = TITLESCREEN_STATE_DISPLAY;
            Audio_PlaySfxGeneral(NA_SE_SY_CANCEL, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);   
            break;            
        }
        case SCENE_SELECT_CHOICE_TALONGAME:
        {
            TitleLogo_GoTalonGame(thisx, play);
            break;    
        }
        case SCENE_SELECT_CHOICE_CASINO:
        {
            TitleLogo_GoIngoCasino(thisx, play); 
            break;    
        }
        default:
        {
            SAVE_PROGRESS = this->aux->scriptVars[0];   
            SAVE_HEALTH = 5;
            InvalidateMsgLogChecksum();
            this->globalState = TITLESCREEN_STATE_STARTING_GAME;  
            break;               
        }
    }
    
    SetTrsPakDisableStatus(thisx, false);  
    Actor_Kill(&this->aux->actor);
    this->actor.update = &TitleLogo_Update;
    this->actor.draw = &TitleLogo_Draw; 
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
        InvalidateMsgLogChecksum();
        this->globalState = TITLESCREEN_STATE_STARTING_GAME;
        
        SetTrsPakDisableStatus(thisx, true);     
    }
    
    this->actor.update = &TitleLogo_Update;
    this->actor.draw = &TitleLogo_Draw;    
}

void TitleLogo_Draw(Actor* thisx, PlayState* play)
{
    TitleLogo* this = THIS;
    GraphicsContext* __gfxCtx = play->state.gfxCtx;
    Gfx* gfx;
    Gfx* gfxRef;
    
    if (this->fontReloadTimer)
        this->fontReloadTimer--;


    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);

    TitleLogo_DrawLogo(thisx, play, &gfx);
    
    if (this->fontReloadTimer <= 1)
    {
    #if DEBUGVER == 1

        Gfx_SetupDL_39Ptr(&gfx);
        gDPPipeSync(gfx++);    

        if (this->globalState < TITLESCREEN_STATE_FADE_OUT)
        {       

            // "DEBUG VERSION"
            TextOperation(play, NULL, &gfx, COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                          255, 255, 
                          sDebugVersion, 
                          GetStringCenterX(sDebugVersion, TEXT_SCALE_TITLE), 
                          10, 1, 1, 
                          NULL, TEXT_SCALE_TITLE, TEXT_SCALE_TITLE, 0, false, OPERATION_DRAW_SHADOW); 
            
            sprintf(BUILDUSERSTRING, "BUILD %s %s", __DATE__, __TIME__);
            
            // Build date and time
            TextOperation(play, NULL, &gfx, COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                          255, 255, 
                          BUILDUSERSTRING, 
                          GetStringCenterX(BUILDUSERSTRING, TEXT_SCALE_TITLE), 
                          220, 1, 1, 
                          NULL, TEXT_SCALE_TITLE, TEXT_SCALE_TITLE, 0, false, OPERATION_DRAW_SHADOW); 

            // Scene list
            for (int i = 0; i < ARRAY_COUNT(sScenes); i++)
            {
                TextOperation(play, NULL, &gfx, 
                              i == this->selectedScene ? COLOR_HIGHLIGHT : COLOR_NO_HIGHLIGHT, 
                              COLOR_BLACK, 
                              255, 255, 
                              sScenes[i].name, 
                              GetStringCenterX(sScenes[i].name, TEXT_SCALE_TITLE), 
                              85 + (i * 16), 1, 1, 
                              NULL, TEXT_SCALE_TITLE, TEXT_SCALE_TITLE, 0, false, OPERATION_DRAW_SHADOW); 
            }

            // "[L] Language
            TextOperation(play, NULL, &gfx, 
                          COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                          255, 255, 
                          languageStrings[LANG_INDEX], 
                          GetStringCenterX(languageStrings[LANG_INDEX], TEXT_SCALE_TITLE),
                          200, 1, 1, 
                          NULL, TEXT_SCALE_TITLE, TEXT_SCALE_TITLE, 0, false, OPERATION_DRAW_SHADOW); 

                                    
        }

    #else
        
        if (this->mainAlpha > 0)
        {
            // Version string
            int pxWidth = GetTextPxWidth(StringVERSION, TEXT_SCALE_OPTIONS);
            
            TextOperation(play, NULL, &gfx, 
                          COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                          this->mainAlpha, this->mainAlpha, 
                          StringVERSION, 
                          320 - pxWidth - 15 + (SAVE_WIDESCREEN ? 50 : 0), 
                          220, 1, 1, 
                          NULL, TEXT_SCALE_OPTIONS, TEXT_SCALE_OPTIONS, 0, false, OPERATION_DRAW_SHADOW);  
                               
                                       
            if (this->globalState == TITLESCREEN_STATE_DISPLAY)
            {
                // Settings [R] string
                char* settings = StringSETTINGS[LANG_INDEX];  
                int pxWidth = GetTextPxWidth(settings, TEXT_SCALE_OPTIONS);
                
                TextOperation(play, NULL, &gfx, 
                              COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                              this->mainAlpha, this->mainAlpha, 
                              settings, 
                              320 - pxWidth - 15 + (SAVE_WIDESCREEN ? 50 : 0), 
                              10, 1, 1, 
                              NULL, TEXT_SCALE_OPTIONS, TEXT_SCALE_OPTIONS, 0, false, OPERATION_DRAW_SHADOW);   
                            
           
                // [L] Language string
                #ifdef LANGUAGE_PICKER        
                    char* language = languageStrings[LANG_INDEX];               
                    TextOperation(play, NULL, &gfx, 
                                  COLOR_NO_HIGHLIGHT, COLOR_BLACK, 
                                  this->mainAlpha, this->mainAlpha, 
                                  language, 
                                  15 - (SAVE_WIDESCREEN ? 50 : 0), 
                                  10, 1, 1, 
                                  NULL, TEXT_SCALE_OPTIONS, TEXT_SCALE_OPTIONS, 0, false, OPERATION_DRAW_SHADOW);     

                #endif
                                   
                                   
            }
        }


        if (this->globalState >= TITLESCREEN_STATE_DISPLAY)
        {         
            const int BASE_YPOS = 170;
            const int SCENE_SELECT_YPOS = 186;
            
            int sceneSelectOffs = ((SAVE_HASBEATENGAME || SAVE_EXTRASCENES) ? 12 : 0);
            
            char* ngString = NewGameString[LANG_INDEX];
            char* ssString = SceneSelectString[LANG_INDEX];
            char* contString = ContinueString[LANG_INDEX];

            // No file, display only New Game and (if unlocked) Scene Select
            if (!this->hasFile)
            {
                // "New Game"
                int xScale = GetTextScaleToFitX(ngString, TEXT_SCALE_TITLE, 240); 
                int xPos = GetStringCenterX(ngString, xScale); 

                if (SAVE_WIDESCREEN)
                {
                    xScale *= WIDESCREEN_SCALEX;
                    xPos *= WIDESCREEN_SCALEX;
                    xPos += WIDESCREEN_OFFSX;
                }            
                
                bool isNgHighlighted = (this->highlightedOption == 0);
                TextOperation(play, NULL, &gfx, 
                                        isNgHighlighted ? COLOR_HIGHLIGHT : COLOR_NO_HIGHLIGHT, 
                                        COLOR_BLACK, 
                                        isNgHighlighted ? this->textAlpha : this->textAlphaFull, 
                                        isNgHighlighted ? this->textAlpha : this->textAlphaFull, 
                                        ngString, xPos, BASE_YPOS - sceneSelectOffs, 1, 1, 
                                        NULL, xScale, TEXT_SCALE_TITLE, 0, true, OPERATION_DRAW_SHADOW);
                                    
                // "Scene Select / Extras"
                if (SAVE_EXTRASCENES)
                {
                    xScale = GetTextScaleToFitX(ngString, TEXT_SCALE_TITLE, 240);
                    xPos = GetStringCenterX(ssString, xScale);     

                    if (SAVE_WIDESCREEN)
                    {
                        xScale *= WIDESCREEN_SCALEX;
                        xPos *= WIDESCREEN_SCALEX;
                        xPos += WIDESCREEN_OFFSX;
                    }                   
                    
                    bool isSsHighlighted = (this->highlightedOption == 2);
                    TextOperation(play, NULL, &gfx, 
                                  isSsHighlighted ? COLOR_HIGHLIGHT : this->sceneSelectRed ? COLOR_RED : COLOR_NO_HIGHLIGHT, 
                                  COLOR_BLACK, 
                                  isSsHighlighted ? this->textAlpha : this->textAlphaFull, 
                                  isSsHighlighted ? this->textAlpha : this->textAlphaFull, 
                                  ssString, xPos, SCENE_SELECT_YPOS - sceneSelectOffs, 1, 1, 
                                  NULL, xScale, TEXT_SCALE_TITLE, 0, true, OPERATION_DRAW_SHADOW);        
                }                                 
            }
            else
            {
                const int SS_STRING_MAX_WIDTH = 240;
                const int OPTION_MAX_WIDTH = 105;
                const int OPTION_SPACING = 10;
                
                // Calculate base for the scene select text (used as reference for the other positions)
                int xScaleSS = GetTextScaleToFitX(ssString, TEXT_SCALE_TITLE, SS_STRING_MAX_WIDTH);
                int xStartPosSS = GetStringCenterX(ssString, xScaleSS);
                int xEndPosSS = xStartPosSS + GetTextPxWidth(ssString, xScaleSS);
                
                // "New Game"
                int ngScale = GetTextScaleToFitX(ngString, TEXT_SCALE_TITLE, OPTION_MAX_WIDTH);
                int ngPos = xStartPosSS - OPTION_SPACING;
                
                if (SAVE_WIDESCREEN) 
                {
                    ngScale *= WIDESCREEN_SCALEX;
                    ngPos = (ngPos * WIDESCREEN_SCALEX) + WIDESCREEN_OFFSX;
                }
                
                bool isNgHighlighted = (this->highlightedOption == 0);
                TextOperation(play, NULL, &gfx,
                              isNgHighlighted ? COLOR_HIGHLIGHT : COLOR_NO_HIGHLIGHT, 
                              COLOR_BLACK,
                              isNgHighlighted ? this->textAlpha : this->textAlphaFull,
                              isNgHighlighted ? this->textAlpha : this->textAlphaFull,
                              ngString, ngPos, BASE_YPOS - sceneSelectOffs, 1, 1,
                              NULL, ngScale, TEXT_SCALE_TITLE, 0, true, OPERATION_DRAW_SHADOW);
                
                // "Continue"
                int contScale = GetTextScaleToFitX(contString, TEXT_SCALE_TITLE, OPTION_MAX_WIDTH);
                int contPos = xEndPosSS + OPTION_SPACING - GetTextPxWidth(contString, contScale);
                
                if (SAVE_WIDESCREEN) 
                {
                    contScale *= WIDESCREEN_SCALEX;
                    contPos = (contPos * WIDESCREEN_SCALEX) + WIDESCREEN_OFFSX;
                }
                
                bool isContHighlighted = (this->highlightedOption == 1);
                TextOperation(play, NULL, &gfx,
                              isContHighlighted ? COLOR_HIGHLIGHT : COLOR_NO_HIGHLIGHT,
                              COLOR_BLACK,
                              isContHighlighted ? this->textAlpha : this->textAlphaFull,
                              isContHighlighted ? this->textAlpha : this->textAlphaFull,
                              contString, contPos, BASE_YPOS - sceneSelectOffs, 1, 1,
                              NULL, contScale, TEXT_SCALE_TITLE, 0, true, OPERATION_DRAW_SHADOW);
                
                // "Scene Select / Extras"
                if (SAVE_HASBEATENGAME || SAVE_EXTRASCENES) 
                {
                    int ssScale = xScaleSS;
                    int ssPos = GetStringCenterX(ssString, xScaleSS);
                    
                    if (SAVE_WIDESCREEN) 
                    {
                        ssScale *= WIDESCREEN_SCALEX;
                        ssPos = (ssPos * WIDESCREEN_SCALEX) + WIDESCREEN_OFFSX;
                    }
                    
                    bool isSsHighlighted = (this->highlightedOption == 2);
                    Color_RGB8 ssColor = isSsHighlighted ? COLOR_HIGHLIGHT : (this->sceneSelectRed ? COLOR_RED : COLOR_NO_HIGHLIGHT);
                    
                    TextOperation(play, NULL, &gfx,
                                  ssColor, COLOR_BLACK,
                                  isSsHighlighted ? this->textAlpha : this->textAlphaFull,
                                  isSsHighlighted ? this->textAlpha : this->textAlphaFull,
                                  ssString, ssPos, SCENE_SELECT_YPOS - sceneSelectOffs, 1, 1,
                                  NULL, ssScale, TEXT_SCALE_TITLE, 0, true, OPERATION_DRAW_SHADOW);
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
                    this->stringSlot += 6 * LANG_INDEX;
                    
                    this->gbCamEasterEggAlpha = 255;
                    Audio_PlaySfxGeneral(NA_SE_EN_GANON_LAUGH, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, 
                                        &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                }
                
                if (this->drawGbCamEasterEgg < 50)
                    Math_ApproachS(&this->gbCamEasterEggAlpha, 0, 1, 5);
                
                if (this->gbCamEasterEggAlpha > 0)
                {
                    Draw2DInternal(IA8, (u8*)in->gbCameraPic, NULL, &gfx, 160, 120, 128, 112, 320, 240, this->gbCamEasterEggAlpha);        
                    TextOperation(play, NULL, &gfx, 
                                  COLOR_RED, COLOR_BLACK, 
                                  this->gbCamEasterEggAlpha, this->gbCamEasterEggAlpha, 
                                  gbCamEasterEggStrings[this->stringSlot], 
                                  GetStringCenterX(gbCamEasterEggStrings[this->stringSlot], 140), 
                                  48, 1, 1, NULL, 140, 140, 0, false, OPERATION_DRAW_SHADOW);          
                }                                
            }
            else
                this->drawGbCamEasterEgg = 1;
            
            this->drawGbCamEasterEgg--;
        }
    }


    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;

}

void TitleLogo_Draw_Settings(Actor* thisx, PlayState* play)
{
}

void TitleLogo_DrawSceneSelect(Actor* thisx, PlayState* play)
{
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

void HaltCutscene(PlayState* play)
{
    play->csCtx.frames = 30;
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

void SetTrsPakDisableStatus(Actor* thisx, bool status)
{
    TitleLogo* this = THIS;
    
    if (this->trsPak != NULL)
        this->trsPak->disabled = status;    
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