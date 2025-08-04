#include "global.h"
#include "message_data_static.h"
#include "sfx.h"

#include "../../../../actor/_custom-1.0/common.h"

void Sram_WriteSave(SramContext* sramCtx) 
{
    gSaveContext.checksum = 0;
    u16* ptr = (u16*)&gSaveContext;

    for (int i = 0; i < CHECKSUM_SIZE; i++) 
        gSaveContext.checksum += *ptr++;

    // Write save, and backup save.
    for (int slot = 0; slot < 2; slot++) 
    {
        SsSram_ReadWrite(SRAM_BASE_ADDR + SLOT_OFFSET(slot), &gSaveContext, SLOT_SIZE, OS_WRITE);
    }
}
