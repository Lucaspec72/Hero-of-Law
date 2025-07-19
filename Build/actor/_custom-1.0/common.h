#ifndef COMMON_H
#define COMMON_H

#define AudioHeapSize 0x3C000

u32* emuIdentifier = (u32*)0x80198948;
u32* hzChoice = (u32*)0x80198950;
u32* tpakBeingUsed = (u32*)0x80198954;

#define GCN_CHECK_MAGIC 0x47434E45
#define WII_CHECK_MAGIC 0x5245564F
#define WIIU_CHECK_MAGIC 0x43414645

OSPiHandle** sISVHandle = (OSPiHandle**)0x8019894C;

#define WIDESCREEN_OFFSX 40
#define WIDESCREEN_TXBOX_OFFSX (WIDESCREEN_OFFSX - 6)
#define WIDESCREEN_SCALEX 0.75117370f

#define SAVE_HEALTH gSaveContext.healthCapacity
#define SAVE_PROGRESS gSaveContext.bgsDayCount
#define SAVE_EVIDENCE gSaveContext.swordHealth
#define SAVE_DEBUG_DEPRECATED gSaveContext.deaths
#define SAVE_DEBUGMODE gSaveContext.isDoubleDefenseAcquired
#define SAVE_CREDITSDEBUGCOUNTER gSaveContext.highScores[0]
#define SAVE_LASTDIEDONSCENE gSaveContext.scarecrowLongSong[0]
#define SAVE_EXTRASCENES gSaveContext.scarecrowLongSong[1]
#define SAVE_HASBEATENGAME (gSaveContext.scarecrowLongSongSet == 125)

#define SAVE_LASLOT gSaveContext.scarecrowLongSong[2] 
#define SAVE_LAKILLEDBYSHOPKEEPER gSaveContext.scarecrowLongSong[3] 
#define SAVE_LANAME gSaveContext.scarecrowLongSong[4] // 5 bytes
#define SAVE_NUMTHEFTS gSaveContext.scarecrowLongSong[9]
#define SAVE_LAUNCHSCENE gSaveContext.scarecrowLongSong[10]
#define SAVE_LAUNCHLEVEL gSaveContext.scarecrowLongSong[11]
#define SAVE_ANTIPIRACYGAG gSaveContext.scarecrowLongSong[12]
#define SAVE_CASINORUPEES (u16*)&gSaveContext.scarecrowLongSong[13] // 2 bytes
#define SAVE_INGOHINT gSaveContext.scarecrowLongSong[15]
#define SAVE_SHOWNTALONGAMECONTROLS gSaveContext.scarecrowLongSong[16]
#define SAVE_LATUNIC gSaveContext.scarecrowLongSong[17]
#define SAVE_SHOWNUNLOCK gSaveContext.scarecrowLongSong[18]
#define SAVE_LANOLAWYERS gSaveContext.scarecrowLongSong[19]
#define SAVE_SCREENXPOS gSaveContext.scarecrowLongSong[20]
#define SAVE_SCREENYPOS gSaveContext.scarecrowLongSong[21]
#define SAVE_SCREENSIZEX gSaveContext.scarecrowLongSong[22]
#define SAVE_SCREENSIZEY gSaveContext.scarecrowLongSong[23]
#define SAVE_AUDIOSETTING gSaveContext.scarecrowLongSong[24]        // Saved twice because it's less annoying than writing to the header
#define SAVE_WIDESCREEN gSaveContext.scarecrowLongSong[25]
#define SAVE_ANTIALIASOFF gSaveContext.scarecrowLongSong[26]
#define SAVE_LANGUAGE gSaveContext.scarecrowLongSong[27]

#define SCREENSIZE_DEFAULT 225
#define SCREENPOSX_DEFAULT 0
#define SCREENPOSY_DEFAULT 0
#define WIDESCREEN_DEFAULT 0
#define ANTIALIASOFF_DEFAULT 0

#ifndef MAX
    #define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef SAVE_STUFF

    #define SAVE_OK 0
    #define SAVE_CORRUPTED 0x5
    #define SAVE_NOT_HOL 0x12

    #define SRAM_SIZE 0x8000
    #define SRAM_HEADER_SIZE 0x10
    #define SLOT_SIZE (sizeof(SaveContext) + 0x28)
    #define SLOT_OFFSET(index) (SRAM_HEADER_SIZE + 0x10 + (index * SLOT_SIZE))
    #define SAVESTRUCT_SIZE 0x1354
    #define CHECKSUM_SIZE (SAVESTRUCT_SIZE / 2)

    char sSaveDefaultMagic[] = {0x98, 0x09, 0x10, 0x21, 'Z', 'E', 'L', 'D', 'A'};
    char sHeroOfLawMagic[] = {'H', 'E', 'R', 'O', '+', 'L', 'A', 'W'};

    int LoadSaveAndVerify(int slot)
    {
        gSaveContext.fileNum = 0;
        
        u16 i, j, oldChecksum, newChecksum, offset;
        u16* ptr;
          
        SsSram_ReadWrite(OS_K1_TO_PHYSICAL(0xA8000000) + SLOT_OFFSET(slot), &gSaveContext, SAVESTRUCT_SIZE, OS_READ);
        
        if (bcmp(sHeroOfLawMagic, &gSaveContext.playerName, ARRAY_COUNTU(sHeroOfLawMagic)))
            return SAVE_NOT_HOL;     
        
        oldChecksum = gSaveContext.checksum;
        gSaveContext.checksum = 0;  
        ptr = (u16*)&gSaveContext;

        for (i = newChecksum = j = 0; i < CHECKSUM_SIZE; i++, offset += 2) 
        {
            j += 2;
            
            if (j == 0x20) 
                j = 0;
            
            newChecksum += *ptr++;
        } 
        
        return oldChecksum == newChecksum ? SAVE_OK : SAVE_CORRUPTED;
    }

#endif


extern void Screen_Adjust(GameState* state, View* view);
    asm("Screen_Adjust = 0x8006CA64 + 0x8");
    
extern void SsSram_ReadWrite(u32 addr, void* dramAddr, size_t size, s32 direction);
    asm("SsSram_ReadWrite = 0x80091474");
    
    
    

#endif