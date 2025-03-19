/* ========================================================================
   $File: venture_dynamic_font.cpp $
   $Date: Thu, 13 Mar 25: 02:56PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include <venture_base_types.h>
#include <venture_base_memory.h>
#include <venture_platform.h>

// NOTE(Sleepster): This is fine...
global FT_Library FT_Lib;

struct venture_dynamic_render_font;
struct venture_dynamic_font_page;

constexpr uint32 FontAtlasWidth  = 2048;
constexpr uint32 FontAtlasHeight = 2048;

struct glyph_data
{
    ivec2  AtlasOffset;
    ivec2  GlyphSize;

    uint32 HashKey;
    uint32 OffsetX;
    uint32 OffsetY;

    uint32 Advance;
    uint32 Ascent;

    venture_dynamic_font_page *OwnerPage;
};

struct venture_dynamic_font_page
{
    hash_table                 GlyphLookup;

    texture                    AtlasTexture;
    bitmap                     AtlasBitmap;

    int32                      BitmapCursorX;
    int32                      BitmapCursorY;

    venture_dynamic_font_page *NextPage;
};

struct venture_dynamic_font_varient
{
    uint32                       PixelSize;
    uint32                       CharacterHeight;

    uint32                       LineSpacing;
    uint32                       MaxAscender;
    uint32                       MaxDecender;
    uint32                       yCenterOffset;
    uint32                       TypicalAscender;
    uint32                       TypicalDescender;
    uint32                       EmWidth;
    uint32                       DefaultUnknownCharacter;
    uint32                       DefaultUTF32UnknownCharacter;

    venture_dynamic_render_font *ParentFont;
    venture_dynamic_font_page    FirstPage;
};

struct venture_dynamic_render_font
{
    FT_Face                       FontFace;
    string_u8                     Filepath;
    uint8                        *FontData;

    memory_arena                 *FontArena;
    venture_dynamic_font_varient *PixelSizes;
};

internal void
InitFontLoading()
{
    FT_Error Error = FT_Init_FreeType(&FT_Lib);
    Assert(!Error, "Failed to init freetype");
}

internal venture_dynamic_font_varient
GetFontAtSize(venture_dynamic_render_font *FontData, uint32 FontSize)
{
    venture_dynamic_font_varient Result;
    return(Result);
}

internal venture_dynamic_render_font
LoadFontData(memory_arena *Arena, string_u8 Filepath)
{
    venture_dynamic_render_font Result;

    Result.Filepath   = Filepath;
    Result.FontArena  = Arena;
    Result.PixelSizes = (venture_dynamic_font_varient *)DArrayCreate(venture_dynamic_font_varient, 10);

    uint32 FileSize;
    Result.FontData = PlatformReadEntireFile(Arena, Filepath, &FileSize);
    FT_Error Error  = FT_New_Memory_Face(FT_Lib, Result.FontData, FileSize, 0, &Result.FontFace);
    Assert(!Error, "FT_New_Memory_Face() Error...");

// TODO(Sleepster): UNICODE STUFF!!!! YAYYYYYYYYY (I will blow my fucking head off...) 
}
