#if !defined(VENTURE_RENDER_GROUP_H)
/* ========================================================================
   $File: venture_render_group.h $
   $Date: Tue, 25 Mar 25: 12:02AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_RENDER_GROUP_H
#include <venture_dynamic_font.h>

constexpr uint32 MaxRenderTargets = 10;

struct render_state
{
    memory_arena   Arena;
    
    uint32         VAOID;
    uint32         VBOID;
    shader_program Shader;

    render_vertex *Vertices;
    uint32         VertexCount;

    venture_dynamic_render_font  *ActiveFont;
    venture_dynamic_font_varient *ActivePixelSize;
};

#endif
