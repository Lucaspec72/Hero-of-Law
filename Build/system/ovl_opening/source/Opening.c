#include "../include/Opening.h"

void Opening_SetupTitleScreen(TitleSetupState* this) 
{
    gSaveContext.gameMode = 1;
    this->state.running = false;
    gSaveContext.linkAge = 0;
    
    // Preserve language
    int Language = SAVE_LANGUAGE;
    Sram_InitDefSave();
    SAVE_LANGUAGE = Language;
    
    gSaveContext.cutsceneIndex = 0xFFF3;
    gSaveContext.sceneLayer = 7;
    SET_NEXT_GAMESTATE(&this->state, (void*)0x8009A750, PlayState);
}

void SetupRuntimeFont(GameState* thisx)
{
    RuntimeFont* f = (RuntimeFont*)&that->font->charTexBuf;
    
    if (f->numUsed != 0 || f->usedChar[0] != 0xFF)
    {
        Lib_MemSet(&f->usedChar[0], RUNTIME_CHAR_COUNT, 0xFF);
        f->numUsed = 0;       
    }
}

void Opening_Main(GameState* thisx) 
{
    TitleSetupState* this = (TitleSetupState*)thisx;
    GraphicsContext* __gfxCtx = this->state.gfxCtx;
    
    Gfx* gfxRef = POLY_OPA_DISP;
    Gfx* gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);
    Gfx_SetupFrame(this->state.gfxCtx, 0, 0, 0);
    
    SetupRuntimeFont(thisx);   
    
    Environment_FillScreen(this->state.gfxCtx, 255, 255, 255, (s16)255, FILL_SCREEN_XLU);     
 
    if (that->controllerInfoState != STATE_EXIT)
    {
        that->timer++;
        
        if (that->timer == 320 && that->controllerInfoState == STATE_FADEIN_GRAPHICS)
            that->controllerInfoState = STATE_FADEOUT_GRAPHICS;
       

        if (that->controllerInfoState == STATE_INIT)
        {
            that->controllerInfoState = STATE_FADEIN_WHITE;
        }

        if (that->controllerInfoState == STATE_FADEIN_WHITE)
        {
            if (that->fade > 0)
                that->fade -= 15;
            else 
            {
                that->fade = 255;
                that->controllerInfoState = STATE_FADEIN_GRAPHICS;
            }
        }

        if (that->controllerInfoState == STATE_FADEIN_GRAPHICS)
        {
            if (that->fade == 0 && CHECK_BTN_ALL(this->state.input->press.button, BTN_A))
                that->controllerInfoState = STATE_FADEOUT_GRAPHICS;

            if (that->fade > 0)
                that->fade -= 5;
        }

        if (that->controllerInfoState == STATE_FADEOUT_GRAPHICS)
        {
            if (that->fade < 255)
                that->fade += 5;
            else
            {
                that->fade = 0;
                that->controllerInfoState = STATE_FADEOUT_WHITE;
            }
        }

        if (that->controllerInfoState == STATE_FADEOUT_WHITE)
        {
            if (that->fade < 255)
                that->fade += 15;
            else 
                that->controllerInfoState = STATE_EXIT;
        }  

        if (that->controllerInfoState == STATE_FADEIN_GRAPHICS || that->controllerInfoState == STATE_FADEOUT_GRAPHICS)
        {
            Draw2DInternal(IA4_Setup39, (u8*)that->controllerInfoGfx, NULL, &gfx, SCREEN_WIDTH / 2, 90, CONTROLLER_GRAPHIC_X, CONTROLLER_GRAPHIC_Y, CONTROLLER_GRAPHIC_X, CONTROLLER_GRAPHIC_Y, 255 - that->fade); 
            
            TextOperation(NULL, that->font, &gfx, 
                          controllerInfoFontColor, controllerInfoShadowColor, 
                          255 - that->fade, 255 - that->fade, 
                          controllerInfoLines[LANG_INDEX], 
                          0, 186, 
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
        }
    }
    else if (that->controllerInfoState == STATE_EXIT)
        Opening_SetupTitleScreen(this);
    
    if (that->controllerInfoState == STATE_FADEIN_WHITE || that->controllerInfoState == STATE_FADEOUT_WHITE || that->controllerInfoState == STATE_EXIT)
        Environment_FillScreen(this->state.gfxCtx, 0, 0, 0, (s16)that->fade, FILL_SCREEN_XLU);
    
    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;
    
    Screen_Adjust(&this->state, &this->view);
}


void Opening_Destroy(GameState* thisx) 
{
}

void ConsoleLogo_SetupView(TitleSetupState* this, f32 x, f32 y, f32 z)
{
    View* view = &this->view;
    Vec3f eye;
    Vec3f lookAt;
    Vec3f up;

    eye.x = x;
    eye.y = y;
    eye.z = z;
    up.x = up.z = 0.0f;
    lookAt.x = lookAt.y = lookAt.z = 0.0f;
    up.y = 1.0f;

    View_SetPerspective(view, 30.0f, 10.0f, 12800.0f);
    View_LookAt(view, &eye, &lookAt, &up);
    View_Apply(view, VIEW_ALL);  
}

void ConsoleLogo_Calc(TitleSetupState* this)
{
    if ((that->coverAlpha == 0) && (that->visibleDuration != 0)) 
    {
        that->visibleDuration--;
    } 
    else 
    {
        that->coverAlpha += that->addAlpha;
        if (that->coverAlpha <= 0) 
        {
            that->coverAlpha = 0;
            that->addAlpha = 3;
        } 
        else if (that->coverAlpha >= 0xFF) 
        {
            that->coverAlpha = 0xFF;
        }
    }
    that->uls = that->ult & 0x7F;
    that->ult++;
}

void ConsoleLogo_Draw(TitleSetupState* this) 
{
    GraphicsContext* __gfxCtx = this->state.gfxCtx;    

    u16 y;
    u16 idx;
    static Lights1 sTitleLights = gdSPDefLights1(100, 100, 100, 255, 255, 255, 69, 69, 69);
    
    Vec3f v1 = {0, 0, 0};
    Vec3f v2 = {-4949.148, 4002.5417, 1119.0837};
    Vec3f v3 = {69, 69, 69};
   
    Gfx_SetupDL_39Opa(this->state.gfxCtx);
    gDPPipeSync(POLY_OPA_DISP++);
    gDPSetCycleType(POLY_OPA_DISP++, G_CYC_2CYCLE);
    gDPSetRenderMode(POLY_OPA_DISP++, G_RM_PASS, G_RM_CLD_SURF2);
    gDPSetCombineLERP(POLY_OPA_DISP++, TEXEL1, PRIMITIVE, ENV_ALPHA, TEXEL0, 0, 0, 0, TEXEL0, PRIMITIVE, ENVIRONMENT,
                      COMBINED, ENVIRONMENT, COMBINED, 0, PRIMITIVE, 0);
    
    gDPSetPrimColor(POLY_OPA_DISP++, 0, 0, that->n64TextColorPrim.r, that->n64TextColorPrim.g, that->n64TextColorPrim.b, that->n64TextColorPrim.a);
    gDPSetEnvColor(POLY_OPA_DISP++, that->n64TextColorEnv.r, that->n64TextColorEnv.g, that->n64TextColorEnv.b, that->n64TextColorEnv.a);
    
    gDPLoadMultiBlock(POLY_OPA_DISP++, N64_LOGO_SHEEN, 0x100, 1, G_IM_FMT_I, G_IM_SIZ_8b, 32, 32, 0,
                      G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, 5, 5, 2, 11);

    if (that->logoState >= LOGOSTATE_MOVE)
        gDPSetScissor(POLY_OPA_DISP++, G_SC_NON_INTERLACE, (that->logoPosMoveVec.x + 100 )* 1.1, 0, 360, 240);
    
    float scaleX = 1 << 10;
    
    if (SAVE_WIDESCREEN)
        scaleX /= WIDESCREEN_SCALEX;

    for (idx = 0, y = 94 - that->logoPosOffs.y; idx < 16; idx++, y += 2) 
    {
        gDPLoadTextureBlock(POLY_OPA_DISP++, &(N64_LOGO_TEXT)[0x180 * idx], G_IM_FMT_I,
                            G_IM_SIZ_8b, 192, 2, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK,
                            G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);

        gDPSetTileSize(POLY_OPA_DISP++, 1, that->ult, (that->uls & 0x7F) - (14 << 2), 0, 0);
        gSPTextureRectangle(POLY_OPA_DISP++, ((SAVE_WIDESCREEN ? 20 : 0) + 97 + (int)that->logoPosOffs.x) << 2, y << 2, (289 + (int)that->logoPosOffs.y) << 2, (y + 2) << 2, G_TX_RENDERTILE, 0, 0, (s32)scaleX,
                            1 << 10);
    }
    
    
    func_8002EABC(&v1, &v2, &v3, this->state.gfxCtx);
    gSPSetLights1(POLY_OPA_DISP++, sTitleLights);   
    
    ConsoleLogo_SetupView(this, 0, 150.0, 300.0);
    Gfx_SetupDL_25Opa(this->state.gfxCtx);
    Matrix_Translate(-53.0 + that->logoPosOffs.x + that->logoPosMoveVec.x, -5.0 + (that->logoPosOffs.y * 0.75) + that->logoPosMoveVec.y, that->logoPosOffs.z + that->logoPosMoveVec.z, MTXMODE_NEW);
    Matrix_Scale(that->logoScale, that->logoScale, that->logoScale, MTXMODE_APPLY);
    Matrix_RotateZYX(0, that->logoRot, 0, MTXMODE_APPLY);


    gSPMatrix(POLY_OPA_DISP++, Matrix_NewMtx(this->state.gfxCtx, __FILE__, __LINE__), G_MTX_LOAD);
    gSPDisplayList(POLY_OPA_DISP++, N64_LOGO_DLIST);    
    
}

void RudimentaryColorInterpolate(Color_RGBA8 color1, Color_RGBA8 color2, float fraction, Color_RGBA8 *colorOut) 
{
    colorOut->r = (u8)(color1.r + (color2.r - color1.r) * fraction + 0.5f);
    colorOut->g = (u8)(color1.g + (color2.g - color1.g) * fraction + 0.5f);
    colorOut->b = (u8)(color1.b + (color2.b - color1.b) * fraction + 0.5f);
    colorOut->a = (u8)(color1.a + (color2.a - color1.a) * fraction + 0.5f);
}

void GetHSVSpectrumColor(GameState* thisx, float hue, Color_RGBA8 *colorOut, int alpha) 
{
    float s = 1.0f;
    float v = 1.0f;
    
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(hue / 60.0f, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (hue < 60)
    {
        r = c; g = x; b = 0;
    }
    else if (hue < 120)
    {
        r = x; g = c; b = 0;
    }
    else if (hue < 180)
    {
        r = 0; g = c; b = x;
    }
    else if (hue < 240)
    {
        r = 0; g = x; b = c;
    }
    else if (hue < 300)
    {
        r = x; g = 0; b = c;
    }
    else
    {
        r = c; g = 0; b = x;
    }
    
    colorOut->r = (u8)((r + m) * 255 + 0.5f);
    colorOut->g = (u8)((g + m) * 255 + 0.5f);
    colorOut->b = (u8)((b + m) * 255 + 0.5f);
    colorOut->a = alpha;
}

void ConsoleLogo_Main(GameState* thisx) 
{
    TitleSetupState* this = (TitleSetupState*)thisx;
    GraphicsContext* __gfxCtx = this->state.gfxCtx;

    gSPSegment(POLY_OPA_DISP++, 0, NULL);
    gSPSegment(POLY_OPA_DISP++, 1, that->segBuf);
    gSPSegment(POLY_XLU_DISP++, 1, that->segBuf);

    Gfx_SetupFrame(this->state.gfxCtx, 0, 0, 0);
    
    SetupRuntimeFont(thisx);       
    
    that->logoRot += that->spinSpeed;
    ConsoleLogo_Calc(this);
    ConsoleLogo_Draw(this);
    
    if (gSaveContext.fileNum == 0xFEDC)
        that->noController = true;
    
    if (that->logoState == LOGOSTATE_PREINIT)
    {
        int res = LoadSaveAndVerify(SAVE_SLOT);
        int res2 = SAVE_NOT_HOL;
        
        if (res)
            res2 = LoadSaveAndVerify(SAVE_SLOT_BACKUP); 
        
        if (res == SAVE_OK || res2 == SAVE_OK)
        {
        }
        else
        {
            SAVE_SCREENXPOS = 0;
            SAVE_SCREENYPOS = 0;
            SAVE_SCREENSIZEX = 225;
            SAVE_SCREENSIZEY = 225;
            SAVE_WIDESCREEN = 0;
            SAVE_LANGUAGE = 0;
            that->langChoice = true;
        }
        
#ifndef LANGUAGE_PICKER
        SAVE_LANGUAGE = 0;
#endif
        
        // Have to load the font here, so that SAVE_LANGUAGE is properly loaded.
        Font_LoadFont(that->font);
        that->logoState = LOGOSTATE_INIT;
    }
    
    #if DEBUGVER == 1
        if (that->logoState == LOGOSTATE_INIT && !CHECK_BTN_ALL(this->state.input->cur.button, BTN_START) && *memSize == 0x800000)
        {
            that->controllerInfoState = STATE_EXIT;
            that->logoState = LOGOSTATE_DONE;
        }
    #endif
    
    switch (that->logoState)
    {
        case LOGOSTATE_INIT:
        {
#ifdef LANGUAGE_PICKER
            if ((that->langChoice && that->visibleDuration < 200) || CHECK_BTN_ALL(this->state.input->cur.button, BTN_START))
            {
                that->logoState = LOGOSTATE_PICK_LANGUAGE_MOVE_UP;
                break;
            }
#endif
            if (*memSize < 0x800000 || that->noController)
            {           
                if (that->visibleDuration < 165)
                {
                    RudimentaryColorInterpolate(that->n64TextColorPrim, primRed, that->colorInterpolationFraction, &that->n64TextColorPrim);
                    RudimentaryColorInterpolate(that->n64TextColorEnv, envRed, that->colorInterpolationFraction, &that->n64TextColorEnv);
                    
                    that->colorInterpolationFraction += 0.01;
                    
                    if (that->colorInterpolationFraction > 1)
                        that->colorInterpolationFraction = 1;
                }
        
                if(that->visibleDuration == 100)
                {
                    that->visibleDuration = 0xFFFF;
                    Audio_PlaySfxGeneral(NA_SE_SY_ERROR, &gSfxDefaultPos, 4, &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale, &gSfxDefaultReverb);
                    
                    if (that->noController)
                        that->logoState = LOGOSTATE_NO_CONTROLLER_MOVE_UP;
                    else
                        that->logoState = LOGOSTATE_MEMORY_MISSING_MOVE_UP;
                }
            }
#ifdef HZSWITCH            
            else if (osTvType == OS_TV_PAL || that->hzChoiceForce)
            {
                if (that->visibleDuration < 165)
                {
                    that->hzChoiceTimer = 1000;
                    that->logoState = LOGOSTATE_PICK6050HZ_MOVE_UP;
                }
            }
#endif
            else
            {
                if (that->visibleDuration < 150)
                    that->logoState = LOGOSTATE_SPEEDUP;
            }
            
            break;
        }
        case LOGOSTATE_PICK_LANGUAGE_MOVE_UP:
        case LOGOSTATE_NO_CONTROLLER_MOVE_UP:
        case LOGOSTATE_MEMORY_MISSING_MOVE_UP:
        case LOGOSTATE_PICK6050HZ_MOVE_UP:
        {
            if (that->logoPosOffs.y != 30)
                that->logoPosOffs.y += 1;
            else
            {
                switch (that->logoState)
                {
                    case LOGOSTATE_NO_CONTROLLER_MOVE_UP: 
                        that->logoState = LOGOSTATE_NO_CONTROLLER; break;
                    case LOGOSTATE_MEMORY_MISSING_MOVE_UP: 
                        that->logoState = LOGOSTATE_MEMORY_MISSING; break;
                    case LOGOSTATE_PICK6050HZ_MOVE_UP: 
                        that->logoState = LOGOSTATE_PICK6050HZ; break;     
                    case LOGOSTATE_PICK_LANGUAGE_MOVE_UP:
                        that->logoState = LOGOSTATE_PICK_LANGUAGE; break;
                }
            }
            
            break;
        }
        case LOGOSTATE_PICK_LANGUAGE:
        {
            Gfx* gfx = POLY_OPA_DISP;
            Gfx_SetupDL_39Ptr(&gfx);
            
            if (that->colorInterpolationFraction < 1)
            {
                that->hue = 240;
                GetHSVSpectrumColor(thisx, that->hue, &that->n64TextColorTemp, 128);
                RudimentaryColorInterpolate(that->n64TextColorEnv, that->n64TextColorTemp, that->colorInterpolationFraction, &that->n64TextColorEnv);  

                that->colorInterpolationFraction += 0.01;
                
                if (that->colorInterpolationFraction > 1)
                    that->colorInterpolationFraction = 1;            
            }
            else
            {
                GetHSVSpectrumColor(thisx, that->hue, &that->n64TextColorEnv, 128);
                that->hue += 1;

                if (that->hue > 360)
                    that->hue = 0;             
            }
            
            
            if (!that->fontReloadTimer)
            {
                // Pick language explanation
                TextOperation(NULL, that->font, &gfx, 
                              langSelectFontColor, langSelectShadowFontColor, 
                              255, 255, 
                              pickLanguageLines[LANG_INDEX], 
                              0, 150, 
                              1, 1, 
                              NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
                              
                // Cursor
                TextOperation(NULL, that->font, &gfx, 
                              langSelectFontColor, langSelectShadowFontColor, 
                              255, 255, 
                              languageCursor, 
                              0, 180, 
                              1, 1, 
                              NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);  

                // Language name
                TextOperation(NULL, that->font, &gfx, 
                              langSelectFontColor, langSelectShadowFontColor, 
                              255, 255, 
                              languageNames[LANG_INDEX], 
                              0, 180, 
                              1, 1, 
                              NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);                                        
            }
            else
            {
                // If we're on the first frame of the font reload timer, reload the font.
                // This is so that we know the message has disappeared.
                if (that->fontReloadTimer == LANGUAGE_PICKER_FONT_LOAD_COOLDOWN - 1)
                {
                    // The message log also needs to be erased when the language is changed.
                    InvalidateMsgLogChecksum();
                    Font_LoadFont(that->font); 
                }                    
                
                that->fontReloadTimer--;
            }
                
            if (!that->langChoiceCoold && !that->fontReloadTimer)
            {
                if (CHECK_BTN_ALL(this->state.input->press.button, BTN_DRIGHT) || this->state.input->cur.stick_x > 20)
                {
                    if (SAVE_LANGUAGE < HOL_LANGUAGE_MAX - 1)
                        SAVE_LANGUAGE++;
                    else
                        SAVE_LANGUAGE = 0;
                    
                    that->fontReloadTimer = LANGUAGE_PICKER_FONT_LOAD_COOLDOWN;                    
                }    
                else if (CHECK_BTN_ALL(this->state.input->press.button, BTN_DLEFT) || this->state.input->cur.stick_x < -20)
                {
                    if (SAVE_LANGUAGE > 0)
                        SAVE_LANGUAGE--;
                    else
                        SAVE_LANGUAGE = HOL_LANGUAGE_MAX - 1;
                    
                    that->fontReloadTimer = LANGUAGE_PICKER_FONT_LOAD_COOLDOWN;
                }
            }
            else
                that->langChoiceCoold--;
            
            if (!CHECK_BTN_ALL(this->state.input->press.button, BTN_DRIGHT | BTN_DLEFT) && ABS(this->state.input->cur.stick_x) < 20)
                that->langChoiceCoold = 0;
            else
                that->langChoiceCoold = LANGUAGE_PICKER_INPUT_COOLDOWN;
            
            if (CHECK_BTN_ALL(this->state.input->press.button, BTN_A))
            {
                that->langChoice = false;
                that->logoState =  LOGOSTATE_PICK_LANGUAGE_MOVE_DOWN;
                that->colorInterpolationFraction = 0;
            }

            that->visibleDuration = 0xFFFF;
            POLY_OPA_DISP = gfx;          
            
            break;              
        }
        case LOGOSTATE_NO_CONTROLLER:
        {
            Gfx* gfx = POLY_OPA_DISP;
            Gfx_SetupDL_39Ptr(&gfx);  

            TextOperation(NULL, that->font, &gfx, 
                          noExpPakFontColor, noExpPakShadowColor, 
                          255, 255, 
                          noControllerLines[LANG_INDEX], 
                          0, 130, 
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
            
            that->visibleDuration = 0xFFFF;
            POLY_OPA_DISP = gfx;          
            
            break;            
        }        
        case LOGOSTATE_MEMORY_MISSING:
        {
            Gfx* gfx = POLY_OPA_DISP;
            Gfx_SetupDL_39Ptr(&gfx);  
            
            TextOperation(NULL, that->font, &gfx, 
                          noExpPakFontColor, noExpPakShadowColor, 
                          255, 255, 
                          noExpPakLines[LANG_INDEX], 
                          0, 130, 
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);            
            
            that->visibleDuration = 0xFFFF;
            POLY_OPA_DISP = gfx;          
            
            break;            
        }
        case LOGOSTATE_PICK6050HZ:
        {
            Gfx* gfx = POLY_OPA_DISP;
            Gfx_SetupDL_39Ptr(&gfx);    

            if (that->hzChoiceMade == false)
            {
                Math_ApproachS(&that->hzChoiceAlpha, 255, 1, 15);
                
                if (that->hzChoiceAlpha == 255)
                {
                    bool inputMade = true;
                    
                    if (CHECK_BTN_ALL(this->state.input->press.button, BTN_DRIGHT) || this->state.input->cur.stick_x > 20)
                        that->hzChoice = 1;
                    else if (CHECK_BTN_ALL(this->state.input->press.button, BTN_DLEFT) || this->state.input->cur.stick_x < -20)
                        that->hzChoice = 0;    
                    else if (CHECK_BTN_ALL(this->state.input->press.button, BTN_A) || that->hzChoiceTimer == 0)
                    {
                        that->hzChoiceForce = false;
                        that->hzChoiceMade = true;
                        *hzChoice = that->hzChoice;
                        
                        if (*hzChoice == 0)
                        {
                            gAudioContext.unk_2960 = 1000 * REFRESH_RATE_DEVIATION_NTSC / REFRESH_RATE_NTSC;
                            gAudioContext.refreshRate = REFRESH_RATE_NTSC;  
                            osViClock = VI_NTSC_CLOCK;
                        }
                        else
                        {
                            gAudioContext.unk_2960 = 1000 * REFRESH_RATE_DEVIATION_PAL / REFRESH_RATE_PAL;
                            gAudioContext.refreshRate = REFRESH_RATE_PAL; 
                            osViClock = VI_PAL_CLOCK;
                        }
                        
                        SEQCMD_RESET_AUDIO_HEAP(0, 0);
          
                        that->logoState =  LOGOSTATE_PICK6050HZ_MOVE_DOWN;
                    }    
                    else
                    {
                        inputMade = false;
                        that->hzChoiceTimer--;
                    }
                    
                    if (inputMade)
                        that->hzChoiceTimer = 1000;
                }                
            }
            else
                Math_ApproachS(&that->hzChoiceAlpha, 0, 1, 15);
            
            // Explanation text
            TextOperation(NULL, that->font, &gfx, 
                          hzSwitchFontColor, hzSwitchShadowFontColor, 
                          that->hzChoiceAlpha, that->hzChoiceAlpha, 
                          pickHzLines[LANG_INDEX], 
                          0, 130, 
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
                          
            // 60Hz
            TextOperation(NULL, that->font, &gfx, 
                          hzSwitchFontColor, hzSwitchShadowFontColor, 
                          that->hzChoiceAlpha, that->hzChoiceAlpha, 
                          hzOptions1[LANG_INDEX], 
                          90, 192,
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);   
                          
            // 50Hz
            TextOperation(NULL, that->font, &gfx, 
                          hzSwitchFontColor, hzSwitchShadowFontColor, 
                          that->hzChoiceAlpha, that->hzChoiceAlpha, 
                          hzOptions2[LANG_INDEX], 
                          200, 192,
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);                           
                          
            // Cursor
            TextOperation(NULL, that->font, &gfx, 
                          hzSwitchFontColor, hzSwitchShadowFontColor, 
                          that->hzChoiceAlpha, that->hzChoiceAlpha, 
                          hzCursor, 
                          80 + (that->hzChoice * 110), 192,
                          1, 1, 
                          NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);                          
            
            that->visibleDuration = 0xFFFF;
            
            POLY_OPA_DISP = gfx; 
            
            break;
        }
        case LOGOSTATE_PICK_LANGUAGE_MOVE_DOWN:
        case LOGOSTATE_PICK6050HZ_MOVE_DOWN:
        {
            if (that->logoPosOffs.y != 0)
                that->logoPosOffs.y -= 2;
            else
            {
                that->visibleDuration = 200;
                that->logoState = LOGOSTATE_INIT;
            }
            
            break;
        }
        case LOGOSTATE_SPEEDUP:
        {
            RudimentaryColorInterpolate(that->n64TextColorPrim, primGold, that->colorInterpolationFraction, &that->n64TextColorPrim);
            RudimentaryColorInterpolate(that->n64TextColorEnv, envGold, that->colorInterpolationFraction, &that->n64TextColorEnv);
            
            that->colorInterpolationFraction += 0.01;
                
            if (that->colorInterpolationFraction > 1)
                that->colorInterpolationFraction = 1;                
            
            
            if (that->spinSpeed != 2000)
                that->spinSpeed += 20;
            else
            {
                that->visibleDuration = 200;
                that->logoState = LOGOSTATE_MOVE;
                that->logoPosMoveDiff = -2.5;
            }
            
            break;
        }
        case LOGOSTATE_MOVE:
        {
            that->logoPosMoveDiff += 0.2;
            that->logoPosMoveVec.x += that->logoPosMoveDiff;
            
            if (that->logoPosMoveVec.x >= 250)
                that->logoState = LOGOSTATE_DONE;
            
            break;
        }
        case LOGOSTATE_DONE:
        {
            that->fade = 255;
            this->state.main = &Opening_Main;
        }
      
    }

    Environment_FillScreen(this->state.gfxCtx, 0, 0, 0, (s16)that->coverAlpha, FILL_SCREEN_XLU);
    Screen_Adjust(&this->state, &this->view);      

    return;    
}

#ifdef MUSID
    #pragma GCC optimize ("O1")


    #define SEQCMD_SET_TEMPO(seqPlayerIndex, duration, tempoTarget)\
        Audio_QueueSeqCmd((0xB << 28) | (0x0 << 12) | ((u8)(seqPlayerIndex) << 24) | \
                          ((u8)(duration) << 16) | (u16)(tempoTarget))

    #define SEQCMD_RESET_TEMPO(seqPlayerIndex, duration)\
        Audio_QueueSeqCmd((0xB << 28) | (0x4 << 12) | ((u8)(seqPlayerIndex) << 24) | \
                          ((u8)(duration) << 16))

    #define SEQCMD_SET_SEQPLAYER_FREQ(seqPlayerIndex, duration, freqScale)                                           \
        Audio_QueueSeqCmd((0xD << 28) | ((u8)(seqPlayerIndex) << 24) | ((duration) << 16) | \
                          (freqScale))
                          
    void SetMusicTempo(int tempo, int frames, int freq)
    {
        if (tempo == -1)
        {
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, frames + 5, freq);
            SEQCMD_RESET_TEMPO(SEQ_PLAYER_BGM_MAIN, frames);
        }
        else
        {
            SEQCMD_SET_SEQPLAYER_FREQ(SEQ_PLAYER_BGM_MAIN, frames + 5, freq);
            SEQCMD_SET_TEMPO(SEQ_PLAYER_BGM_MAIN, frames, tempo);
        }
    }
    void Opening_MainPlayMusic(GameState* thisx) 
    {
        TitleSetupState* this = (TitleSetupState*)thisx;
        GraphicsContext* __gfxCtx = this->state.gfxCtx;

        Gfx* gfxRef = POLY_OPA_DISP;
        Gfx* gfx = Graph_GfxPlusOne(gfxRef);
        gSPDisplayList(OVERLAY_DISP++, gfx);
        
        Gfx_SetupFrame(this->state.gfxCtx, 0, 0, 0);
        SetupRuntimeFont(thisx);  

        gDPPipeSync(gfx++);
        gDPSetCycleType(gfx++, G_CYC_FILL);
        gDPSetRenderMode(gfx++, G_RM_NOOP, G_RM_NOOP2);
        gDPSetFillColor(gfx++, (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
        gDPFillRectangle(gfx++, 0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
        Gfx_SetupDL_39Ptr(&gfx);  
        
        int FastForward = 0;
        
        if (CHECK_BTN_ALL(this->state.input->cur.button, BTN_A))
        {
            FastForward = 5;
            SetMusicTempo(150, 0, 1250);
        }
        else if (CHECK_BTN_ALL(this->state.input->cur.button, BTN_B))
        {
            FastForward = 15;
            SetMusicTempo(300, 0, 1750);
        }
        else
            SetMusicTempo(-1, 0, 1000);    
        

        // Music, innit
        TextOperation(NULL, that->font, &gfx, 
                      noExpPakFontColor, noExpPakShadowColor, 
                      255, 255, 
                      debugMusicString, 
                      GetStringCenterX(debugMusicString, TEXT_SCALE) + Rand_S16Offset(-2 - FastForward, 2 + FastForward), 
                      110 + Rand_S16Offset(-2 - FastForward, 2 + FastForward), 
                      1, 1, 
                      NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);
                      
        // Controls
        TextOperation(NULL, that->font, &gfx, 
                      noExpPakFontColor, noExpPakShadowColor, 
                      255, 255, 
                      debugMusicStringControls, 
                      GetStringCenterX(debugMusicStringControls, TEXT_SCALE), 200,
                      1, 1, 
                      NULL, TEXT_SCALE, TEXT_SCALE, 0, false, OPERATION_DRAW_SHADOW);                      
        
        gSPEndDisplayList(gfx++);
        Graph_BranchDlist(gfxRef, gfx);
        POLY_OPA_DISP = gfx;
        return;
    }

    #pragma GCC reset_options
#endif

extern u8 gViConfigModeType;
extern ViMode sViMode;

void Opening_Init(GameState* thisx) 
{
    TitleSetupState* this = (TitleSetupState*)thisx;
    
    // Delay the proceedings for a frame so that emulators have time to register the L + R 50hz input
    if (this->state.main != Opening_Init)
    {
        this->state.main = Opening_Init;
        return;
    }
    
    _isPrintfInit();
    is64Printf("=== HERO OF LAW ===\n");     
    
    that = (OpeningData*)THA_AllocTailAlign16(&thisx->tha, sizeof(OpeningData));    
    
    // Load emulator identifier from reserved space in the header. 
    osEPiReadIo(gCartHandle, 0x18, emuIdentifier);
    *hzChoice = 0;
    
    that->hzChoiceForce = CHECK_BTN_ALL(this->state.input->cur.button, BTN_L | BTN_R);
    
    // Set the video mode to PAL50 if region is set to EU to let user pick between 60Hz/50Hz.
    // Pressing L and R together forces this, as well.

#ifdef HZSWITCH
    
        if (osTvType == OS_TV_PAL || that->hzChoiceForce)
        {
            *hzChoice = 1;
            ViMode_Configure(&sViMode, OS_VI_FPAL_LAN1, OS_TV_PAL, 1, 0, 1, 1, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0);
            gViConfigMode = sViMode.customViMode;
            osViSetYScale(0.833f);      
            osViClock = VI_PAL_CLOCK;
        }
    
#endif
    
    // Controller Info Graphic.
    RomFile* cig = &objectTable[8];
    that->controllerInfoGfx = (u8*)THA_AllocTailAlign16(&thisx->tha, cig->vromEnd - cig->vromStart);
    DmaMgr_SendRequest1(that->controllerInfoGfx, cig->vromStart, cig->vromEnd - cig->vromStart);

    // Font.
    that->font = (Font*)THA_AllocTailAlign16(&thisx->tha, sizeof(Font));

    // Logo.
    DmaMgr_SendRequest1(that->segBuf, gDmaDataTable[938].vromStart, gDmaDataTable[938].vromEnd - gDmaDataTable[938].vromStart);   

    R_UPDATE_RATE = 1;
    Matrix_Init(&this->state);
    View_Init(&this->view, this->state.gfxCtx);
    this->state.main = &ConsoleLogo_Main;
    this->state.destroy = &Opening_Destroy;
    
    // Patch the G_MEMMOVE commands in the dlisplay list, so that lights work properly.
    AVAL(that->segBuf, Gfx, 0x27B0) = gsSPNoOp();
    AVAL(that->segBuf, Gfx, 0x27B8) = gsSPNoOp();
    AVAL(that->segBuf, Gfx, 0x2808) = gsSPNoOp();
    AVAL(that->segBuf, Gfx, 0x2810) = gsSPNoOp();
    
    that->ult = 0;
    that->coverAlpha = 255;
    that->addAlpha = -3;
    that->visibleDuration = 200;
    that->logoScale = 1.0;
    that->n64TextColorPrim = primBlue;
    that->n64TextColorEnv = envBlue;
    that->spinSpeed = 300;
    that->fade = 255;
    that->timer = 0;
    
    that->controllerInfoState = STATE_INIT;
    that->logoState = LOGOSTATE_PREINIT;
    
#ifdef MUSID
    Font_LoadFont(that->font);
    Audio_QueueSeqCmd(MUSID); 
    this->state.main = Opening_MainPlayMusic;
#endif
    
    // Detect any N64 mice connected to the system, as long as they're not in Port 1,
    // but disable any normal controllers in those slots.
    for (int i = 1; i < MAXCONTROLLERS; i++)
    {
        if (gPadMgr.padStatus[i].errno == 0) 
        {
            if (gPadMgr.padStatus[i].type == CONT_TYPE_NORMAL)  
                gPadMgr.validCtrlrsMask &= ~1 << i;
            if (gPadMgr.padStatus[i].type == CONT_TYPE_MOUSE) 
                gPadMgr.validCtrlrsMask |= 1 << i;
            
        }   
    }
}

void* InitADDR = &Opening_Init;