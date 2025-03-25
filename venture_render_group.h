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

struct rect
{
    ivec2 Position;
    ivec2 Size;
};

struct render_group
{
    vertex       *GroupVertices;
    uint32        GroupVertexCount;

    render_group *NextGroup;
};

struct render_target
{
    rect           RenderRect;

    memory_arena  *Arena;
    render_group  *FirstRenderGroup;
    render_group  *FirstFreeRenderGroup;
};

struct render_state
{
    uint32         VAOID;
    uint32         VBOID;
    shader_program Shader;

    // IMPORTANT(Sleepster): This is a Dynamic Array
    render_target *RenderTargets;
};
#endif
