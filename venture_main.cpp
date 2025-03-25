/* ========================================================================
   $File: venture_main.cpp $
   $Date: Wed, 12 Mar 25: 03:29PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
// BASE HEADERS
#include <venture_base_types.h>
#include <venture_base_debug.h>
#include <venture_base_math.h>
#include <venture_base_memory.h>
#include <venture_base_string.h>
#include <venture_base_hash_table.h>
#include <venture_base_darray.h>

// PLATFORM DEFINES
#include <venture_platform.h>

// UNITY BUILD CORE
#include <venture_opengl.cpp>
#include <venture_dynamic_font.cpp>
#include <venture_render_group.cpp>

internal void
VentureMain(platform_data_handles *PlatformHandles, application_memory *Memory)
{
    render_state RenderState = {};
    
    InitFontLoading();
    VentureInitOpenGLRenderer(&RenderState);

    memory_arena Arena = ArenaCreate(&Memory->PrimaryPool, MB(1000));
    render_target *FirstTarget = RenderTargetCreate(&Arena, &RenderState);

    venture_dynamic_render_font Font       = LoadFontData(&Arena, str_lit("../fonts/LiberationMono-Regular.ttf"));
    venture_dynamic_font_varient *FontSize = GetFontAtSize(&Font, 32);

    string_u8 Data         = str_lit("We tried to convert this");
    uint32 Value           = UTF8ConvertToUTF32(&Data.Data[0]);
    glyph_data *Result     = GetUTF8Glyph(FontSize, &Data.Data[0]);
    glyph_data *HashResult = (glyph_data *)HashGetValue(&Result->OwnerPage->GlyphLookup, Result->HashKey);

    HashResult->OwnerPage->AtlasTexture = VentureCreateTextureFromBitmap(&HashResult->OwnerPage->AtlasBitmap);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, HashResult->OwnerPage->AtlasTexture.TexID);
    
    while(AppIsRunning)
    {
        ClientSize = PlatformGetClientSize(PlatformHandles);
        PlatformPollEvents(PlatformHandles);
        
        VentureRenderOneFrame(&RenderState);
    }
}
