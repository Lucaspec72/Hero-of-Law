#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"

//8005BD78
void Font_LoadFont(Font* font)
{
    RomFile* fnt = &objectTable[OBJECT_FONT];
    DmaMgr_SendRequest1(&font->fontBuf, fnt->vromStart, fnt->vromEnd - fnt->vromStart);
}