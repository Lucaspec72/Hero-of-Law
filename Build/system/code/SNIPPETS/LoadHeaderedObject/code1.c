#include "global.h"
#include "../../../../actor/_custom-1.0/common.h"


void* LoadFromHeaderObjectToDest(int object, int entryId, u32* header, void* dest) 
{
    RomFile* file = &objectTable[object];
    
    // Check if object is valid
    if (file->vromEnd - file->vromStart < 32)
        return NULL;
    
    // Calculate required header size
    int headerSize = ((MAX(32, 12 + entryId * 8) + 31) & ~31);
    bool headerProvided = (header != NULL);
    
    // Allocate header if not provided
    if (!headerProvided) 
    {
        header = ZeldaArena_Malloc(headerSize);
        
        // Out of memory...
        if (!header) 
            return NULL;
    }
    
    // Load header data
    DmaMgr_SendRequest1(header, file->vromStart, headerSize);
    
    // Validate entry exists and is not empty
    if (header[0] <= entryId || header[1 + entryId * 2 + 1] == 0) 
    {
        if (!headerProvided)
            ZeldaArena_Free(header);
        
        return NULL;
    }
    
    // Extract entry information
    int offset = header[1 + entryId * 2];
    int size = (header[1 + entryId * 2 + 1] + 31) & ~31;
    
    // Clean up header if we allocated it
    if (!headerProvided)
        ZeldaArena_Free(header);
    
    // Handle destination buffer
    bool destProvided = (dest != NULL);
    void* buffer = dest;
    
    if (!destProvided) 
    {
        buffer = ZeldaArena_Malloc(size);
        
        // Out of memory...
        if (!buffer)
            return NULL;
    }
    
    // Load data from ROM
    DmaMgr_SendRequest1(buffer, file->vromStart + offset, size);
    
    // If caller provided destination, they handle decompression
    if (destProvided)
        return buffer;
    
    // Handle decompression for allocated buffers
    if (*(u32*)buffer == 0x59617A30) // Yaz0 magic
    {
        Yaz0Header* compBuf = (Yaz0Header*)buffer;
        int decSize = (compBuf->decSize + 31) & ~31;
        
        u8* decompressed = ZeldaArena_Malloc(decSize);
        
        if (!decompressed) 
        {
            // Out of memory...
            ZeldaArena_Free(buffer);
            return NULL;
        }
        
        Yaz0_DecompressImpl(compBuf, decompressed);
        ZeldaArena_Free(buffer);
        return decompressed;
    }
    
    return buffer;
}