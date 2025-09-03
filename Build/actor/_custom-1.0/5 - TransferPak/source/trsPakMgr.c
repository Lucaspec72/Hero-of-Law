
#include "../include/trsPakMgr.h"
#include "../../is64Printf.h"
#include "../../common.h"

#define THIS ((TrsPakMgr*)thisx)

void TrsPakMgr_Init(Actor* thisx, PlayState* play);
void TrsPakMgr_Destroy(Actor* thisx, PlayState* play);
void TrsPakMgr_Update(Actor* thisx, PlayState* play);
void TrsPakMgr_Draw(Actor* thisx, PlayState* play);

// Transfer Pak functions based on the libdragon implementation
// https://github.com/DragonMinded/libdragon

const ActorInitExplPad TrsPakMgr_InitVars =
{
    0xDEAD,
    ACTORCAT_NPC,
    0x00000030,
    1,
    0xBEEF,
    sizeof(TrsPakMgr),
    (ActorFunc)TrsPakMgr_Init,
    (ActorFunc)TrsPakMgr_Destroy,
    (ActorFunc)TrsPakMgr_Update,
    (ActorFunc)TrsPakMgr_Draw,
};

void TrsPakMgr_Init(Actor* thisx, PlayState* play)
{
    TrsPakMgr* this = THIS;
    this->insertedGame = GBGAME_NONE;
    this->gbpakStatus = 0;
    this->init = INIT_IDLE;
    this->msgEntry = Rom_GetMessageEntry(DUMMY_MSG_ENTRY);
    play->msgCtx.textboxBackgroundBackColorIdx = 3;
    bzero(this->individualData, INDIVIDUAL_DATA_SIZE);    
    this->gameFuncFunc = &IndividualGameFuncs;
    this->disabled = false;
}

void TrsPakMgr_Destroy(Actor* thisx, PlayState* play)
{
    TrsPakMgr* this = THIS;
    
    DestroyIndividualData(thisx, this->insertedGame);
    this->insertedGame = GBGAME_NONE;
}

void TrsPakMgr_Update(Actor* thisx, PlayState* play)
{
    TrsPakMgr* this = THIS;

    if (gPadMgr.pakType[0] != CONT_PAK_OTHER)
    {
        this->init = INIT_IDLE;
        DestroyIndividualData(thisx, this->insertedGame);      
        bzero(this->individualData, INDIVIDUAL_DATA_SIZE);
        bzero(&this->gbPakHeader, sizeof(gameboy_cartridge_header));    
        this->insertedGame = GBGAME_NONE;
        this->noPak = true;
        *tpakBeingUsed = 0;
    }
    else
        this->noPak = false;
    
    // Start initing as soon as pak is inserted.
    // Re-init if the player presses D-Pad Left + Z.
    if ((this->init == INIT_IDLE && 
        !this->disabled && 
        gPadMgr.pakType[0] == CONT_PAK_OTHER) ||
        (this->init == INIT_DONE &&
        this->actor.params == TRSPAKMGR_REINITIALIZABLE && 
        CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_DLEFT) && 
        CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_Z)))
    {
        this->init = INIT_START;
    }    
    
    if (this->init == INIT_START && !this->disabled)
    {
        bzero(this->individualData, INDIVIDUAL_DATA_SIZE);
        bzero(&this->gbPakHeader, sizeof(gameboy_cartridge_header));
        this->insertedGame = GBGAME_NONE;
        
        if (!InitGbPak(thisx))
        {
            *tpakBeingUsed = 1;
            ShowWaitMsg(thisx, play);
            this->init = INIT_OK;
            return;
        }
        else
        {
            *tpakBeingUsed = 0;
            this->init = INIT_DONE;
        }
    }
    
    if (this->init == INIT_OK && (play->msgCtx.msgMode == MSGMODE_TEXT_DONE || this->msgEntry == NULL))
    {
        if (!GetGbPakStatus(thisx))
        {
            if (this->gbpakStatus & TPAK_STATUS_POWERED)
            {
                if (GetGbPakData(thisx))
                {
                    bzero(&this->gbPakHeader, sizeof(gameboy_cartridge_header));
                    this->insertedGame = GBGAME_NONE;   
                    this->init = INIT_DONE; 
                    return;
                }
                
                this->insertedGame = CheckGameTitle(thisx);
                
                if (this->insertedGame == GBGAME_EVERDRIVE && CHECK_BTN_ALL(play->state.input[0].cur.button, BTN_CUP))
                {
                    if (!SetupGBEverdrive(thisx))
                    {                        
                        if (GetGbPakData(thisx))
                        {
                            bzero(&this->gbPakHeader, sizeof(gameboy_cartridge_header));
                            this->insertedGame = GBGAME_NONE;   
                            this->init = INIT_DONE; 
                            return;
                        }                 
                    }
                }
                
                this->prevInsertedGame = this->insertedGame;
                this->insertedGame = CheckGameTitle(thisx);
                
                IndividualGameChecks(thisx);     

                if (this->actor.params != 0 && this->actor.params != TRSPAKMGR_REINITIALIZABLE)
                   IndividualGameFuncs(thisx, this->actor.params);

                GbPakOff(thisx);
            }
            else
            {
                bzero(&this->gbPakHeader, sizeof(gameboy_cartridge_header));
                this->insertedGame = GBGAME_NONE;
            }
        }     

        this->init = INIT_DONE;
        *tpakBeingUsed = 0;
    }

}

void TrsPakMgr_Draw(Actor* thisx, PlayState* play)
{
#if DEBUGVER == 1
    TrsPakMgr* this = THIS;
    GraphicsContext* __gfxCtx = play->state.gfxCtx;
    Gfx* gfx;
    Gfx* gfxRef;
    gfxRef = POLY_OPA_DISP;
    gfx = Graph_GfxPlusOne(gfxRef);
    gSPDisplayList(OVERLAY_DISP++, gfx);  
          
    char* buf = "DEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUGDEBUG";
    char* paks[] = {"No Pak", "Rumble Pak", "Mem Pak/Other", "Transfer Pak"};
    char* inits[] = {"IDLE", "START", "OK", "DONE"};
    
    char* gbpakTitle[17];
    bcopy(this->gbPakHeader.title, gbpakTitle, 16);
    gbpakTitle[16] = 0;
    
    sprintf(buf, "Init: %s, %s, Cart: 0x%x, GBC: 0x%x, Region: %x, Ver: 0x%x\x01""GB Title: %s (ID: %d)\x02", 
                inits[this->init], 
                gPadMgr.pakType[0] == CONT_PAK_OTHER ? (this->trPakInited ? paks[3] : paks[2]) : paks[gPadMgr.pakType[0]], 
                this->gbpakStatus, 
                this->gbPakHeader.new_title.gbc_support, 
                this->gbPakHeader.destination_code, 
                this->gbPakHeader.version_number, 
                gbpakTitle, 
                this->insertedGame);
                
    TextOperation(play, NULL, &gfx, 
                  COLOR_WHITE, COLOR_BLACK,
                  255, 255, 
                  buf, 
                  5, 24, 
                  1, 1, 
                  NULL, 65, 65, 0, false, OPERATION_DRAW);         
                        
                        
    switch (this->insertedGame)
    {
        case GBGAME_GBCAMERA:
        case GBGAME_POCKETCAMERA:
        {
            #ifdef PREVIEW_GBPIC
                GBCameraIndividual* data = (GBCameraIndividual*)this->individualData;
                
                if (data->gbCameraPic != NULL)
                    Draw2DInternal(RGBA16, (u8*)data->gbCameraPic, NULL, &gfx, 120, 120, GBCAMERA_PIC_X, GBCAMERA_PIC_Y, GBCAMERA_PIC_X, GBCAMERA_PIC_Y, 255);
                
            #endif
            break;
        }
        case GBGAME_LINKSAWAKENING:
        {
            GBLAIndividual* data = (GBLAIndividual*)this->individualData;
            
            char* tunics[] = {"Green", "Red", "Blue"};
            sprintf(buf, "Thief detected: %x, Slot: %x, Name: %s, Tunic: %s", data->thiefNum, data->slot, &data->ogName, tunics[data->tunicColor]);

            TextOperation(play, NULL, &gfx, 
                          COLOR_WHITE, COLOR_BLACK,
                          255, 255, 
                          buf, 
                          5, 44, 
                          1, 1, 
                          NULL, 65, 65, 0, false, OPERATION_DRAW);     

            break;                            
        }                     
    }
    
    #ifdef PRINTOUT_SRAM                
        int x = 10;
        int y = 150;
        
        u32* buff = (u32*)this->debugSramBuf;

        for (int i = 0; i < SRAM_PRINTOUT_SIZE / 4; i++)
        {
            sprintf(buf, "%08x", buff[i]);
            
            TextOperation(play, NULL, &gfx, 
                          COLOR_WHITE, COLOR_BLACK,
                          255, 255, 
                          buf, 
                          x, y, 
                          1, 1, 
                          NULL, 65, 65, 0, false, OPERATION_DRAW);             
                                
            x += 64;
            
            if (x > 280)
            {
                x = 10;
                y += 16;
            }
        }        
    #endif          
    
    gSPEndDisplayList(gfx++);
    Graph_BranchDlist(gfxRef, gfx);
    POLY_OPA_DISP = gfx;

    
#endif        
}

void DestroyIndividualData(Actor* thisx, int game)
{
    TrsPakMgr* this = THIS;
    
    switch (game)
    {
        case GBGAME_GBCAMERA:
        case GBGAME_POCKETCAMERA:
        {
            GBCameraIndividual* data = (GBCameraIndividual*)this->individualData;
            
            if (data->gbCameraPic != NULL)
                ZeldaArena_Free(data->gbCameraPic);
        }
    }
}

void ShowWaitMsg(Actor* thisx, PlayState* play)
{
    TrsPakMgr* this = THIS;
    
    if (this->msgEntry != NULL)
    {
        this->msgEntry->settings = 0x12;
        Message_StartTextbox(play, DUMMY_MSG_ENTRY, NULL);
        char* msg = waitMessage[MIN(SAVE_LANGUAGE, HOL_LANGUAGE_MAX - 1)];
        bcopy(msg, play->msgCtx.font.msgBuf, strlen(msg));
    }    
}

MessageEntry* Rom_GetMessageEntry(s16 msgId)
{
    #define messageTable (*(MessageEntry(*)[]) 0x8010EA8C)
    
    for (int i = 0; i < 65535; i++)
    {
        MessageEntry* MsgE = AADDR(messageTable, i * sizeof(MessageEntry));

        if (MsgE->msg_id == msgId)
            return MsgE;
    }

    return NULL;
}

u8 CheckGameTitle(Actor* thisx)
{
    TrsPakMgr* this = THIS;
    
    int j = 0;
    int eC = 0;
    
    while(this->gbPakHeader.title[j])
    {
        if (this->gbPakHeader.title[j] == everdriveCheck[eC])
            eC++;
        else
            eC = 0;
        
        if (eC == ARRAY_COUNT(everdriveCheck))
            return GBGAME_EVERDRIVE;
        
        j++;
    }
    
    j = 0;
    
    for (int i = 0; i < ARRAY_COUNT(recognizedGames); i++)
    {
        if (!bcmp(this->gbPakHeader.title, recognizedGames[i], recognizedGamesTitleLenghts[i]))
            return i + 1;
    }
    
    return GBGAME_UNKNOWN;
    
}

u16 GetAlignedLAOffset(u8 slot, u8 game, u16 offset)
{
    // Get closest (lower) address that's divisible by 32
    // This is because TPak can only read in chunks of 32.
    return (LA_SLOT_OFFSET(slot, game) + offset) & ~(0x1F);
}

// Based on https://www.insidegadgets.com/2017/07/11/learning-about-gameboy-camera-saves-and-converting-stored-images-to-bitmap/
void DecodeGameBoyCameraPic(u8* in, u8* out, GBColorPalette* pal)
{
    int curByte = 0;
    int outIndex = 0;
    
    for (int a = 0; a < 14; a++) 
    {
        for (int b = 0; b < 8; b++) 
        {
            for (int c = 0; c < 16; c++) 
            {
                u8 pixelWhiteLGray = in[curByte];
                u8 pixelGreyBlack = in[curByte + 1];
                
                for (int d = 7; d >= 0; d--) 
                {
                    u8 wl = pixelWhiteLGray & 1 << d;
                    u8 gb = pixelGreyBlack & 1 << d;
                    
                    out[outIndex] = 0xF;
                    
                    if (wl && gb) 
                        out[outIndex] |= (pal->colorBlack << 4);
                    else if (gb) 
                        out[outIndex] |= (pal->colorGray << 4);
                    else if (wl)
                        out[outIndex] |= (pal->colorLGray << 4);
                    else
                        out[outIndex] |= (pal->colorWhite << 4);   

                    outIndex++;
                }
                
                curByte += 0x10;
            }
            curByte -= 0xFE;
        }
        curByte += 0xF0;
    }    
}

void IndividualGameFuncs(Actor* thisx, u16 func)
{
    TrsPakMgr* this = THIS;
    
    switch (func & 0xFF)
    {
        case GBGAMEFUNC_GETGBCAMERAPHOTO:
        {
            GBCameraIndividual* data = (GBCameraIndividual*)this->individualData;
            data->status = FUNCTION_ERR;
            
            if (this->insertedGame == 0)
            {
                data->status = FUNCTION_NO_PAK;
                return;
            }     
            
            if (this->insertedGame != GBGAME_GBCAMERA &&  
                this->insertedGame != GBGAME_POCKETCAMERA)
            {
                data->status = FUNCTION_PAK_CHANGED;
                return;
            }            

            u8 slot = ((func >> 8) & 0x1F);
            u8 bank = 1 + (slot / 2);
            u8* buf = ZeldaArena_Malloc(GBCAMERA_PICSIZE);
            
            if (EnableGbPakSram(thisx) ||
                SwitchGbPakRAMBank(thisx, bank) ||
                ReadGbPak(thisx, GB_SRAM + ((slot % 2) * GBCAMERA_PICSIZE), GBCAMERA_PICSIZE, buf))
            {
                data->status = FUNCTION_ERR;
                return;
            }
            
            if (data->gbCameraPic == NULL)
                data->gbCameraPic = ZeldaArena_Malloc(GBCAMERA_PICSIZE_PIXELS);
            
            if (data->gbCameraPic != NULL)
            {
                GBColorPalette standardPalette = 
                {
                    .colorBlack = 0, 
                    .colorGray = 85, 
                    .colorLGray = 170, 
                    .colorWhite = 255
                };                
                
                DecodeGameBoyCameraPic(buf, data->gbCameraPic, &standardPalette);
                data->status = FUNCTION_OK;
            }
            else
                data->status = FUNCTION_ERR;
            
            
            ZeldaArena_Free(buf);
            break;
        }
        case GBGAMEFUNC_LA_LADX_REMOVETHIEF:
        {
            GBLAIndividual* data = (GBLAIndividual*)this->individualData;
            data->status = FUNCTION_ERR;
            
            if (this->insertedGame == GBGAME_NONE)
            {
                data->status = FUNCTION_NO_PAK;
                return;
            }     
            
            if (this->insertedGame != GBGAME_LINKSAWAKENING)
            {
                data->status = FUNCTION_PAK_CHANGED;
                return;
            }           
            
            u8 buf[96];
            
            if (EnableGbPakSram(thisx) || SwitchGbPakRAMBank(thisx, 0))
            {
                data->status = FUNCTION_ERR;
                return;
            }
            
            u8 game = (this->gbPakHeader.new_title.gbc_support != GBC_NOT_SUPPORTED);
            u8 slot = ((func >> 8) & 0x3);
            
            u16 offsetAligned = GetAlignedLAOffset(slot, game, LA_RELEVANT_DATA_OFFSET);
            u16 dataOffset = LA_GET_DATA_OFFSET(slot, game, offsetAligned);
            
            if (ReadGbPak(thisx, offsetAligned, 96, buf))
            {
                data->status = FUNCTION_ERR;
                return;
            }                
            
            if (buf[LA_LINK_THIEF_OFFSET(dataOffset)])
            {
                buf[LA_LINK_THIEF_OFFSET(dataOffset)] = 0;
                buf[LA_SHOP_MURDER_OFFSET(dataOffset)] = 0;
                
                if (WriteGbPak(thisx, offsetAligned, 96, buf) == 0)
                    data->status = FUNCTION_OK;
                else
                    data->status = FUNCTION_ERR;
                
                u8 buf2[96];
                
                if (ReadGbPak(thisx, offsetAligned, 96, buf2))
                {
                    data->status = FUNCTION_ERR;
                    return;
                }                  
                
                // Compare what we've written.
                if (bcmp(&buf, &buf2, 96))
                {
                    data->status = FUNCTION_ERR;
                    return;
                }
            }         
            else
                data->status = FUNCTION_NO_ACTION;
        }
        default:
            break;
    }   
    
}

void IndividualGameChecks(Actor* thisx)
{
    TrsPakMgr* this = THIS;
    
    DestroyIndividualData(thisx, this->prevInsertedGame);
    
    bzero(this->individualData, INDIVIDUAL_DATA_SIZE);
        
    switch (this->insertedGame)
    {
        case GBGAME_GBCAMERA:
        case GBGAME_POCKETCAMERA:
        {
            GBCameraIndividual* data = (GBCameraIndividual*)this->individualData;
            u8 buf[64];
            
            Lib_MemSet(&data->slots[0], GBCAMERA_NUMSLOTS, GBCAMERA_SLOT_EMPTY);

            if (EnableGbPakSram(thisx) ||
                SwitchGbPakRAMBank(thisx, 0) ||
                ReadGbPak(thisx, GBCAMERA_SLOTSDATA_OFFSET, 64, buf))
                return;
            
#ifdef PRINTOUT_SRAM
            bcopy(buf, this->debugSramBuf, SRAM_PRINTOUT_SIZE);
#endif         
            if (bcmp(&buf[0x30], &GB_CAMERA_MAGIC, 0x5))
                return;
            
            bcopy(&buf[0x12], &data->slots[0], GBCAMERA_NUMSLOTS);
           
            break;
        }            
        case GBGAME_LINKSAWAKENING:
        {
            // Detect whether the player has stolen an item from the shop.
            GBLAIndividual* data = (GBLAIndividual*)this->individualData;
            u8 buf[128];
            
            bcopy(emptyName, &data->ogName, 5);
            data->slot = 0xFF;

            if (EnableGbPakSram(thisx) || SwitchGbPakRAMBank(thisx, 0))
                return;
            
            for (int z = 2; z >= 0; z--)
            {
                u8 game = (this->gbPakHeader.new_title.gbc_support != GBC_NOT_SUPPORTED);
                u16 offsetAligned = GetAlignedLAOffset(z, game, 0);
                
                if (ReadGbPak(thisx, offsetAligned, 32, buf))
                    continue;
                
#ifdef PRINTOUT_SRAM
                bcopy(buf, this->debugSramBuf, SRAM_PRINTOUT_SIZE);
#endif              
                // Check if slot is valid.
                if (bcmp(&buf[LA_SLOT_OFFSET(z,game) - offsetAligned] , &LA_SAVESLOT_MAGIC, 5))
                    continue;
  
                offsetAligned = GetAlignedLAOffset(z, game, LA_RELEVANT_DATA_OFFSET);
                
                // for LADX, read more data
                if (ReadGbPak(thisx, offsetAligned, game ? 128 : 96, buf))
                    continue;
                
#ifdef PRINTOUT_SRAM
                bcopy(&buf[128 - SRAM_PRINTOUT_SIZE], this->debugSramBuf, SRAM_PRINTOUT_SIZE);
#endif
                u16 dataOffset = LA_GET_DATA_OFFSET(z, game, offsetAligned);
                
                u8 blank[] = {0x00, 0x00, 0x00, 0x00, 0x00};
                
                // Check if name is blank.
                if (!bcmp(&buf[LA_LINK_NAME_OFFSET(dataOffset)], blank, 5))
                    continue;
                
                data->thiefNum = buf[LA_LINK_THIEF_OFFSET(dataOffset)];
                
                if (data->thiefNum > 1 || buf[LA_SHOP_MURDER_OFFSET(dataOffset)] == 0)
                    data->killedByShopkeeper = 1;

                data->slot = z;
                bcopy(&buf[LA_LINK_NAME_OFFSET(dataOffset)], &data->ogName, 5);
                
                // Turn trailing NULL characters into 0xFF so that when the name is inserted into messages
                // the blank chars just don't print instead of ending the message.
                for (int i = 4; i >= 0; i--)
                {
                    if (data->ogName[i] == 0x0)
                        data->ogName[i] = 0xFF;
                    else
                        break;
                }
                
                // Turn the rest of 0x0s into spaces.
                for (int i = 0; i < 5; i++)
                {
                    if (data->ogName[i] == 0x0)
                        data->ogName[i] = 0x20;
                }
                
                // Reencode Japanese
                if (this->gbPakHeader.destination_code == 0)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        if (data->ogName[i] != 0xFF && data->ogName[i] != 0x20)
                        {
                            if (data->ogName[i] == 0xA3)
                                data->ogName[i] = 0xF7; // EM Dash
                            else
                                data->ogName[i] = data->ogName[i] + 0xAB;   
                        }
                    }
                }
                else
                {
                    // Reencode German chars
                    for (int i = 0; i < 5; i++)
                    {
                        switch (data->ogName[i])
                        {
                            case 0xC5: data->ogName[i] = 0x83; break;   // Ä
                            case 0xD7: data->ogName[i] = 0x8B; break;   // Ö
                            case 0xDD: data->ogName[i] = 0x8E; break;   // Ü
                            case 0xE0: data->ogName[i] = 0x8F; break;   // ß
                            case 0xE5: data->ogName[i] = 0x93; break;   // ä
                            case 0xF7: data->ogName[i] = 0x9B; break;   // ö
                            case 0xFD: data->ogName[i] = 0x9E; break;   // ü
                            default:
                            {
                                if (data->ogName[i] != 0xFF && data->ogName[i] != 0x20)
                                    data->ogName[i]--;
                                break;
                            }
                        }
                    }   
                }
                
                if (game)
                    data->tunicColor = buf[LADX_TUNIC_OFFSET(dataOffset)];
                else
                    data->tunicColor = 0;

                if (data->thiefNum != 0)
                    break;
            }
 
            break;
        }
        default:
            break;
    }
}

s8 EnableGbPakSram(Actor* thisx)
{
    u8 buf[32];
    Lib_MemSet(buf, 32, 0xA);
    
    return WriteGbPak(thisx, 0x0000, 32, buf); 
}

s8 GbPakOff(Actor* thisx)
{
    OSMesgQueue* serialEventQueue;    
    u8 ret;
    
    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    ret = tpak_open(serialEventQueue, 0);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);       
    
    return ret;
}

s8 SetupGBEverdrive(Actor* thisx)
{
    u8 buf[32];
    
    OSMesgQueue* serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);

    u8 ret = tpak_open(serialEventQueue, 0);
    
    if (!ret)
    {
        ret = tpak_get_status(serialEventQueue, 0);
    
        if (ret & TPAK_STATUS_POWERED)
        {
            Lib_MemSet(buf, 32, 2);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xA000), buf, 0);
            Lib_MemSet(buf, 32, 0);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF00), buf, 0);
            Lib_MemSet(buf, 32, 0xFF);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF80), buf, 0);      
            Lib_MemSet(buf, 32, 1);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF00), buf, 0);      
            Lib_MemSet(buf, 32, 0xFF);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF80), buf, 0);  
            Lib_MemSet(buf, 32, 7);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF00), buf, 0);
            Lib_MemSet(buf, 32, 0xA);
            ret = __osContRamWrite(serialEventQueue, 0, CONT_BLOCKS(0xFF80), buf, 0);
        }
    }    
    
    
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);  

    return ret;    
}

s8 WriteGbPak(Actor* thisx, u16 address, u16 size, u8* buf)
{
    OSMesgQueue* serialEventQueue;    
    u8 ret;
    
    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    ret = tpak_open(serialEventQueue, 0);
    
    if (!ret)
        ret = tpak_write(serialEventQueue, 0, address, buf, size);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);  

    return ret;
}

s8 SwitchGbPakRAMBank(Actor* thisx, u8 bank)
{
    u8 buf[32];
    Lib_MemSet(buf, 32, bank);

    return WriteGbPak(thisx, 0x4000, 32, buf);
}

s8 ReadGbPak(Actor* thisx, u16 address, u16 size, u8* buf)
{
    OSMesgQueue* serialEventQueue;    
    u8 ret;
    
    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    ret = tpak_open(serialEventQueue, 0);
    
    if (!ret)
        ret = tpak_read(serialEventQueue, 0, address, buf, size);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);       
    
    return ret;
}


s8 GetGbPakStatus(Actor* thisx)
{
    TrsPakMgr* this = THIS;
    OSMesgQueue* serialEventQueue;    

    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    
    u8 ret = tpak_open(serialEventQueue, 0);
    
    if (!ret)
        this->gbpakStatus = tpak_get_status(serialEventQueue, 0);
    
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);     
    return 0;
}

s8 GetGbPakData(Actor* thisx)
{
    TrsPakMgr* this = THIS;
    OSMesgQueue* serialEventQueue;    
    u8 ret;

    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    ret = tpak_open(serialEventQueue, 0);
    
    if (!ret)
        ret = tpak_get_cartridge_header(serialEventQueue, 0, &this->gbPakHeader);
    
    if (!ret)
        ret = tpak_check_header(serialEventQueue, 0, &this->gbPakHeader) ? 0 : 1;
    
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);       

    return ret;
}

s8 InitGbPak(Actor* thisx) 
{
    TrsPakMgr* this = THIS;
    OSMesgQueue* serialEventQueue;
    u8 ret;
    
    serialEventQueue = PadMgr_AcquireSerialEventQueue(&gPadMgr);
    ret = tpak_init(serialEventQueue, 0);
    PadMgr_ReleaseSerialEventQueue(&gPadMgr, serialEventQueue);
    
    this->trPakInited = ret ? 0 : 1;
    
    return ret;
}