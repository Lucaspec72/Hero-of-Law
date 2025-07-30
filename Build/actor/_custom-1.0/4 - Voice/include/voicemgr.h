#ifndef _Z_VOICEMGR_H_
#define _Z_VOICEMGR_H_

#define GET_DIMENSIONS_FUNCS

#include <z64hdr/oot_u10/z64hdr.h>
#include "PR/controller_voice.h"
#include "z64voice.h"
#include "../../draw2D.h"
#include "../../holText.h"

#define SEQCMD_SET_SEQPLAYER_VOLUME(seqPlayerIndex, duration, volume)\
    Audio_QueueSeqCmd((0x4 << 28) | ((u8)(seqPlayerIndex) << 24) | ((duration) << 16) | \
                      (volume))
                     
void PadMgr_InitVoice(void);
int GetStringDrawLen(char* string, float scale);
int GetStringScreenXCenter(char* string, float scale);
void SetMusicVolume(PlayState* play, float volume, float frames);
bool IsVRU(OSMesgQueue* mq, OSVoiceHandle* hd, int channel);
bool PadMgr_GetRegion(void);
void SetMusicVolume(PlayState* play, float volume, float frames);
s32 AudioVoice_IsRelevantError(s32 errorCode);
#define SAVE_DEBUGMODE gSaveContext.isDoubleDefenseAcquired

//#define DISABLE_VRS

typedef enum {
    /* 0 */ VOICE_INIT_FAILED, 
    /* 1 */ VOICE_INIT_TRY,  
    /* 2 */ VOICE_INIT_SUCCESS 
} VoiceInitStatus;

typedef enum {
    /* 0 */ VOICE_STATE_START,
    /* 1 */ VOICE_STATE_LISTENING,
    /* 2 */ VOICE_STATE_GOTWORD,
    /* 3 */ VOICE_STATE_AWAIT_RESTART,    
} VoiceState;

typedef struct VoiceMgr
{
    Actor actor;
    u8 lastWord;
    u8 listening;
    u8 detectionTimer;
    u8 disabled;
    u8 region;
    u8 showDebugTemp;
    u8 soundStatus;
    u8 needsReinit;
    u8 timer;
    u8 turnedOff;
} VoiceMgr;

typedef enum OSVoiceWordId {
    /*  0 */ VOICE_WORD_ID_IGIARI,
    /*  1 */ VOICE_WORD_ID_MATTA,
    /*  2 */ VOICE_WORD_ID_KURAE,
    /*  3 */ VOICE_WORD_ID_MAX,
    /* -1 */ VOICE_WORD_ID_NONE = 0xFFFF
} OSVoiceWordId;

OSVoiceDictionary sVoiceDictionary = {
    {
        // "Igiari"
        { 0x8343, 0x834d, 0x8341, 0x838a },
        // "Matta"
        { 0x837d, 0x835e },
        // "Kurae"
        { 0x834e, 0x8389, 0x8347},       
    },

    VOICE_WORD_ID_MAX,
};

// Sum of frame counts from the VRU Indices List 
// (https://pastebin.com/ajLzRLze)
u16 USAWordsFrames[] = { 0x0016, 0x0010, 0x000F };

OSVoiceDictionary sVoiceDictionaryEnglish = {
    {
        // "Objection"
        { 
            0x0099,  // "a" ("away")
            0x0372,  // "b" ("baked")
            0x0366,  // "j" ("job")
            0x0069,  // "e" ("let's")
            0x03C9,  // "c" ("cute")
            0x03FF,  // "sh" ("fishing")
            0x0045,  // "i" ("sticky")
            0x033C   // "n" ("golden")
        },
        // "Hold it"
        { 
            0x0411,  // "h" ("home")
            0x00CF,  // "o" ("home")
            0x02EE,  // "l" ("golden")
            0x0381,  // "d" ("golden")
            0x0045,  // "i" ("it")
            0x03B7   // "t" ("it")
        },
        // "Take that"
        {
            0x03BA,  // "t" ("taste")
            0x0192,  // "a" ("taste")
            0x03C9,  // "k" ("cute")
            0x036C,  // "th" ("that")
            0x0087,  // "a" ("that")
            0x0003,  // precedes "t" ("that")
            0x03C6   // "t" ("that")
        },
    },

    VOICE_WORD_ID_MAX,
};

#endif
