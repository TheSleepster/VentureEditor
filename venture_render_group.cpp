/* ========================================================================
   $File: venture_render_group.cpp $
   $Date: Mon, 24 Mar 25: 01:21PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include <venture_base_types.h>
#include <venture_base_debug.h>
#include <venture_base_math.h>
#include <venture_base_memory.h>
#include <venture_base_string.h>
#include <venture_base_hash_table.h>
#include <venture_base_darray.h>

#include <venture_opengl.h>

#include <venture_dynamic_font.h>

internal void
PushVertices(render_state *RenderState, render_vertex *Vertices)
{
    Assert(Vertices, "Vertices is invalid...\n");
    Assert(Vertices + 6, "Vertices is invalid...\n");
    Assert(RenderState->VertexCount + 6 < MAX_VERTICES, "We have reached the end of our vertex buffer. MAX_VERTICES is: '%d'... Current Vertex Count: '%d'...", MAX_VERTICES, RenderState->VertexCount);
    render_vertex *VertexPTR = RenderState->Vertices + RenderState->VertexCount;

    VertexPTR[0] = Vertices[0];
    VertexPTR[1] = Vertices[1];
    VertexPTR[2] = Vertices[2];
    VertexPTR[3] = Vertices[3];
    VertexPTR[4] = Vertices[4];
    VertexPTR[5] = Vertices[5];
    
    RenderState->VertexCount += 6;
}

// NOTE(Sleepster): This must be passed a unicode codepoint...
internal void 
PushGlyph(render_state *RenderState, char *Character, vec2 Position, vec4 Color)
{
    if(RenderState->ActiveFont)
    {
        render_vertex Vertices[6] = {};
        glyph_data *Glyph = GetUTF8Glyph(RenderState->ActivePixelSize, (uint8 *)Character);

        /*
          2 == 4
          0 == 3
        */

        Vertices[0].Position =  Position;                                                            // Bottom-left
        Vertices[1].Position = {Position.X + Glyph->GlyphSize.X, Position.Y};                        // Bottom-right
        Vertices[2].Position = {Position.X + Glyph->GlyphSize.X, Position.Y + Glyph->GlyphSize.Y};   // Top-right
        Vertices[5].Position = {Position.X,                      Position.Y + Glyph->GlyphSize.Y};   // Top-left

        Vertices[0].TexCoords = {Glyph->AtlasOffset.X, Glyph->AtlasOffset.Y + Glyph->GlyphSize.Y};
        Vertices[1].TexCoords = {Glyph->AtlasOffset.X + Glyph->GlyphSize.X, Glyph->AtlasOffset.Y + Glyph->GlyphSize.Y};
        Vertices[2].TexCoords = {Glyph->AtlasOffset.X + Glyph->GlyphSize.X, Glyph->AtlasOffset.Y};
        Vertices[5].TexCoords = {Glyph->AtlasOffset.X, Glyph->AtlasOffset.Y};

        Vertices[4] = Vertices[2];
        Vertices[3] = Vertices[0];

        for(uint32 VertexIndex = 0;
            VertexIndex < 6;
            ++VertexIndex)
        {
            Vertices[VertexIndex].Color = Color;
            Vertices[VertexIndex].HalfThickness = 0.f;
        }

        PushVertices(RenderState, Vertices);
    }
    else
    {
        Log(LOG_ERROR, "RenderState->ActiveFont is null...");
    }
}

internal void
RenderPushString(render_state *RenderState, string_u8 Text, vec2 Position, vec4 Color)
{
    vec2 DrawPosition = Position;
    venture_dynamic_font_varient *Metrics = RenderState->ActivePixelSize;
    
    for(uint32 StringIndex = 0;
        StringIndex < Text.Length;
        ++StringIndex)
    {
        uint8 *pCharacter = Text.Data + StringIndex;
        uint8  Character  = *pCharacter;
        if(Character == '\n' || Character == '\r')
        {
            DrawPosition.X = 0;
            DrawPosition.Y += Metrics->TypicalAscender;

            continue;
        }
        
        glyph_data *Glyph = GetUTF8Glyph(Metrics, pCharacter);
        if(Character == '\t' || Character == ' ')
        {
            DrawPosition.X += Glyph->Advance;
        }
        else
        {
            PushGlyph(RenderState, (char *)pCharacter,
                      {floorf(DrawPosition.X + Glyph->OffsetX), floorf(DrawPosition.Y - Glyph->Ascent)},
                      Color);

            DrawPosition.X += Glyph->Advance;
        }
    }
}

internal void 
RenderPushRectangle(render_state *RenderState, vec2 Position, vec2 Size, real32 Roundness, real32 Thickness, vec4 Color)
{    
    render_vertex Vertices[6] = {};
    real32        HalfThickness = Thickness;

    Vertices[0].Position = {Position.X         , Position.Y};
    Vertices[1].Position = {Position.X + Size.X, Position.Y};
    Vertices[2].Position = {Position.X + Size.X, Position.Y + Size.Y};
    Vertices[5].Position = {Position.X         , Position.Y + Size.Y};

    Vertices[4] = Vertices[2];
    Vertices[3] = Vertices[0];

  vec2 Center = {Position.X + (Size.X * 0.5f), Position.Y + (Size.Y * 0.5f)};
  for(int32 Index = 0;
        Index < 6;
        ++Index)
    {
        Vertices[Index].TexCoords     = Center;
        Vertices[Index].Color         = Color;
        Vertices[Index].HalfThickness = HalfThickness;
        Vertices[Index].Roundness     = Roundness;
    }

    PushVertices(RenderState, Vertices);
}
