#ifndef IS64_PRINTF
#define IS64_PRINTF

#include <z64hdr/oot_u10/z64hdr.h>

extern void _isPrintfInit();
    asm("_isPrintfInit = 0x800D5EF0");

extern void is64Printf(const char* fmt, ...) ;
    asm("is64Printf = 0x800DCF50");
	
#endif