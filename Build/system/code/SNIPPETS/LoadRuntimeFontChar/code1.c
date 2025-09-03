#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/holText.h"

int FindCharInList(u8* charList, int len, u8 character);

//0x800D6290 + 0x300
void* Font_LoadRuntimeFontChar(Font* font, u8 character)
{
    if (font == NULL)
        return NULL;
    
    void* charGraphics = NULL;
    RuntimeFont* f = (RuntimeFont*)&font->charTexBuf;
    
    // Check already stored characters, use the graphics if already loaded.
    int storedIdx = FindCharInList(f->storedChar, f->numStored, character);
    
    if (storedIdx != -1)
        charGraphics = &f->charTexBuf[storedIdx * FONT_CHAR_TEX_SIZE];
    else
    {
        // Not loaded?
        int unusedSlot = -1;
        
        // If there are empty slots, use that.
        if (f->numStored < RUNTIME_CHAR_COUNT)
        {
            unusedSlot = f->numStored;
            f->numStored++;           
        }            
        else
        {
            // If not, try finding a character that's loaded but hasn't been used on this frame yet.
            for (int j = 0; j < RUNTIME_CHAR_COUNT; j++)
            {
                if (FindCharInList(f->usedChar, RUNTIME_CHAR_COUNT, f->storedChar[j]) == -1)
                {
                    unusedSlot = j;
                    break;
                }
            }
        }
        
        // If unused slot has been found, load that character in.
        if (unusedSlot != -1)
        {
            f->storedChar[unusedSlot] = character;
            charGraphics = &f->charTexBuf[unusedSlot * FONT_CHAR_TEX_SIZE];

            RomFile* fnt = &objectTable[OBJECT_FONT];
            
            // The font file is a headered object
            // This is the minimum viable setup to get the address of the first file within it (which the default font must be)
            u32 buf[32];
            DmaMgr_SendRequest1(buf, fnt->vromStart, 32);
            u32 fontStart = buf[1];
            
            DmaMgr_SendRequest1(charGraphics, fnt->vromStart + fontStart + (FONT_CHAR_TEX_SIZE * (character - ' ')), FONT_CHAR_TEX_SIZE);  
        }           
        // Otherwise, give up.
        else
            return NULL;
    }
    
    // Store character in the used character list if it hasn't yet been used.
    // This list has to be re-set every frame by the state's main loop
    if (f->numUsed < RUNTIME_CHAR_COUNT)
    {
        if (FindCharInList(f->usedChar, RUNTIME_CHAR_COUNT, character) == -1)
        {
            f->usedChar[f->numUsed] = character;
            f->numUsed++;                                     
        }            
    }

    return charGraphics;
}

int FindCharInList(u8* charList, int len, u8 character)
{
    for (int i = 0; i < len; i++)
    {
        if (charList[i] == character)
            return i;
    }
    
    return -1; 
}