#ifndef _Z_TITLE_H_
#define _Z_TITLE_H_

#define SAVE_STUFF
#define GET_DIMENSIONS_FUNCS

#include <z64hdr/oot_u10/z64hdr.h>
#include "../../draw2D.h"
#include "../../holText.h"
#include "../../common.h"
#include "../../5 - TransferPak/include/trsPakMgr.h"
#include "../../6 - BioSensor/include/bioSnsrMgr.h"
#include "../../3 - NPC Maker/include/npc_maker_types.h"

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define FLAGS 0x00000030
#define LOGO_OFFSET 0x0
#define COPYRIGHT_OFFSET 0x19000

#define FADEIN_SPEED 6
#define TEXT_FADE_SPEED 15
#define FADEOUT_SPEED 25
#define FADEIN_FRAMES_START 30
#define LOGO_ALPHA_TARGET 245
#define TEXT_ALPHA_TARGET_FULL 255
#define TEXT_ALPHA_TARGET_NONE 0
#define TEXT_FADE_SPEED_PAUSE_DURATION 20
#define INPUT_COOLDOWN_DURATION 3

#define ROUTE_COURTROOM 0x50
#define ROUTE_TALONGAME 0x70
#define ROUTE_CREDITS 0x25
#define ROUTE_WANTED 0xE0
#define ROUTE_CASINO 0x80

#define GBCAMERA_EASTEREGG_LENGTH 125

#define NPCMAKER_FILE 120

#define NPCMAKER_ACTOR_CONTROLLER 10
#define NPCMAKER_ACTOR_SETTINGS 108
#define NPCMAKER_ACTOR_OVERWRITEQ 110
#define NPCMAKER_ACTOR_CORRUPTED 111
#define NPCMAKER_ACTOR_SCENE_SELECT 112

#define COLOR_HIGHLIGHT      (Color_RGB8) {0xFF, 0xFF, 0x43}
#define COLOR_NO_HIGHLIGHT   (Color_RGB8) {0xA6, 0xA6, 0xA6}

#define TEXT_SCALE_TITLE 100
#define TEXT_SCALE_OPTIONS 65

char sDebugVersion[] = "DEBUG VERSION";

char* ContinueString[] =
{
    "CONTINUE",
    "CONTINUER"
};

char* NewGameString[] = 
{
    "NEW GAME",
    "NOUVELLE PARTIE"
};

char* SceneSelectString[] = 
{
    "SCENE SELECT / EXTRAS",
    "CHOIX DE LA SC\x94""NE / EXTRAS"
};

char StringVERSION[] = "v1.10";

char* StringSETTINGS[] = 
{
    "Settings  \xA3""",
    "Param\x85""tres  \xA3""",
};

char BUILDUSERSTRING[] = "BUILDUSERBUILDUSERBUILDUSERBUILDUSE";
char* gbCamEasterEggStrings[] = 
{
    "What are you doing...?",
    "Why, hello there!",
    "Lights, camera, action!",
    "Be not afraid...",
    "KONAMI Code + Dragmire",
    "Very, very interesting...",
    
    "Que faites-vous... ?",
    "Oh, bonjour !",
    "Silence, on tourne !\x01""Moteur... Action !",
    "N'ayez crainte...",
    "Code KONAMI + Dragmire",
    "Int\x84""ressant. Tr\x85""s int\x84""ressant...",

};

char* languageStrings[] = 
{
    "\xA2""  Language\x02",
    "\xA2""  Langue\x02"
};

#define FONT_RELOAD_LENGTH 5

void TitleLogo_DrawSceneSelect(Actor* thisx, PlayState* play);
void TitleLogo_Draw(Actor* thisx, PlayState* play);
void TitleLogo_GoTalonGame(Actor* thisx, PlayState* play);
void ProcessTransferPak(Actor* thisx, PlayState* play);
void TitleLogo_GoIngoCasino(Actor* thisx, PlayState* play);
void TitleLogo_GoTalonGame(Actor* thisx, PlayState* play);
void TitleLogo_Draw_Settings(Actor* thisx, PlayState* play);

void TitleLogo_Update_SceneSelect(Actor* thisx, PlayState* play);
void TitleLogo_Update_SaveCorrupted(Actor* thisx, PlayState* play);
void TitleLogo_Update_Overwrite(Actor* thisx, PlayState* play);
void TitleLogo_Update_Settings(Actor* thisx, PlayState* play);
void SetTrsPakDisableStatus(Actor* thisx, bool status);


void HaltCutscene(PlayState* play);

#define SCENE_SELECT_CHOICE_CANCEL 0x8000
#define SCENE_SELECT_CHOICE_TALONGAME 0x8001
#define SCENE_SELECT_CHOICE_CASINO 0x8002

typedef struct TitleLogo
{
    Actor actor;
	
    s16 globalState;
    s16 mainAlpha;
    s16 copyrightAlpha;
    s16 textAlpha;
    s16 textAlphaFull;
    s16 textFadeDirection;
    s16 stateDelayTimer;
    s16 inputPauseTimer;
    u8 hasFile;
    u8 highlightedOption;
    u8 stopTextAlphaCounter;
    s8 selectedScene;
    u8 sceneSelectRed;
    u8 drawGbCamEasterEgg;
    u8 stringSlot;
    u8 initialTimer;
    u8 fontReloadTimer;
    bool saveCorrupted;
    s16 gbCamEasterEggAlpha;
    TrsPakMgr* trsPak;
    BioSnsrMgr* bioSnsr;
    NpcMaker* aux; 
} TitleLogo;

typedef enum
{
    OPTION_NEWGAME,
    OPTION_CONTINUE,
    OPTION_STAGESELECT,
} TitleLogoOptions;

typedef enum
{
    TITLESCREEN_STATE_INITIAL,
    TITLESCREEN_STATE_FADE_IN,
    TITLESCREEN_STATE_DISPLAY,
    TITLESCREEN_STATE_FADE_OUT,
    TITLESCREEN_STATE_POST_DISPLAY,
    TITLESCREEN_STATE_SCENE_SELECT,
    TITLESCREEN_STATE_STARTING_GAME,
} TitleLogoGlobalState;

typedef enum
{
    SCENE_ID_TITLE = 0x36,
    SCENE_ID_COURT = 0x38,
    SCENE_ID_LOBBY = 0x35,
    SCENE_ID_TALONGAME = 0x62,
    SCENE_ID_RANCH = 0x37,
    SCENE_ID_THONK = 0x3A,
    SCENE_ID_CREDITS = 0x26,
    SCENE_ID_OFFICE = 0x31,
} enumSceneID;

typedef enum
{
    PROGRESS_START = 0,
    PROGRESS_LOBBY_INTRO = 1,
    PROGRESS_COURT_OPENINGSTATEMENT = 2,
    PROGRESS_COURT_IMPAENTERS = 3,
    PROGRESS_COURT_IMPA1 = 4,
    PROGRESS_COURT_IMPA1_POST = 5,
    PROGRESS_COURT_IMPA_CE = 6,
    PROGRESS_COURT_ZELDAENTERS = 7,
    PROGRESS_COURT_ZELDA1 = 8,
    PROGRESS_COURT_ZELDA1_POST = 9,
    PROGRESS_COURT_ZELDA1_CE = 10,
    PROGRESS_COURT_ZELDA2 = 11,
    PROGRESS_COURT_ZELDA2_POST = 12,
    PROGRESS_COURT_ZELDA2_CE = 13,
    PROGRESS_COURT_ZELDANEWEVIDENCE = 14,
    PROGRESS_COURT_ZELDA3 = 15,
    PROGRESS_COURT_ZELDA3_POST = 16,
    PROGRESS_COURT_ZELDA3_CE = 17,
    PROGRESS_LOBBY_RECESS = 18,
    PROGRESS_COURT_INGOENTERS = 19,
    PROGRESS_COURT_INGO1 = 20,
    PROGRESS_COURT_INGO1_POST = 21,
    PROGRESS_COURT_INGO1_CE = 22,
    PROGRESS_COURT_INGO2 = 23,
    PROGRESS_COURT_INGO2_POST = 24,
    PROGRESS_COURT_INGO2_CE = 25,
    PROGRESS_UBER_THONK = 26,
    PROGRESS_INGO_PURSUIT = 27,
    PROGRESS_INGO_PURSUIT2 = 28,
    PROGRESS_COURT_INGO3 = 29,
    PROGRESS_COURT_INGO3_POST = 30,
    PROGRESS_COURT_INGO3_CE = 31,
    PROGRESS_DRAGMIRE_REBUTTAL = 32,
    PROGRESS_DRAGMIRE_REBUTTAL_AFTER = 33,
    PROGRESS_FLASHBACK = 34,
    PROGRESS_VERDICT = 35,
    PROGRESS_EPILOGUE = 36,
    PROGRESS_CREDITS = 37,

    PROGRESS_EXTRA_ZELDA = 38,

    PROGRESS_EXTRA_THIEF_TRSPAKCHECK = 39,
    PROGRESS_EXTRA_THIEF_TRIALSTART = 40,
    PROGRESS_EXTRA_THIEF_SHOPK_ENTERS = 41,
    PROGRESS_EXTRA_THIEF_SHOPK_TESTIMONY = 42,
    PROGRESS_EXTRA_THIEF_SHOPK_POST = 43,
    PROGRESS_EXTRA_THIEF_SHOPK_CE = 44,
    PROGRESS_EXTRA_THIEF_OWL_TESTIMONY = 45,
    PROGRESS_EXTRA_THIEF_OWL_POST = 46,
    PROGRESS_EXTRA_THIEF_OWL_CE = 47,
    PROGRESS_EXTRA_THIEF_THONKING = 48,
    PROGRESS_EXTRA_THIEF_VERDICT = 49,
    PROGRESS_EXTRA_THIEF_VERDICT2 = 50,
    PROGRESS_EXTRA_THIEF_EPILOGUE = 51,
} enumProgress;

typedef struct 
{
    u8 idx;
    u8 progress;
    char* name;
    u8 extraScene;
} sSelectEntry;

#define EXTRA_TALONGAME 24
#define EXTRA_ZELDA 25
#define EXTRA_THIEF 26

sSelectEntry SceneSelectData[] = 
{
    {
        .idx = 1,
        .progress = PROGRESS_START,
        .name = "Intro Scene",
    },
    {
        .idx = 2,
        .progress = PROGRESS_LOBBY_INTRO,
        .name = "Before the Trial",
    },
    {
        .idx = 3,
        .progress = PROGRESS_COURT_OPENINGSTATEMENT,
        .name = "The Trial Begins",
    },
    {
        .idx = 4,
        .progress = PROGRESS_COURT_IMPAENTERS,
        .name = "Impa Enters",
    },
    {
        .idx = 5,
        .progress = PROGRESS_COURT_IMPA1,
        .name = "Impa's Testimony",
    },
    {
        .idx = 6,
        .progress = PROGRESS_COURT_ZELDAENTERS,
        .name = "Zelda Enters",
    },
    {
        .idx = 7,
        .progress = PROGRESS_COURT_ZELDA1,
        .name = "Zelda's First Testimony",
    },
    {
        .idx = 8,
        .progress = PROGRESS_COURT_ZELDA2,
        .name = "Zelda's Second Testimony",
    },
    {
        .idx = 9,
        .progress = PROGRESS_COURT_ZELDANEWEVIDENCE,
        .name = "Zelda's New Evidence",
    },
    {
        .idx = 10,
        .progress = PROGRESS_COURT_ZELDA3,
        .name = "Zelda's Third Testimony",
    },    
    {
        .idx = 11,
        .progress = PROGRESS_LOBBY_RECESS,
        .name = "Recess",
    },      
    {
        .idx = 12,
        .progress = PROGRESS_COURT_INGOENTERS,
        .name = "Ingo Enters",
    },     
    {
        .idx = 13,
        .progress = PROGRESS_COURT_INGO1,
        .name = "Ingo's First Testimony",
    },     
    {
        .idx = 14,
        .progress = PROGRESS_COURT_INGO2,
        .name = "Ingo's Second Testimony",
    },     
    {
        .idx = 15,
        .progress = PROGRESS_UBER_THONK,
        .name = "Deep Thought",
    },     
    {
        .idx = 16,
        .progress = PROGRESS_INGO_PURSUIT2,
        .name = "Accusing Ingo",
    },                       
    {
        .idx = 17,
        .progress = PROGRESS_COURT_INGO3,
        .name = "Ingo's Third Testimony",
    },      
    {
        .idx = 18,
        .progress = PROGRESS_DRAGMIRE_REBUTTAL,
        .name = "Dragmire's Rebuttal",
    },          
    {
        .idx = 19,
        .progress = PROGRESS_DRAGMIRE_REBUTTAL_AFTER,
        .name = "The Truth Revealed",
    },              
    {
        .idx = 20,
        .progress = PROGRESS_FLASHBACK,
        .name = "A Flashback",
    },          
    {
        .idx = 21,
        .progress = PROGRESS_VERDICT,
        .name = "The Verdict",
    },          
    {
        .idx = 22,
        .progress = PROGRESS_EPILOGUE,
        .name = "Epilogue",
    },               
    {
        .idx = 23,
        .progress = PROGRESS_CREDITS,
        .name = "Credits",
    },    
    {
        .idx = EXTRA_TALONGAME,
        .name = "EXTRA 1: Meanwhile, Somewhere Else...",
        .extraScene = (1 << 0),
    },   
    {
        .idx = EXTRA_ZELDA,
        .name = "EXTRA 2: Ingo's Casino",
        .extraScene = (1 << 1),
    }, 
    {
        .idx = EXTRA_THIEF,
        .progress = PROGRESS_EXTRA_THIEF_TRSPAKCHECK,
        .name = "EXTRA 3: Turnabout Thief",
        .extraScene = (1 << 2),
    },     
};

#if DEBUGVER == 1

    typedef struct 
    {
        char* name;
        s32 entranceIndex;
        int cutsceneIndex;
    } debugSceneSelectEntry;

    static debugSceneSelectEntry sScenes[] =
    {
        { .name = "Scene Select", .entranceIndex = ROUTE_COURTROOM, .cutsceneIndex = 0xFFF0 },
        { .name = "Talon Game", .entranceIndex = ROUTE_TALONGAME, .cutsceneIndex = 0xFFF0  },
        { .name = "Unused", .entranceIndex = ROUTE_WANTED, .cutsceneIndex = 0xFFF0  },
        { .name = "Casino", .entranceIndex = ROUTE_CASINO, .cutsceneIndex = 0xFFF0 },
        { .name = "Credits", .entranceIndex = ROUTE_CREDITS, .cutsceneIndex = 0xFFF1 },
    };

#endif


extern void SsSram_ReadWrite(u32 addr, void* dramAddr, size_t size, s32 direction);
    asm("SsSram_ReadWrite = 0x80091474");

extern void Audio_StopBGMAndFanfares(u16 FadeoutDur);
    asm("Audio_StopBGMAndFanfares = 0x800C77D0");
    
void TitleLogo_InitNewSave();
void TitleLogo_InvalidateMsgLogChecksum();
int mStrlen(const char *str);
void TitleLogo_DrawLogo(Actor* thisx, PlayState* play, Gfx** gfxp);
int GetStringDrawnLength(char* string, float scale);
int GetStringCenterX(char* string, float scale);
int GetScreenCenterY(int Size);
void TitleLogo_DrawSceneSelect(Actor* thisx, PlayState* play);   
void TitleLogo_NewGame();

#endif
