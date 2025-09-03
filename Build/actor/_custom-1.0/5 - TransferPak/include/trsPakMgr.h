#ifndef _Z_TRANSFERPAK_H_
#define _Z_TRANSFERPAK_H_

#include "tpak.h"
#include <z64hdr/oot_u10/z64hdr.h>
#include "../../draw2D.h"
#include "../../holText.h"
#include "../../3 - NPC Maker/include/npc_maker_types.h"

//#define PRINTOUT_SRAM
//#define PREVIEW_GBPIC

#define AVAL(base,type,offset)  (*(type*)((u8*)(base)+(offset)))
#define AADDR(a,o)  ((void*)((u8*)(a)+(o)))

#define TRSPAKMGR_REINITIALIZABLE 0x0F0F
#define DUMMY_MSG_ENTRY 0x011A

#define FUNCTION_OK 0
#define FUNCTION_ERR 1
#define FUNCTION_PAK_CHANGED 2
#define FUNCTION_NO_ACTION 3
#define FUNCTION_NO_PAK 4

#define GBGAMEFUNC_LA_LADX_REMOVETHIEF 1
#define GBGAMEFUNC_GETGBCAMERAPHOTO 2

#define SRAM_PRINTOUT_SIZE 16 * 4
#define INDIVIDUAL_DATA_SIZE 0x100

#define GB_SRAM 0xA000

#define LA_SAVE_PREFIX 0x100
#define LA_SLOT_SIZE 0x385
#define LADX_SLOT_SIZE 0x3AD

#define LA_RELEVANT_DATA_OFFSET 0x34B

#define LA_SHOP_MURDER_OFFSET(dataOffset) 0x0 + dataOffset
#define LA_LINK_NAME_OFFSET(dataOffset) 0x9 + dataOffset
#define LA_LINK_THIEF_OFFSET(dataOffset) 0x28 + dataOffset
#define LADX_TUNIC_OFFSET(dataOffset) 0x5F + dataOffset

#define LA_SLOT_OFFSET(slot, game) GB_SRAM + LA_SAVE_PREFIX + slot * (game ? LADX_SLOT_SIZE : LA_SLOT_SIZE)
#define LA_GET_DATA_OFFSET(slot, game, offset) LA_SLOT_OFFSET(slot, game) + LA_RELEVANT_DATA_OFFSET - offset

u8 LA_SAVESLOT_MAGIC[] = {0x01, 0x03, 0x05, 0x07, 0x09};

#define GBCAMERA_PICSIZE 0x1000
#define GBCAMERA_PIC_X 128
#define GBCAMERA_PIC_Y 112
#define GBCAMERA_PICSIZE_PIXELS GBCAMERA_PIC_X*GBCAMERA_PIC_Y
#define GBCAMERA_SLOTSDATA_OFFSET 0xB1A0
#define GBCAMERA_NUMSLOTS 0x1E
#define GBCAMERA_SLOT_EMPTY 0xFF

u8 GB_CAMERA_MAGIC[] = {0x4D, 0x61, 0x67, 0x69, 0x63};

char* waitMessage[] = 
{
    "\x08""Accessing the Transfer Pak...\x01""\x05""\x41""Do not remove the Transfer Pak or touch\x01""the POWER switch.\x05""\x40""\x0E""\x14""\x02\0x00",    
    "\x08""Acc\x85""s au Transfer Pak en cours... \x01""\x05""\x41""Veuillez ne pas retirer le Transfer Pak\x01""ni appuyer sur le bouton POWER.\x05""\x40""\x0E""\x14""\x02\0x00", 
};

char emptyName[] = "?????";

#ifndef RunGameFunc
	typedef void RunGameFunc(Actor* thisx, u16 func);
#endif

enum InitStatus
{
    INIT_IDLE,
    INIT_START,
    INIT_OK,
    INIT_DONE,
};

enum Games
{
    GBGAME_NONE = 0,
    GBGAME_SUPERMARIOLAND = 1,
    GBGAME_SUPERMARIOLAND2 = 2,
    GBGAME_WARIOLAND1 = 3,
    GBGAME_WARIOLAND2 = 4,
    GBGAME_CGBWARIOLAND2 = 5,
    GBGAME_WARIOLAND3 = 6,
    GBGAME_LINKSAWAKENING = 7,
    GBGAME_ORACLEOFAGES = 8,
    GBGAME_ORACLEOFSEASONS = 9,
    GBGAME_GBCAMERA = 10,
    GBGAME_POCKETCAMERA = 11,
    GBGAME_EZGB = 12,
    GBGAME_UNKNOWN = 254,
    GBGAME_EVERDRIVE = 255,
};

char* recognizedGames[] = 
{
    "SUPER MARIOLAND",  // 1
    "MARIOLAND2",       // 2
    "SUPERMARIOLAND3",  // 3
    "WARIOLAND2",       // 4
    "CGBWARIOLAND2",    // 5
    "WARIOLAND3",       // 6
    "ZELDA\x00",        // 7
    "ZELDA NAYRU",      // 8
    "ZELDA DIN",        // 9
    "GAMEBOYCAMERA",    // 10
    "POCKETCAMERA",     // 11
    "EZGB"              // 12
};

u8 recognizedGamesTitleLenghts[] =
{
    15,
    10,
    15,
    10,
    13,
    10,
    6,
    11,
    9,
    13,
    12,
    4
};

char* everdriveCheck = "KRKZ";

typedef struct GBColorPalette
{
    u8 colorBlack;
    u8 colorGray;
    u8 colorLGray;
    u8 colorWhite;
} GBColorPalette;

typedef struct TrsPakMgr
{
    Actor actor;
    u8 init;
    u8 gbpakStatus;
    u8 insertedGame;
    u8 cgbFlag;
    u8 noPak;
    u8 prevInsertedGame;
    u8 trPakInited;
    u8 disabled;
    u8 individualData[INDIVIDUAL_DATA_SIZE];

    MessageEntry* msgEntry;
    gameboy_cartridge_header gbPakHeader;
    RunGameFunc* gameFuncFunc;
    
#ifdef PRINTOUT_SRAM    
    u8 debugSramBuf[SRAM_PRINTOUT_SIZE];
#endif

} TrsPakMgr;

typedef struct GBLAIndividual
{
    u8 status;
    u8 thiefNum;
    u8 slot;
    u8 killedByShopkeeper;
    u8 tunicColor;
    u8 pad[3];
    u8 ogName[5];
} GBLAIndividual;

typedef struct GBCameraIndividual
{
    u8 status;
    u8* gbCameraPic;
    u8 slots[0x1E];
} GBCameraIndividual;

s8 InitGbPak(Actor* thisx);
s8 GetGbPakData(Actor* thisx);
s8 GetGbPakStatus(Actor* thisx);
s8 SwitchGbPakRAMBank(Actor* thisx, u8 bank);
u8 CheckGameTitle(Actor* thisx);
int GetStringDrawnLength(char* string, float scale);
int GetStringCenterX(char* string, float scale);
int GetScreenCenterY(int Size);
void IndividualGameChecks(Actor* thisx);
s8 ReadGbPak(Actor* thisx, u16 address, u16 size, u8* buf);
s8 WriteGbPak(Actor* thisx, u16 address, u16 size, u8* buf);
s8 EnableGbPakSram(Actor* thisx);
void IndividualGameFuncs(Actor* thisx, u16 func);
s8 GbPakOff(Actor* thisx);
void ShowWaitMsg(Actor* thisx, PlayState* play);
MessageEntry* Rom_GetMessageEntry(s16 msgId);
void DestroyIndividualData(Actor* thisx, int game);
s8 SetupGBEverdrive(Actor* thisx);

#endif
