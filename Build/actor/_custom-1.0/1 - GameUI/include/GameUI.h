#ifndef _Z_GAMEUI_H_
#define _Z_GAMEUI_H_

#define GET_DIMENSIONS_FUNCS

#include <z64hdr/oot_u10/z64hdr.h>
#include "../../draw2D.h"
#include "../../holText.h"
#include "../../common.h"
#include "../../3 - NPC Maker/include/npc_maker_types.h"
#include "../../4 - Voice/include/voicemgr.h"

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define G_IM_SIZ_16b_BYTES      4
#define G_IM_SIZ_16b_TILE_BYTES 2
#define G_IM_SIZ_16b_LINE_BYTES 2
#define G_TEXTURE_IMAGE_FRAC    2
#define G_TEXTURE_SCALE_FRAC    16

#define G_IM_SIZ_32b_BYTES      4
#define G_IM_SIZ_32b_TILE_BYTES 2
#define G_IM_SIZ_32b_LINE_BYTES 2
#define G_TEXTURE_IMAGE_FRAC    2
#define G_TEXTURE_SCALE_FRAC    16

//=============== OBJ_GRAPHICS_COMMON ========================

#define CR_BASE_OFFSET 0x0
#define CR_BIG_BORDER_OFFSET 0x3020
#define CR_BORDER_OFFSET 0x3820
#define CR_LIGHTSLOT_OFFSET 0x38A0
#define CR_DARKSLOT_OFFSET 0x3930

#define BUTTON_B_CLOSE_OFFSET 0x39B8
#define BUTTON_R_EVIDENCE_OFFSET 0x3E58
#define BUTTON_R_PROFILES_OFFSET 0x42F8
#define LARROW_OFFSET 0x4798
#define RARROW_OFFSET 0x4838
#define BUTTON_R_COURTRECORD_OFFSET 0x48D8
#define BUTTON_L_PRESS_OFFSET 0x4D98
#define BUTTON_CUP_PRESENT_OFFSET 0x5258
#define BUTTON_B_CLOSE_IND_OFFSET 0x5D00
#define BUTTON_CONSULT_OFFSET 0x61A0

#define HEARTS_OFFSET 0x6698
#define HEARTS_DAMAGE_OFFSET 0x6810

#define EVIDENCE_FRAME_OFFSET 0x7208
#define POINTERHAND 0x76A8
#define CHECKMARK_OFFSET 0x78C8
#define MSGLOG_TRIFORCE_OFFSET 0x7968

#define CASE2_PICPART1 0x7E98
#define CASE2_PICPART2 0xF720
#define CASE2_PICPART3 0x16FA8

//================ OBJ_GRAPHICS_CASE1 =======================

#define PHOTO_OFFSET 0x0
#define INVENTORY_PART1 0x6468
#define INVENTORY_PART2 0xDE68
#define INVENTORY_PART3 0x15868
#define CRATES_PART1 0x1D268
#define CRATES_PART2 0x24C68
#define CRATES_PART3 0x2C668
#define SHOP_LIST 0x30468

//===========================================================

#define SHOP_LIST_XSIZE 192
#define SHOP_LIST_YSIZE 167
#define SHOP_LIST_POS_X (SCREEN_WIDTH / 2)
#define SHOP_LIST_POS_Y (SCREEN_HEIGHT / 2)

#define BARBUTTON_XSIZE 64
#define BARBUTTON_YSIZE 16
#define CONSULT_BUTTON_POSX (SCREEN_WIDTH / 2)
#define CONSULT_BUTTON_POSY 24

#define BARBUTTON_L_POS_X 64
#define BARBUTTON_R_POS_X 254
#define BARBUTTON_POS_Y CR_BASE_POSY + (CR_BASE_Y / 2) + 10

#define PHOTO_XSIZE 180
#define PHOTO_YSIZE 140
#define PHOTO_POS_X SCREEN_WIDTH / 2
#define PHOTO_POS_Y 100

#define BARBUTTON_PHOTO_POS_X PHOTO_POS_X
#define BARBUTTON_PHOTO_POS_Y PHOTO_POS_Y + PHOTO_YSIZE/2 + 8

#define ARROW_XSIZE 16
#define ARROW_YSIZE 16

#define ARROW_L_POS_X 22
#define ARROW_R_POS_X 299
#define ARROW_POS_Y 200

#define ARROW_L_POS_X_COURTRECORD CR_BASE_POSX - 119
#define ARROW_R_POS_X_COURTRECORD CR_BASE_POSX + 120
#define ARROW_POS_Y_COURTRECORD CR_BASE_POSY + 33

#define ROUNDBUTTON_XSIZE 64
#define ROUNDBUTTON_YSIZE 37

#define ROUNDBUTTON_R_POS_X SCREEN_WIDTH - (ROUNDBUTTON_XSIZE / 2) - 20
#define ROUNDBUTTON_R_POS_Y 30
#define ROUNDBUTTON_L_POS_X (ROUNDBUTTON_XSIZE / 2) + 20
#define ROUNDBUTTON_L_POS_Y 30

#define MIC_POS_X ROUNDBUTTON_R_POS_X
#define MIC_POS_Y ROUNDBUTTON_R_POS_Y + (ROUNDBUTTON_YSIZE / 2) + 8

#define HISTORY_POS_X ROUNDBUTTON_R_POS_X
#define HISTORY_POS_Y ROUNDBUTTON_R_POS_Y + (ROUNDBUTTON_YSIZE / 2) + 8


#define CUP_PRESENT_XSIZE 48
#define CUP_PRESENT_YSIZE 48
#define CUP_PRESENT_POS_X SCREEN_WIDTH / 2
#define CUP_PRESENT_POS_Y CR_BASE_POSY - CR_BASE_Y / 2 - 8

#define HEARTS_DAMAGE_SIZE 0x100
#define HEARTS_POS_DEFAULT -105 + (16 * (5 - gSaveContext.healthCapacity))
#define HEARTS_POS_SHOWING 5
#define HEARTS_POS_Y 20
#define HEARTS_MOVE_RATE 12
#define HEARTS_HEIGHT 16
#define HEARTS_WIDTH 16
#define HEARTS_DAMAGE_FRAMES_COUNT 8

#define EVIDENCE_FRAME_HEIGHT 48
#define EVIDENCE_FRAME_WIDTH 48
#define EVIDENCE_FRAME_POSX 36
#define EVIDENCE_FRAME_POSY 36

#define EVIDENCE_ICON_POSX (this->guiDrawIcon >= ITEM_MEDALLION_FOREST ? 24 : 20)
#define EVIDENCE_ICON_POSY EVIDENCE_ICON_POSX
#define EVIDENCE_ICON_X 64
#define EVIDENCE_ICON_Y 64

#define CHECKMARK_XSIZE 16
#define CHECKMARK_YSIZE 16

#define CASE2_PICPART1X 128
#define CASE2_PICPART1Y 240
#define CASE2_PICPART2X 128
#define CASE2_PICPART2Y 240
#define CASE2_PICPART3X 64
#define CASE2_PICPART3Y 240

#define CASE2_PICPART1_XPOS 128 / 2
#define CASE2_PICPART1_YPOS SCREEN_HEIGHT / 2

#define CASE2_PICPART2_XPOS 128 + (128 / 2)
#define CASE2_PICPART2_YPOS SCREEN_HEIGHT / 2

#define CASE2_PICPART3_XPOS 256 + (64 / 2)
#define CASE2_PICPART3_YPOS SCREEN_HEIGHT / 2

#define INVENTORY_PART1X 128
#define INVENTORY_PART1Y 240
#define INVENTORY_PART1_XPOS INVENTORY_PART1X / 2
#define INVENTORY_PART1_YPOS INVENTORY_PART1Y / 2

#define INVENTORY_PART2X 128
#define INVENTORY_PART2Y 240
#define INVENTORY_PART2_XPOS INVENTORY_PART1X + (INVENTORY_PART2X / 2)
#define INVENTORY_PART2_YPOS INVENTORY_PART2Y / 2

#define INVENTORY_PART3X 64
#define INVENTORY_PART3Y 240
#define INVENTORY_PART3_XPOS INVENTORY_PART1X + INVENTORY_PART2X + (INVENTORY_PART3X / 2)
#define INVENTORY_PART3_YPOS INVENTORY_PART3Y / 2

#define BARBUTTON_INVENTORY_POS_X SCREEN_WIDTH / 2
#define BARBUTTON_INVENTORY_POS_Y SCREEN_HEIGHT - (BARBUTTON_YSIZE  / 2) - 10

#define CRATES_PART1X 128
#define CRATES_PART1Y 240
#define CRATES_PART1_XPOS CRATES_PART1X / 2
#define CRATES_PART1_YPOS CRATES_PART1Y / 2

#define CRATES_PART2X 128
#define CRATES_PART2Y 240
#define CRATES_PART2_XPOS CRATES_PART1X + (CRATES_PART2X / 2)
#define CRATES_PART2_YPOS CRATES_PART2Y / 2

#define CRATES_PART3X 64
#define CRATES_PART3Y 240
#define CRATES_PART3_XPOS CRATES_PART1X + CRATES_PART2X + (CRATES_PART3X / 2)
#define CRATES_PART3_YPOS CRATES_PART3Y / 2

#define POINTERHANDX 32
#define POINTERHANDY 32

#define SPEAKER_LAST 10
#define FONT_SIZE 139
#define ICONBUF_SIZE 0x1000

#define EXPLOSION_CHAR_GRAVITY -2

#define MSG_LOG_SIZE 30         // This * sizeof(LoggedMsg) must be divisible by 16, else Wii VC explodes 
#define NPCMAKER_ACTORID 3

#define TEXTBOX_POS_Y_TOP 18
#define TEXTBOX_POS_Y_BOTTOM 168

#define STICK_DEADZONE 5
#define DPAD_PICKER_SPEED 6
#define STICK_PICKER_DIVIDER 12

#define MSGLOG_SPEAKERNAME_OFFSET 10
// 60 limit matches retail Lib_GetControlStickData()
#define MSGLOG_STICK_MAX 60
#define MSGLOG_STICK_OUTER_RING_MIN 45
#define MSGLOG_SCROLL_SPEED_DPAD_UD 10.0f
#define MSGLOG_SCROLL_SPEED_DPAD_LR 32.0f
#define MSGLOG_SCROLL_SPEED_STICK 0.28f
#define MSGLOG_SCROLL_DECEL_DPAD_UD 3.0f
#define MSGLOG_SCROLL_DECEL_DPAD_LR 5.0f
#define MSGLOG_SCROLL_DECEL_STICK 8.0f

#define CR_BASE_X 256
#define CR_BASE_Y 96
#define CR_BASE_POSX SCREEN_WIDTH / 2
#define CR_BASE_POSY (this->forcedPresent ? (SCREEN_HEIGHT / 2 - 25) : (SCREEN_HEIGHT / 2))

#define CR_BIG_BORDER_X  64
#define CR_BIG_BORDER_Y  64
#define CR_BIG_BORDER_POSX  CR_BASE_POSX - CR_BASE_X / 2 + CR_BIG_BORDER_X / 2 + 5
#define CR_BIG_BORDER_POSY  CR_BASE_POSY - CR_BASE_Y / 2 + CR_BIG_BORDER_Y / 2 + 5

#define CR_BORDER_X  16
#define CR_BORDER_Y  16
#define CR_BORDER_FIRSTPOSX  CR_BORDER_X + 45
#define CR_BORDER_FIRSTPOSY CR_BASE_POSY + 33

#define CR_LIGHTSLOT_X  16
#define CR_LIGHTSLOT_Y  16
#define CR_LIGHTSLOT_FIRSTPOSX  CR_BORDER_X + 45
#define CR_LIGHTSLOT_POSY CR_BASE_POSY + 33

#define CR_DARKSLOT_X  16
#define CR_DARKSLOT_Y  16
#define CR_DARKSLOT_FIRSTPOSX  CR_BORDER_X + 45
#define CR_DARKSLOT_POSY CR_BASE_POSY + 33

#define CR_TEXT_X CR_BASE_POSX - 52
#define CR_TEXT_Y CR_BASE_POSY - 39

#define CR_TEXT_MAX_XSIZE 172
#define CR_TEXT_MAX_YSIZE 60

#define CR_MAX_EVIDENCE_PER_PAGE 12
#define CR_STICK_COOLDOWN 3 * (float)((float)3 / (float)R_UPDATE_RATE);

#define evidenceProgress gSaveContext.swordHealth
#define gTextSpeed play->msgCtx.textboxBackgroundBackColorIdx

#define crMagic this->CrDataNpcMaker->scriptVars[0]
#define evidenceData ((CourtRecordEntry*)(this->CrDataNpcMaker->scriptVars[1]))
#define evidenceNum this->CrDataNpcMaker->scriptVars[2]
#define speakerData ((SpeakerEntry*)(this->CrDataNpcMaker->scriptVars[3]))
#define speakerNum this->CrDataNpcMaker->scriptVars[4]
#define crMagicEnd this->CrDataNpcMaker->scriptVars[5]

#define COURT_RECORD_MAGIC 0xABABCDCD
#define MSGLOG_MAGIC 0x12121212
#define MSGLOG_SUBTITLE_MAGIC 0xABCDDCBA

#define EVIDENCE_START 0
#define NPCMAKER_COURT_RECORD_DATA 0

#define OBJ_GRAPHICS_COMMON 3
#define OBJ_GRAPHICS_CASE1 154
#define OBJ_GRAPHICS_ICONS 7

#define MESSAGE_STATIC_VROM 0x008E6000

#define MSGLOG_TRIFORCE_X 32
#define MSGLOG_TRIFORCE_Y 32

#define SRAM_SIZE 0x8000
#define SRAM_HEADER_SIZE 0x10
#define SLOT_SIZE (sizeof(SaveContext) + 0x28)
#define SLOT_OFFSET(index) (SRAM_HEADER_SIZE + 0x10 + (index * SLOT_SIZE))

#define CR_ALPHA_NONE 0
#define CR_ALPHA_FULL 255
#define CR_ALPHA_DELTA 50

#define GUI_ALPHA_NONE 0
#define GUI_ALPHA_FULL 255
#define GUI_ALPHA_DELTA 70

#define EVIDENCE_ALPHA_NONE 0
#define EVIDENCE_ALPHA_FULL 255
#define EVIDENCE_ALPHA_DELTA 100

#define SPEAKER_INDICATOR_ALPHA_NONE 0
#define SPEAKER_INDICATOR_ALPHA_FULL 200
#define SPEAKER_INDICATOR_TEXT_ALPHA_NONE 0
#define SPEAKER_INDICATOR_TEXT_ALPHA_FULL 255

#define EVIDENCE_INVALID -1

//#define USE_INTERNAL_BUFFER
#define SAVE_MSGLOG

#define NUM_BUFFERS 2
#define GRAPHICS_BUF_SIZE 0x18000

#define SMALL_GFX_TEST 10

#define ACTOR_VRS 4

typedef struct {
    /* 0x00000 */ u16 headMagic; // GFXPOOL_HEAD_MAGIC
    /* 0x00008 */ Gfx polyOpaBuffer[0x4FE0];
    /* 0x27F08 */ Gfx polyXluBuffer[0x800];
    /* 0x2BF08 */ Gfx overlayBuffer[0x400];
    /* 0x2DF08 */ Gfx workBuffer[0x80];
    /* 0x2E308 */ Gfx unusedBuffer[0x1F];
                  Gfx* polyOpaOverride;
                  u32 polyOpaOverrideSize;
    /* 0x2E408 */ u16 tailMagic; // GFXPOOL_TAIL_MAGIC
} MyGfxPool; // size = 0x2E410

MyGfxPool* gfxPools = (MyGfxPool*)0x8016A640;

typedef struct CourtRecordEntry
{
    u8 id;
    u8 list;
    u8 showOnProgress;
    u8 guiOnA;
    u8 msgId;
    u8 scale;

} CourtRecordEntry;

typedef struct LoggedMsg
{
    char message[200];
    int speakerId;
    int msgChoice;
    int msgId;
    int msgYSize;
} LoggedMsg;

typedef struct SpeakerEntry
{
	u8 id;
    bool disableIndicator;
    u16 sndID;
	f32 sndPitch;
	Color_RGB8 textboxColor;
	Color_RGB8 textColor;
	Color_RGB8 textShadowColor;
	char* text;    
} SpeakerEntry;

typedef struct UIStruct
{
    Actor actor;
    u8 guiSpeaker;
    u8 guiToEnable;
    u8 guiArrows;
    u8 guiSubtitle;
    u8 guiShowHearts;
    s8 guiDrawIcon;
    u8 guiShowHeartsDamageFrame;
    u8 guiDrawCheckmark;
    u8 guiCEStatementCount;
    u8 guiCECurStatement;
    u8 guiPickerEnabled;
    u8 guiExplodeTextboxStatus;
    u8 guiMsgLoggingDisabled;
    u8 guiMsgLogStatus;
    u8 guiMsgLogCurrentScene;
    u8 guiCourtRecordStatus;
    s8 guiCourtRecordPlayerPresented;
    u8 guiCourtRecordMode;
    s8 guiCourtRecordListOnly;
    s8 guiSpeakerForce;
    s8 guiLogSubtitle;
    u8 guiShowConsult;
    u8 forcedPresent;
    s8 forceMicShow;
    s16 guiPickerPosX;
    s16 guiPickerPosY;
    SpeakerEntry* curSpeakerData;
    SpeakerEntry* curTestimonySpeakerData;

// Only add new stuff below; the above struct must stay intact unless adjusted in npcmaker

    u8 speakerLastFrame;
    u8 arrowSineCounter;
    u8 guiLastEffectiveGui;
    u8 guiEffective;
    u8 guiAlphaDir;
    u8 explodedTextboxLength;
    u8 msgLogRingBufferPos;
    u8 guiCourtRecordShowingList;
    s8 crPickPos;
    u8 crPickPage;
    u8 crPickPageMax;
    u8 crPickEvidenceMax;
    u8 crCooldown;
    u8 firstInput;
    s8 crSelectedIdLast;
    u8 wasShowingHearts;
    u8 guiDrawIconEffective;
    s16 heartsPos;
    s16 curSpeakerTextAlpha;
    s16 curSpeakerTextboxAlpha;
    s16 guiAlpha;
    s16 crAlpha;
    s16 evidenceAlpha;
    int msgLogYSize;
    int msgLogPosition;
    float msgLogSpeed;
    float msgLogAccel;
    Color_RGB8 lastTextColor;
    s16 lastTextPrimAlpha;
    CourtRecordEntry* selectedCREntry;

    char* msgBufDecodedCopy;
    char* msgBufCR;
    u8* arrowGraphic;
    Vec2s* textDrawPositions;
    Vec2s* textMovementVectors;
    NpcMaker* CurScene;
    NpcMaker* CrDataNpcMaker;
    VoiceMgr* VoiceM;
    LoggedMsg* msgLog;
    
#ifdef USE_INTERNAL_BUFFER
    int bufCur;
    Gfx* gfxBuffers[NUM_BUFFERS];
    TwoHeadGfxArena uiGraphics;
#endif

#ifdef USE_POLYOPA_OVERRIDE
    Gfx* gfxBuffers[NUM_BUFFERS];
#endif

} UIStruct;

typedef enum
{
    LIST_EVIDENCE = 0,
    LIST_PROFILES = 1,
} CourtRecordLists;

typedef enum
{
    COURTRECORD_MODE_NORMAL = 0,
    COURTRECORD_MODE_PRESENT = 1,
} CourtRecordMode;

typedef enum
{
    UIACTOR_CR_IDLE = 0,
    UIACTOR_CR_OPENING = 1,
    UIACTOR_CR_OPEN = 2,
    UIACTOR_CR_SWITCHING_OUT = 3,
    UIACTOR_CR_SWITCHING_IN = 4,
    UIACTOR_CR_CLOSING = 5,
    UIACTOR_CR_CHECKING_OPEN = 6,
    UIACTOR_CR_CHECKING = 7,
    UIACTOR_CR_CHECKING_CLOSE = 8,
} CourtRecordState;

typedef enum
{
    UIACTOR_MSGLOG_IDLE = 0,
    UIACTOR_MSGLOG_OPENED = 1,
    UIACTOR_MSGLOG_OPEN = 2,
} MsgLogState;

typedef enum
{
    UIACTOR_EXPLOSION_IDLE = 0,
    UIACTOR_EXPLOSION_START = 1,
    UIACTOR_EXPLOSION_ONGOING = 2,
    UIACTOR_EXPLOSION_COMPLETE = 3
} TextboxExplosionState;


typedef enum
{
    SPEAKER_COMMON_FIRST = 249,
    SPEAKER_TYPEWRITER = 250,
    //SPEAKER_NOLOG can be used to mark messages that should not be logged.
    //It is also used internally to log scene breaks in the message log.
    //I.e SPEAKER_NOLOG messages should not appear normally in the log,
    //But if one does, then it's a scene break message.
    SPEAKER_NOLOG = 251,
    SPEAKER_CROSS_EXAM = 252,
    SPEAKER_REBUTTAL = 253,
    SPEAKER_UNKNOWN = 254,
    SPEAKER_NONE = 255
} SpeakerType;

typedef enum
{
    HEARTS_NONE = 0,
    HEARTS_IN = 1,
    HEARTS_OUT = 2,
    HEARTS_SHOWING = 3,
    HEARTS_DAMAGE = 4,
} HeartsShowType;

typedef enum
{
    SUBTITLE_NONE = 0,
    SUBTITLE_HOLDIT = 3,
    SUBTITLE_OBJECTION = 4,
    SUBTITLE_TAKETHAT = 5,
    SUBTITLE_WITNESST =  6,
    SUBTITLE_CROSSE = 7,
    SUBTITLE_GUILTY = 8,
    SUBTITLE_NGUILTY = 9,
} GUISubtitle;

typedef enum
{
    GUI_NONE = 0,
    GUI_INVESTIGATION = 1,
    GUI_COURTRECORD_EVIDENCE = 2,
    GUI_COURTRECORD_PROFILES = 3,
    GUI_CROSS_EXAMINATION_EVIDENCE = 4,
    GUI_CROSS_EXAMINATION_PROFILES = 5,
    GUI_PRESENT = 6,
    GUI_PHOTO = 7,
    GUI_PHOTO_EVIDENCE = 8,
    GUI_CROSS_EXAMINATION = 9,
    GUI_CASE2PIC = 10,
    GUI_INVENTORY = 11,
    GUI_INVENTORY_EVIDENCE = 12,
    GUI_CRATES = 13,
    GUI_SHOPLIST = 14,
    GUI_SHOPLIST_EVIDENCE = 15,
} GUIType;

typedef enum
{
    ARROWS_NONE = 0,
    ARROWS_RIGHT = 1,
    ARROWS_LEFT = 2,
    ARROWS_BOTH = 3,
} GUIArrows;

typedef enum
{
    DIR_NONE = 0,
    DIR_OUT = 1,
    DIR_IN = 2,
} GUIAlphaDir;

SpeakerEntry commonSpeakers[] =
{	
	{
		.id = SPEAKER_NOLOG,
        .disableIndicator = true,
		.text = "Skawo",
		.sndID = 0,
		.sndPitch = 1.0f,
	},
	{
		.id = SPEAKER_TYPEWRITER,
        .disableIndicator = true,
		.text = "ParallelSkawo",
		.sndID = 0x0F01,
		.sndPitch = 1.0f,
	},
	{
		.id = SPEAKER_CROSS_EXAM,
		.text = "99 / 99",
		.textboxColor = {.r = 201, .g = 201, .b = 201},
		.textColor = {.r = 0, .g = 0, .b = 0},
		.textShadowColor = {.r = 255, .g = 255, .b = 255},
		.sndID = 0,
		.sndPitch = 1.0f,
	},
	{
		.id = SPEAKER_REBUTTAL,
		.text = "Argument 99",
		.textboxColor = {.r = 59, .g = 3, .b = 92},
		.textColor = {.r = 255, .g = 255, .b = 255},
		.textShadowColor = {.r = 0, .g = 0, .b = 0},
		.sndID = 0,
		.sndPitch = 1.0f,
	},
	{
		.id = SPEAKER_UNKNOWN,
		.text = "????",
		.textboxColor = {.r = 201, .g = 201, .b = 201},
		.textColor = {.r = 0, .g = 0, .b = 0},
		.textShadowColor = {.r = 255, .g = 255, .b = 255},
		.sndID = 0,
		.sndPitch = 1.0f,
	},   
};

char* subtitles[] =
{
    "(Hold it!)",
    "(Objection!)",
    "(Take that!)",
    "(Witness Testimony)",
    "(Cross-Examination)",
    "(Guilty)",
    "(Not Guilty)",
    "(Rebuttal)",
    "(Argument Complete)",
};

char* msgLogEnd = "--                    --";
char* consultMsg = "\x05""\x46""\xA7""\x05""\x40"" Consult\x02";
char sceneBreakMsg[] = "SCENEBREAK";
char* micMsg = "\x05""\x46""\xA6""\x05""\x40"" MIC\x02";

char* micMsgJpn[] = { "\"IGIARI!\"", "\"MATTA!\"", "\"KURAE!\"" };
char* micMsgEng[] = { "\"OBJECTION!\"", "\"HOLD IT!\"", "\"TAKE THAT!\"" };

char* logMsg = "\x05""\x41""\xF8""\x05""\x40"" History\x02";

Color_RGB8 explosionTextColor = {.r = 70, .g = 255, .b = 80};
Color_RGBA8 msgLogBackgroundColor = {.r = 0, .g = 0, .b = 0, .a = 127};
Color_RGB8 msgLogSpeakerNameColor = {.r = 200, .g = 170, .b = 90};
Color_RGB8 msgLogEndColor = {.r = 105, .g = 110, .b = 190};
Color_RGB8 msgLogPickedChoiceArrowColor = {.r = 255, .g = 223, .b = 0};
Color_RGB8 colorWhite = {.r = 255, .g = 255, .b = 255};
Color_RGB8 colorBlack = {.r = 0, .g = 0, .b = 0};
Color_RGB8 colorRed = {.r = 255, .g = 0, .b = 0};
Color_RGB8 colorBlue = {.r = 0, .g = 0, .b = 255};
Color_RGB8 colorGreen = {.r = 0, .g = 255, .b = 0};

extern void THGA_Init_New(TwoHeadGfxArena* thga, void* start, size_t size);
    asm("THGA_Init_New = 0x8009FEC0");

extern void Environment_FillScreen(GraphicsContext* gfxCtx, u8 red, u8 green, u8 blue, u8 alpha, u8 drawFlags);
	asm("Environment_FillScreen = 0x800625B0");
    
extern void SsSram_ReadWrite(u32 addr, void* dramAddr, size_t size, s32 direction);
    asm("SsSram_ReadWrite = 0x80091474");

void MsgLogControls(Actor* thisx, PlayState* play);
int GetMessageTextYSize(Actor* thisx, PlayState* play, char* msgData);
int CalculateMsgLogHeight(Actor* thisx, PlayState* play);
void DrawMsgLog(Actor* thisx, PlayState* play, Gfx** gfxp);
void TryLogMessage(Actor* thisx, PlayState* play);
void DrawMessageText(Actor* thisx, PlayState* play, Gfx** gfxp, Color_RGB8 Color, s16 alpha, char* msgData, int posX, int posY);
bool DrawMessageTextIndividual(Actor* thisx, PlayState* play, Gfx** gfxp, Color_RGB8 Color, s16 alpha);
void GetMessageTextPositions(Actor* thisx, PlayState* play, char* msgData, int startPosX, int startPosY);
int DrawMessageTextInternal(Actor* thisx, PlayState* play, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, s16 alpha, s16 shadowAlpha, char* msgData, int posX, int posY, u8 shadowOffsetX, u8 shadowOffsetY, float scale, int operation);
void DrawSpeakerIndicator(Actor* thisx, PlayState* globalCtx, Gfx** gfxp);
void DrawText(Actor* thisx, PlayState* globalCtx, Gfx** gfxp, Color_RGB8 Color, Color_RGB8 ShadowColor, char* Name, s32* XPos, s32* YPos, s16 alpha);
void DrawSubtitle(Actor* thisx, PlayState* globalCtx, Gfx** gfxp);
void DrawCourtRecord(Actor* thisx, PlayState* globalCtx, Gfx** gfxp);
void DrawItemIcon(Actor* thisx, PlayState* play, Gfx** p, int x, int y, u8* tex);
int GetMaxEvidenceID(Actor* thisx);
void GameUI_SetTextboxPosition(Actor* thisx, PlayState* play);
void TryLogMessage(Actor* thisx, PlayState* play);
void DrawUIElements(Actor* thisx, PlayState* play, Gfx** gfxp);
void UpdateCourtRecord(Actor* thisx, PlayState* play);
void StoreMessageInLog(Actor* thisx, PlayState* play, char* data, int size, int msgId, int speaker);
void GetCRDataNpcMaker(Actor* thisx, PlayState* play);
SpeakerEntry* GetSpeakerEntry(Actor* thisx, PlayState* play, int entry);
NpcMaker* GetNpcMakerByID(PlayState* playState, u16 ID);
Actor* GetActorByID(PlayState* playState, u16 ID);

#endif
