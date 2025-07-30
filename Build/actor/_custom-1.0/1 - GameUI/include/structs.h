#ifndef _Z_GAMEUISTRUCTS_H_
#define _Z_GAMEUISTRUCTS_H_

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

typedef struct SpeakerEntry
{
	u8 id;
    bool disableIndicator;
    u16 sndID;
	f32 sndPitch;
	Color_RGB8 textboxColor;
	Color_RGB8 textColor;
	Color_RGB8 textShadowColor;
	u8 textID;    
} SpeakerEntry;

#define UI_STRUCT_FIELDS \
    Actor actor; \
    u8 guiSpeaker; \
    u8 guiToEnable; \
    u8 guiArrows; \
    u8 guiSubtitle; \
    u8 guiShowHearts; \
    s8 guiDrawIcon; \
    u8 guiShowHeartsDamageFrame; \
    u8 guiDrawCheckmark; \
    u8 guiCEStatementCount; \
    u8 guiCECurStatement; \
    u8 guiPickerEnabled; \
    u8 guiExplodeTextboxStatus; \
    u8 guiMsgLoggingDisabled; \
    u8 guiMsgLogStatus; \
    u8 guiMsgLogCurrentScene; \
    u8 guiCourtRecordStatus; \
    s8 guiCourtRecordPlayerPresented; \
    u8 guiCourtRecordMode; \
    s8 guiCourtRecordListOnly; \
    s8 guiSpeakerForce; \
    s8 guiLogSubtitle; \
    u8 guiShowConsult; \
    u8 forcedPresent; \
    s8 forceMicShow; \
    s16 guiPickerPosX; \
    s16 guiPickerPosY; \
    u8 guiTestimonySpeaker; \
    SpeakerEntry* curSpeakerData; \
    SpeakerEntry* curTestimonySpeakerData;

typedef struct UIStruct_Common
{
    UI_STRUCT_FIELDS
} UIStruct_Common;



#endif