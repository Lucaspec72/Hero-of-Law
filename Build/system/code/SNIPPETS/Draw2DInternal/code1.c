#include "global.h"
#include "message_data_static.h"
#include "sfx.h"
#include "../../../../actor/_custom-1.0/common.h"

#define G_IM_SIZ_4b_BYTES		0
#define G_IM_SIZ_4b_TILE_BYTES	G_IM_SIZ_4b_BYTES
#define G_IM_SIZ_4b_LINE_BYTES	G_IM_SIZ_4b_BYTES

#define G_IM_SIZ_8b_BYTES		1
#define G_IM_SIZ_8b_TILE_BYTES	G_IM_SIZ_8b_BYTES
#define G_IM_SIZ_8b_LINE_BYTES	G_IM_SIZ_8b_BYTES

#define G_IM_SIZ_16b_BYTES		2
#define G_IM_SIZ_16b_TILE_BYTES	G_IM_SIZ_16b_BYTES
#define G_IM_SIZ_16b_LINE_BYTES	G_IM_SIZ_16b_BYTES

#define G_IM_SIZ_32b_BYTES		2
#define G_IM_SIZ_32b_TILE_BYTES	2
#define G_IM_SIZ_32b_LINE_BYTES	2

#define G_TEXTURE_IMAGE_FRAC    2
#define G_TEXTURE_SCALE_FRAC    16

#define RGBA16 0
#define RGBA16_Setup39 1
#define RGBA32 2
#define RGBA32_Setup39 3
#define I8 4
#define I8_Setup39 5
#define IA8 6
#define IA8_Setup39 7
#define IA16 8
#define IA16_Setup39 9
#define I4 10
#define I4_Setup39 11
#define CI4 12
#define CI4_Setup39 13
#define CI8 14
#define CI8_Setup39 15
#define IA4 16
#define IA4_Setup39 17

typedef struct {
    u8 fmt;
    u8 siz;
    u8 bytes;
    s8 shift;
} TextureFormat;

#define MIN(a, b)               ((a) < (b) ? (a) : (b))

#define gDPSetTileCustomH(pkt, fmt, siz, bytes, width, height, pal, cms, cmt, masks, maskt, shifts, shiftt)                    \
    do {                                                                                                               \
        gDPPipeSync(pkt);                                                                                              \
        gDPTileSync(pkt);                                                                                              \
        gDPSetTile(pkt, fmt, siz, (((width)*bytes) + 7) >> 3, 0, G_TX_LOADTILE, 0, cmt, maskt, shiftt, cms, \
                   masks, shifts);                                                                                     \
        gDPTileSync(pkt);                                                                                              \
        gDPSetTile(pkt, fmt, siz, (((width)*bytes) + 7) >> 3, 0, G_TX_RENDERTILE, pal, cmt, maskt, shiftt,  \
                   cms, masks, shifts);                                                                                \
        gDPSetTileSize(pkt, G_TX_RENDERTILE, 0, 0, ((width)-1) << G_TEXTURE_IMAGE_FRAC,                                \
                       ((height)-1) << G_TEXTURE_IMAGE_FRAC);                                                          \
    } while (0)


void Draw2DScaled(u8 RGBAType, int object, PlayState* playState, Gfx** gfxp, s16 centerX, s16 centerY, u8* source, u8* sourcePal, u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha);
void Draw2DInternal(u8 RGBAType, u8* texture, u8* palette, Gfx** gfxp, s16 centerX, s16 centerY, u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha);
static TextureFormat GetTextureFormat(u8 RGBAType);
static void SetUpTextureEnvironment(Gfx** gfxp, TextureFormat format, u8* palette, s16 alpha);

//800D7B00
void Draw2DInternal(u8 RGBAType, u8* texture, u8* palette, Gfx** gfxp, s16 centerX, s16 centerY,
                    u32 width, u32 height, u32 drawWidth, u32 drawHeight, s16 alpha)
{
    Gfx* gfx = *gfxp;

    // Setup display list based on RGBAType
    if (RGBAType % 2)
        Gfx_SetupDL_39Ptr(&gfx);
    else
        Gfx_SetupDL_56Ptr(&gfx);

    TextureFormat format = GetTextureFormat(RGBAType);
    u8 ciShift = (format.fmt == G_IM_FMT_CI) ? 1 : 0;
    
    if (SAVE_WIDESCREEN)
    {
        drawWidth *= WIDESCREEN_SCALEX;
        centerX *= WIDESCREEN_SCALEX;
        centerX += WIDESCREEN_OFFSX;
    }

    // Calculate drawing parameters
    float scaleX = (float)drawWidth / (float)width;
    float scaleY = (float)drawHeight / (float)height;
    s32 rectLeft = centerX - (drawWidth / 2);
    s32 rectTop = centerY - (drawHeight / 2);

    // Calculate chunk dimensions
    u32 maxChunkInHeight, strideBytes;

    if (format.siz == G_IM_SIZ_4b)
    {
        maxChunkInHeight = MIN(4096 / (width >> 1 - ciShift), height);
        strideBytes = width >> 1;
    }
    else
    {
        maxChunkInHeight = MIN(4096 / (width << format.shift + ciShift), height);
        strideBytes = width << format.shift;
    }

    SetUpTextureEnvironment(&gfx, format, palette, alpha);

    u32 posOutTopRel = 0;
    u32 prevInHeight = 0;
    float dsdx = (1.0f / scaleX) * (1 << 10);
    float dtdy = (1.0f / scaleY) * (1 << 10);

    while (posOutTopRel < drawHeight)
    {
        // Find where posOutTopRel would fall in the texture data
        float posInTop = posOutTopRel / scaleY;
        // Round to the nearest pixel upward
        u32 posInTopFloor = (u32)posInTop;
        // Find where the bottom of a texture chunk starting there would be
        u32 posInBottomCeil = MIN(posInTopFloor + maxChunkInHeight, height);
        // Zero-height texture chunk? Exit
        if (posInTopFloor == posInBottomCeil) break;
        // Find where the bottom of that chunk would land on screen (relative to rectTop),
        // rounded to the nearest pixel upward
        u32 posOutBottomRel = MIN((u32)(posInBottomCeil * scaleY), drawHeight);
        // Zero-height screen rect? Exit
        if (posOutTopRel == posOutBottomRel) break;

        // Calculate absolute screen rect
        s32 posOutTop = (rectTop + (s32)posOutTopRel) << 2;
        s32 posOutBottom = (rectTop + (s32)posOutBottomRel) << 2;
        s32 posOutLeft = rectLeft << 2;
        s32 posOutRight = (rectLeft + drawWidth) << 2;

        // Full height of texture chunk
        u32 inHeight = posInBottomCeil - posInTopFloor;

        // If needed, set tile info whatever stuff idk
        if (format.siz != G_IM_SIZ_4b && inHeight != prevInHeight)
        {
            gDPSetTileCustomH(gfx++, format.fmt, format.siz, format.bytes, width, inHeight, 0,
                             G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
                             G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
            prevInHeight = inHeight;
        }

        // Load chunk
        u8 *textureCursor = texture + posInTopFloor * strideBytes;
        if (format.siz == G_IM_SIZ_4b)
        {
            gDPLoadTextureBlock_4b(gfx++, textureCursor, format.fmt, width, inHeight, 0,
                                 G_TX_MIRROR, G_TX_MIRROR, 0, 0, G_TX_NOLOD, G_TX_NOLOD);
        } else
        {
            gDPSetTextureImage(gfx++, format.fmt, format.siz, width, textureCursor);
            gDPLoadTile(gfx++, G_TX_LOADTILE, 0, 0, (width - 1) << 2, (inHeight - 1) << 2);
        }

        // Draw chunk
        float posInTopFrac = posInTop - posInTopFloor;
        if (posOutLeft >= 0 && posOutRight + width <= SCREEN_WIDTH &&
            posOutTop >= 0 && posOutBottom + inHeight <= SCREEN_HEIGHT)
        {
            gSPTextureRectangle(gfx++, posOutLeft, posOutTop, posOutRight, posOutBottom,
                              G_TX_RENDERTILE, 0, posInTopFrac * (1 << 5), dsdx, dtdy);
        }
        else
        {
            gSPScisTextureRectangle(gfx++, posOutLeft, posOutTop, posOutRight, posOutBottom,
                                  G_TX_RENDERTILE, 0, posInTopFrac * (1 << 5), dsdx, dtdy);
        }

        // Next chunk should be drawn just below this one
        posOutTopRel = posOutBottomRel;
    }

    *gfxp = gfx;
}


TextureFormat GetTextureFormat(u8 RGBAType)
{
    TextureFormat format = {0};

    switch (RGBAType)
    {
        case RGBA16:
        case RGBA16_Setup39:
            format = (TextureFormat){G_IM_FMT_RGBA, G_IM_SIZ_16b, G_IM_SIZ_16b_BYTES, 1};
            break;
        case RGBA32:
        case RGBA32_Setup39:
            format = (TextureFormat){G_IM_FMT_RGBA, G_IM_SIZ_32b, G_IM_SIZ_32b_BYTES, 2};
            break;
        case I8:
        case I8_Setup39:
        case IA8:
        case IA8_Setup39:
        case CI8:
        case CI8_Setup39:
            format.fmt = (RGBAType > I8_Setup39 ?
                         RGBAType > IA8_Setup39 ? G_IM_FMT_CI : G_IM_FMT_IA :
                         G_IM_FMT_I);
            format.siz = G_IM_SIZ_8b;
            format.bytes = G_IM_SIZ_8b_BYTES;
            format.shift = 0;
            break;
        case IA16:
        case IA16_Setup39:
            format = (TextureFormat){G_IM_FMT_IA, G_IM_SIZ_16b, G_IM_SIZ_16b_BYTES, 1};
            break;
        case I4:
        case I4_Setup39:
        case CI4:
        case CI4_Setup39:
        case IA4:
        case IA4_Setup39:
            format.fmt = (RGBAType > I4_Setup39 ?
                         RGBAType > CI4_Setup39 ? G_IM_FMT_IA : G_IM_FMT_CI :
                         G_IM_FMT_I);
            format.siz = G_IM_SIZ_4b;
            format.bytes = G_IM_SIZ_4b_BYTES;
            format.shift = 0;
            break;
    }

    return format;
}

void SetUpTextureEnvironment(Gfx** gfxp, TextureFormat format, u8* palette, s16 alpha)
{
    Gfx* gfx = *gfxp;

    if (format.fmt == G_IM_FMT_CI)
    {
        gDPLoadTLUT_pal256(gfx++, palette);
        gDPSetTextureLUT(gfx++, G_TT_RGBA16);
    }

    if (format.fmt == G_IM_FMT_I)
    {
        if (palette != NULL)
        {
            Color_RGB8* col = (Color_RGB8*)palette;
            gDPSetPrimColor(gfx++, 0, 0, col->r, col->g, col->b, alpha);
        }
        else
            gDPSetPrimColor(gfx++, 0, 0, 0, 0, 0, alpha);
    }
    else
        gDPSetPrimColor(gfx++, 0, 0, 255, 255, 255, alpha);

    gDPSetColorDither(gfx++, G_CD_DISABLE);
    *gfxp = gfx;
}