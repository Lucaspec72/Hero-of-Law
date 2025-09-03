#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"
#include "../../../../actor/_custom-1.0/is64Printf.h"

void Sram_WriteSave(SramContext* sramCtx) 
{
    gSaveContext.checksum = 0;
    u16* ptr = (u16*)&gSaveContext;

    u16 checksum = 0;
    for (int i = 0; i < CHECKSUM_SIZE; i++) 
        checksum += *ptr++;
    
    gSaveContext.checksum = checksum;
    
    *savingGfxCounter = SAVE_INDICATOR_LENGTH;

    // Write save, and backup save.
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT), &gSaveContext, SLOT_SIZE, OS_WRITE);
    SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(SAVE_SLOT_BACKUP), &gSaveContext, SLOT_SIZE, OS_WRITE);
}
