/* ========================================================================
   $File: venture_dynamic_font.cpp $
   $Date: Thu, 13 Mar 25: 02:56PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include <venture_dynamic_font.h>

// NOTE(Sleepster): This is fine...
global FT_Library FT_Lib;

internal void
InitFontLoading()
{
    FT_Error Error = FT_Init_FreeType(&FT_Lib);
    Assert(!Error, "Failed to init freetype");
}

internal inline int32
FT_ROUND(int32 X)
{
    if (X >= 0) return (X + 0x1f) >> 6;
    return -(((-X) + 0x1f) >> 6);
}

internal ivec2
FindNextGlyphLine(venture_dynamic_font_page *Page, uint32 GlyphWidth, uint32 GlyphHeight)
{
    ivec2 Result = {};
    uint32 DesiredX = Page->BitmapCursorX + GlyphWidth;
    if(DesiredX > Page->AtlasBitmap.Width)
    {
        Page->BitmapCursorX  = 0;
        Page->BitmapCursorY += Page->ParentVarient->MaxAscender;
        
        Result = {0, Page->BitmapCursorY};
    }
    else
    {
        Result = {Page->BitmapCursorX, Page->BitmapCursorY};
    }
    
    return(Result);
}
    
internal void
CopyGlyphDataToPageBitmap(venture_dynamic_font_page *Page, glyph_data *Glyph)
{
    if(!Page->AtlasBitmap.Data)
    {
        // NOTE(Sleepster): font bitmaps are RGBA8
        Page->AtlasBitmap = VentureBitmapCreate(Page->ParentVarient->ParentFont->FontArena,
                                                FontAtlasWidth,
                                                FontAtlasHeight,
                                                4);
    }
    FT_Face FontFace       = Page->ParentVarient->ParentFont->FontFace;

    Glyph->OffsetX  = (int16)FontFace->glyph->bitmap_left;
    Glyph->OffsetY  = (int16)FontFace->glyph->bitmap_top;
    Glyph->Advance  = (int16)FontFace->glyph->advance.x >> 6;
    Glyph->Ascent   = FontFace->glyph->metrics.horiBearingY >> 6; 

    Glyph->AtlasOffset = FindNextGlyphLine(Glyph->OwnerPage,
                                           FontFace->glyph->bitmap.width,
                                           FontFace->glyph->bitmap.rows);
    int32 Width    = FontFace->glyph->bitmap.width;
    int32 RowCount = FontFace->glyph->bitmap.rows;
    for(int32 Row = 0;
        Row < RowCount;
        ++Row)
    {
        for(int32 Column = 0;
            Column < Width;
            ++Column)
        {
            uint8 Source = FontFace->glyph->bitmap.buffer[(RowCount- 1 - Row) * FontFace->glyph->bitmap.pitch + Column];
            uint8 *Dest  = (uint8 *)Page->AtlasBitmap.Data + ((Glyph->AtlasOffset.Y + Row) * Page->AtlasBitmap.Width + (Glyph->AtlasOffset.X + Column)) * 4;

            Dest[0] = Source;
            Dest[1] = Source;
            Dest[2] = Source;
            Dest[3] = Source;
        }
    }
    
    Page->BitmapCursorX += FontFace->glyph->bitmap.width;
    if(!Page->AtlasTexture.Data)
    {
        venture_texture *Texture = &Page->AtlasTexture;
        Texture->Data   = Page->AtlasBitmap.Data;
        Texture->Width  = Page->AtlasBitmap.Width;
        Texture->Height = Page->AtlasBitmap.Height;
    }
}

internal uint32
UTF8ConvertToUTF32(uint8 *Character)
{
    uint32 Result = 0;
    uint8 ContinuationBytes = TrailingBytesUTF8[Character[0]]; 
    if(ContinuationBytes + 1 > 1000)
    {
        return(0);
    }

    uint32 UTF32Character = Character[0] & UTF8InitialBytemask[ContinuationBytes];
    for(int32 Byte = 1;
        Byte < ContinuationBytes;
        Byte++)
    {
        UTF32Character = UTF32Character << 6;
        UTF32Character |= Character[Byte] & 0x3F;
    }

    if(UTF32Character > UTF32_MAX_CHARACTER)
    {
        UTF32Character = UTF32_REPLACEMENT_CHARACTER;
    }

    return(UTF32Character);
}

internal glyph_data*
GetUTF8Glyph(venture_dynamic_font_varient *Varient, uint8 *Character)
{
    glyph_data *Result = {};
    
    venture_dynamic_font_page *Page = 0;
    uint32 UTF32Char = UTF8ConvertToUTF32(Character);
    if(UTF32Char)
    {
        string_u8 Temp = str_lit((char *)&UTF32Char);
        string_u8 *KeyString = StringCopy(Varient->ParentFont->FontArena, Temp);

        for(venture_dynamic_font_page *FirstPage = Varient->FirstPage;
            FirstPage;
            FirstPage = FirstPage->NextPage)
        {
            Result = (glyph_data *)HashGetValue(&FirstPage->GlyphLookup, (void *)KeyString);
            if(Result)
            {
                Page = FirstPage;
                break;
            }
        }

        if(!Result && !Page)
        {
            for(venture_dynamic_font_page *NewPage = Varient->FirstPage;
                NewPage;
                NewPage = NewPage->NextPage)
            {
                if(!NewPage->PageFull)
                {
                    Page = NewPage;
                }
            }

            if(!Page)
            {
                venture_dynamic_font_page *NextPage = 0; 
                for(venture_dynamic_font_page *NewPage = Varient->FirstPage;
                    NewPage;
                    NewPage = NewPage->NextPage)
                {
                    if(NewPage->NextPage == null)
                    {
                        NewPage->NextPage = ArenaPushStruct(Varient->ParentFont->FontArena, venture_dynamic_font_page);
                        NewPage->NextPage->NextPage = null;
                        NewPage->ParentVarient = Varient;
                    }
                }
            }
            
            FT_Error Error  = FT_Set_Pixel_Sizes(Varient->ParentFont->FontFace, 0, Varient->PixelSize);
            Assert(!Error, "FT_Set_Pixel_Sizes has failed...");

            uint32 GlyphIndex = 0;
            if(UTF32Char)
            {
                GlyphIndex = FT_Get_Char_Index(Varient->ParentFont->FontFace, UTF32Char);
                if(!GlyphIndex)
                {
                    Log(LOG_WARNING, "UTF32 character '%d' cannot be found", UTF32Char);
                    GlyphIndex = Varient->DefaultUnknownCharacter;
                }

                if(GlyphIndex)
                {
                    Error = FT_Load_Glyph(Varient->ParentFont->FontFace, GlyphIndex, FT_LOAD_RENDER);
                    Assert(!Error, "FT_Load_Glyph has failed on GlyphIndex '%d'...", GlyphIndex);
                }
            }
            else
            {
                Assert(GlyphIndex >= 0, "GlyphIndex > 0");
                Error = FT_Load_Glyph(Varient->ParentFont->FontFace, GlyphIndex, FT_LOAD_RENDER);
                Assert(!Error, "FT_Load_Glyph has failed on GlyphIndex '%'...", GlyphIndex);
            }

            glyph_data *GlyphData = ArenaPushStruct(Varient->ParentFont->FontArena, glyph_data);
            GlyphData->HashKey     = KeyString;
            GlyphData->OwnerPage   = Page;

            CopyGlyphDataToPageBitmap(Page, GlyphData);
            Page->BitmapDirty = true;

            HashInsertPair(&Page->GlyphLookup, (void *)KeyString, (void *)GlyphData);

            Result = GlyphData;
        }
    }
    else
    {
        Log(LOG_ERROR, "Character '%s' does not return a valid utf32 character... Index returned is '%d'", Character, UTF32Char);
    }

    return(Result);
}

internal inline bool8
SetFontUnknownCharacter(venture_dynamic_font_varient *FontVar, uint32 UTF32Index)
{
    uint32 GlyphIndex = FT_Get_Char_Index(FontVar->ParentFont->FontFace, UTF32Index);
    if(GlyphIndex)
    {
        FontVar->DefaultUnknownCharacter = GlyphIndex;
    }
    else
    {
        return(false);
    }
    return(true);
}

// TODO(Sleepster): UNICODE STUFF!!!! YAYYYYYYYYY (I will blow my fucking head off...) 
internal venture_dynamic_render_font
LoadFontData(memory_arena *Arena, string_u8 Filepath)
{
    venture_dynamic_render_font Result;
    uint32 FileSize;

    Result.Filepath       = Filepath;
    Result.FontArena      = Arena;
    Result.PixelSizes     = (venture_dynamic_font_varient *)DArrayCreate(venture_dynamic_font_varient *, 10);
    Result.FontData       = PlatformReadEntireFile(Arena, Filepath, &FileSize);
    Result.FontDataLength = FileSize;

    return(Result);
}

internal venture_dynamic_font_varient*
CreateFontAtSize(venture_dynamic_render_font *Font, uint32 FontSize)
{
    venture_dynamic_font_varient *Result = 0;
    if(Font->FontData)
    {
        FT_Error Error = FT_New_Memory_Face(FT_Lib, Font->FontData, Font->FontDataLength, 0, &Font->FontFace);
        if(!Error)
        {
            Result            = ArenaPushStruct(Font->FontArena, venture_dynamic_font_varient);
            Result->FirstPage = ArenaPushStruct(Font->FontArena, venture_dynamic_font_page);
            Result->FirstPage->NextPage  = null;
            Result->FirstPage->ParentVarient = Result;

            Error  = FT_Set_Pixel_Sizes(Font->FontFace, 0, FontSize);
            Assert(!Error, "Failure to set the freetype pixel sizes");

            real32 FontScaleToPixels = Font->FontFace->size->metrics.y_scale / (64.0f * 65536.0f);
            Result->PixelSize = FontSize;

            Result->LineSpacing     = (uint32)floorf(FontScaleToPixels * Font->FontFace->height + 0.5f);
            Result->MaxAscender     = (uint32)floorf(FontScaleToPixels * Font->FontFace->bbox.yMax + 0.5f);
            Result->MaxDescender    = (uint32)floorf(FontScaleToPixels * Font->FontFace->bbox.yMin + 0.5f);
            Result->ParentFont      = Font;

            uint32 GlyphIndex = FT_Get_Char_Index(Font->FontFace, 'm');
            if(GlyphIndex)
            {
                FT_Load_Glyph(Font->FontFace, GlyphIndex, FT_LOAD_DEFAULT);
                Result->yCenterOffset = (uint32)(0.5f * FT_ROUND(Font->FontFace->glyph->metrics.horiBearingY) + 0.5f);
            }

            GlyphIndex  = FT_Get_Char_Index(Font->FontFace, 'M');
            if(GlyphIndex)
            {
                FT_Load_Glyph(Font->FontFace, GlyphIndex, FT_LOAD_DEFAULT);
                Result->EmWidth = FT_ROUND(Font->FontFace->glyph->metrics.width);
            }

            GlyphIndex  = FT_Get_Char_Index(Font->FontFace, 'T');
            if(GlyphIndex)
            {
                FT_Load_Glyph(Font->FontFace, GlyphIndex, FT_LOAD_DEFAULT);
                Result->TypicalAscender = FT_ROUND(Font->FontFace->glyph->metrics.horiBearingY);
            }

            GlyphIndex  = FT_Get_Char_Index(Font->FontFace, 'g');
            if(GlyphIndex)
            {
                FT_Load_Glyph(Font->FontFace, GlyphIndex, FT_LOAD_DEFAULT);
                Result->TypicalDescender = FT_ROUND(Font->FontFace->glyph->metrics.horiBearingY - Font->FontFace->glyph->metrics.height);
            }

            Error = FT_Select_Charmap(Font->FontFace, FT_ENCODING_UNICODE);
            if(Error)
            {
                Log(LOG_ERROR, "Font does not support UNICODE...\n");
            }

            // UNKNOWN CHARACTERS
            {
                bool8 Success = SetFontUnknownCharacter(Result, 0xfffd);  // replacement_character
                if(!Success)  Success = SetFontUnknownCharacter(Result, 0x2022);  // bullet
                if(!Success)  Success = SetFontUnknownCharacter(Result, '?');
                if(!Success)
                {
                    Log(LOG_WARNING, "unable to set unknown character for the font.\n");
                }
            }

            DArrayAppendValue(Font->PixelSizes, Result);
        }
    }

    return(Result);
}

internal venture_dynamic_font_varient*
GetFontAtSize(venture_dynamic_render_font *Font, uint32 FontSize)
{
    venture_dynamic_font_varient *Result = 0;
    for(uint32 FontIndex = 0;
        FontIndex < DArrayGetUsedElementCount(Font->PixelSizes);
        ++FontIndex)
    {
        venture_dynamic_font_varient *Found = &Font->PixelSizes[FontIndex];
        if(Found->PixelSize == FontSize)
        {
            Result = Found;
        }
    }
    if(!Result)
    {
        Result = CreateFontAtSize(Font, FontSize);
    }
    
    return(Result);
}

