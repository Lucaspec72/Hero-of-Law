
#include "../include/bioSnsrMgr.h"

#define THIS ((BioSnsrMgr*)thisx)

void BioSnsrMgr_Init(Actor* thisx, PlayState* play);
void BioSnsrMgr_Destroy(Actor* thisx, PlayState* play);
void BioSnsrMgr_Update(Actor* thisx, PlayState* play);
void BioSnsrMgr_Draw(Actor* thisx, PlayState* play);

const ActorInitExplPad BioSnsrMgr_InitVars =
{
    0xDEAD,
    ACTORCAT_NPC,
    0x00000030,
    1,
    0xBEEF,
    sizeof(BioSnsrMgr),
    (ActorFunc)BioSnsrMgr_Init,
    (ActorFunc)BioSnsrMgr_Destroy,
    (ActorFunc)BioSnsrMgr_Update,
    (ActorFunc)BioSnsrMgr_Draw,
};

void BioSnsrMgr_Init(Actor* thisx, PlayState* play)
{
    BioSnsrMgr* this = THIS;
    this->bpm = BIOSENSOR_MEASURING;
    this->init = INIT_ERROR;
    this->beat = 0;
    this->framesNoChange = 120;
    
    this->alpha = 0;
    this->posX = 35;
    this->posY = 220;
    this->curScale = 1;
    this->invisible = false;
    
    for (int i = 0; i < BPS_SAMPLE_SIZE; i++)
        this->bpsdata[i] = 1;
}

void BioSnsrMgr_Destroy(Actor* thisx, PlayState* play)
{
 
}

void BioSnsrMgr_Update(Actor* thisx, PlayState* play)
{
    BioSnsrMgr* this = THIS;
   
    this->frameCount++;
    this->init = INIT_ERROR;
    
    if (gPadMgr.padStatus[0].errno == 0)
    {
        if (gPadMgr.padStatus[0].type == CONT_TYPE_NORMAL)
        {
            if (gPadMgr.pakType[0] == CONT_PAK_OTHER)
            {
                u8 buf[32];
                OSMesgQueue* serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
                u8 ret = __osContRamRead(serialEventQueue, 0, CONT_BLOCKS(0xC000), buf);
                PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);   

                if (!ret)
                {
                    if (buf[0] != 0 && buf[0] != 3)
                        this->init = INIT_ERROR;
                    else
                    {
                        if (this->beat == BEAT_OFF && buf[0] == 0)
                        {
                            this->beat = BEAT_DETECTED;
                            this->beats++;
                        }
                        else if (buf[0] == 0x3)
                            this->beat = BEAT_OFF;
                        
                        if (this->beat == this->beatLast)
                        {
                            this->framesNoChange++;
                            
                            if (this->framesNoChange >= 80)
                                this->init = INIT_ERROR;
                            else
                                this->init = INIT_OK;
                        }
                        else
                        {
                            this->init = INIT_OK;
                            this->framesNoChange = 0;
                        }
                        
                        this->beatLast = this->beat;
                    }
                }
            }
        }
    }
    
    if (this->init == INIT_ERROR)
    {
        this->bpm = BIOSENSOR_MEASURING;
        this->beat = BEAT_OFF;    
        this->frameCount = 0;        
        this->beats = 0;
        this->bpsPtr = 0;
        
        for (int i = 0; i < BPS_SAMPLE_SIZE; i++)
            this->bpsdata[i] = 1;       
    }
    else
    {
        if (this->frameCount == 60 / R_UPDATE_RATE)
        {
            this->bpsdata[this->bpsPtr] = this->beats;
            this->curbps = this->beats;
            this->bpsPtr++;
            this->beats = 0;
            this->frameCount = 0;
            
            this->bpm = 0;
            
            for (int i = 0; i < BPS_SAMPLE_SIZE; i++)
                this->bpm += this->bpsdata[i];
            
            this->bpm *= BPS_MULTIPLIER;
        }
    }
    
    if (this->bpsPtr >= BPS_SAMPLE_SIZE)
        this->bpsPtr = 0;
    
    if (this->curScale > 1)
        this->curScale -= 0.1;    
    
    if (this->beat)
        this->curScale = 1.5;    
    
    if (this->init == INIT_OK && this->bpm != BIOSENSOR_MEASURING && !this->invisible)
        Math_ApproachS(&this->alpha, 255, 1, 15);
    else
        Math_ApproachS(&this->alpha, 0, 1, 15);
    
}

void BioSnsrMgr_Draw(Actor* thisx, PlayState* play)
{

    GraphicsContext* __gfxCtx = play->state.gfxCtx;
    Gfx* gfx;
    Gfx* gfxRef;
    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);   
    
    BioSnsrMgr* this = THIS;

    if (this->init == INIT_OK && this->bpm != BIOSENSOR_MEASURING)
    {
        Draw2DScaled(CI8_Setup39, OBJ_GRAPHICS_COMMON, play, &gfx, this->posX, this->posY, 
                (u8*)HEARTS_OFFSET + 0x78, (u8*)HEARTS_OFFSET, HEARTS_WIDTH, HEARTS_HEIGHT, 
                    this->curScale * HEARTS_WIDTH, this->curScale * HEARTS_HEIGHT, this->alpha);
                    
        char* bufPulse = "9999";
        sprintf(bufPulse, "%d", this->bpm);            
                    
        TextOperation(play, NULL, &gfx, 
                      COLOR_WHITE, COLOR_BLACK,
                      this->alpha, this->alpha, 
                      bufPulse, 
                      this->posX - (this->bpm >= 100 ? 9 : 6), this->posY - 23, 
                      1, 1, 
                      NULL, 65, 65, 0, false, OPERATION_DRAW);                                  
    }
   
          
    #if DEBUGVER == 1
        char* buf = "DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG";
        char* bufPulse = "9999";
        char* inits[] = {"OK", "Not detected"};
        char* beats[] = {" ", "*"};
        
        sprintf(bufPulse, "%d", this->bpm);
        
        sprintf(buf, "BIO SENSOR: %s, BPM: %s, BPS: %d (%s)", 
                    inits[this->init], 
                    this->bpm == BIOSENSOR_MEASURING ? "Measuring" : bufPulse,
                    this->curbps,
                    beats[this->beat]);
        
        TextOperation(play, NULL, &gfx, 
                      COLOR_WHITE, COLOR_BLACK,
                      255, 255, 
                      buf, 
                      5, 55, 
                      1, 1, 
                      NULL, 65, 65, 0, false, OPERATION_DRAW);           
    #endif            
    
    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;    
}
