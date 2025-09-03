#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/holText.h"

//8005BD78
void Font_LoadFont(Font* font)
{
    int languageIdx = LANG_INDEX;
    
    // Load the default font widths
    LoadFromHeaderObjectToDest(OBJECT_FONT, 1, (u32*)&font->fontBuf, fontWidthsDefault);    
    // Load the current language's font widths
    LoadFromHeaderObjectToDest(OBJECT_FONT, 1 + (languageIdx * 2), (u32*)&font->fontBuf, fontWidthsLanguage);
    // Load the current language's font
    void* ret = LoadFromHeaderObjectToDest(OBJECT_FONT, languageIdx * 2, (u32*)&font->fontBuf, &font->fontBuf);
    
    // If loading the current language's font failed, load in the default lengths and font.
    if (!ret)
    {
        LoadFromHeaderObjectToDest(OBJECT_FONT, 1, (u32*)&font->fontBuf, fontWidthsLanguage);
        LoadFromHeaderObjectToDest(OBJECT_FONT, 0, (u32*)&font->fontBuf, &font->fontBuf);
    }
}