#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/draw2D.h"
#include "../../../../actor/_custom-1.0/holText.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"

extern Input* D_8012D1F8;
void Play_Update(PlayState* this);
void Play_Draw(PlayState* this);
void DebugDisplay_Init(void);

extern ViMode sViMode;
extern PreNmiBuff* gAppNmiBufferPtr;

#define SAVE_SCREENXPOS gSaveContext.scarecrowLongSong[20]
#define SAVE_SCREENYPOS gSaveContext.scarecrowLongSong[21]
#define SAVE_SCREENSIZEX gSaveContext.scarecrowLongSong[22]
#define SAVE_SCREENSIZEY gSaveContext.scarecrowLongSong[23]

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define SCREENSIZE_DEFAULT 225
#define SCREENPOSX_DEFAULT 0
#define SCREENPOSY_DEFAULT 0

int preNMI = 0;

void Play_Main2(PlayState* play)
{
    D_8012D1F8 = &play->state.input[0];

    RuntimeFont* f = (RuntimeFont*)&play->msgCtx.font.charTexBuf;
    
    if (f->numUsed != 0 || f->usedChar[0] != 0xFF)
    {
        Lib_MemSet(&f->usedChar[0], RUNTIME_CHAR_COUNT, 0xFF);
        f->numUsed = 0;
    }

    DebugDisplay_Init();
    Play_Update(play);
    Play_Draw(play);
    Screen_Adjust(&play->state, &play->view);
    return;
}

