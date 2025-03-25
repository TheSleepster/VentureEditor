#if !defined(VENTURE_DYNAMIC_FONT_H)
/* ========================================================================
   $File: venture_dynamic_font.h $
   $Date: Mon, 24 Mar 25: 02:50PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_DYNAMIC_FONT_H
#include <venture_base_types.h>
#include <venture_base_memory.h>
#include <venture_platform.h>

#include <venture_opengl.h>
#include <venture_render_group.h>

struct venture_dynamic_render_font;
struct venture_dynamic_font_page;

constexpr uint32 FontAtlasWidth  = 2048;
constexpr uint32 FontAtlasHeight = 2048;

// UNICODE
uint8 TrailingBytesUTF8[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

uint8 UTF8InitialBytemask[] = {0x7f, 0x1f, 0x0f, 0x07, 0x03, 0x01};
uint8 UTF8FirstByteMark[]   = {0x00, 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};

uint32 UTF8Offsets[] = {0x00000000, 0x00003080, 0x000e2080, 
					    0x03c82080, 0xfa082080, 0x82082080};

constexpr uint32 UTF16_MAX_CHARACTER         = 0x0010FFFF;
constexpr uint32 UTF32_MAX_CHARACTER         = 0x7FFFFFFF;
constexpr uint32 UTF32_REPLACEMENT_CHARACTER = 0x0000FFFD;
// UNICODE

struct glyph_data
{
    ivec2  AtlasOffset;
    ivec2  GlyphSize;

    void  *HashKey;
    uint32 OffsetX;
    uint32 OffsetY;

    uint32 Advance;
    uint32 Ascent;

    venture_dynamic_font_page *OwnerPage;
};


struct venture_dynamic_font_varient;
struct venture_dynamic_font_page
{
    bool8                      PageFull;
    bool8                      BitmapDirty;
    
    hash_table                 GlyphLookup;

    venture_texture            AtlasTexture;
    venture_bitmap             AtlasBitmap;

    int32                      BitmapCursorX;
    int32                      BitmapCursorY;

    venture_dynamic_font_varient *ParentVarient;
    venture_dynamic_font_page    *NextPage;
};

struct venture_dynamic_font_varient
{
    uint32                      PixelSize;

    int32                       LineSpacing;
    int32                       MaxAscender;
    int32                       MaxDescender;
    int32                       yCenterOffset;
    int32                       TypicalAscender;
    int32                       TypicalDescender;
    int32                       EmWidth;
    int32                       DefaultUnknownCharacter;
    int32                       DefaultUTF32UnknownCharacter = 0xfffd;

    venture_dynamic_render_font *ParentFont;
    venture_dynamic_font_page   *FirstPage;
};

struct venture_dynamic_render_font
{
    FT_Face                       FontFace;
    string_u8                     Filepath;
    uint8                        *FontData;
    uint32                        FontDataLength;

    memory_arena                 *FontArena;
    venture_dynamic_font_varient *PixelSizes;
};

#endif
