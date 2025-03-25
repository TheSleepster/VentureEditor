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

internal inline rect
CreateRect(ivec2 Position, ivec2 Size)
{
    rect Result;
    Result.Position = Position;
    Result.Size     = Size;

    return(Result);
}

internal render_target*
RenderTargetCreate(memory_arena *Arena, render_state *RenderState)
{
    render_target *Result = {};
    if(RenderState->RenderTargets == null)
    {
        RenderState->RenderTargets = (render_target *)DArrayCreate(render_target*, 10);
    }
    Assert(RenderState->RenderTargets, "RenderState->RenderTargets is invalid...\n");

    ivec2 RenderTargetSize   = ivec2(ClientSize / int32(DArrayGetUsedElementCount(&RenderState->RenderTargets) + 1));
    ivec2 RenderTargetOffset = {0, 0}; 

    Result                   = ArenaPushStruct(Arena, render_target);
    Result->FirstRenderGroup = ArenaPushStruct(Arena, render_group);
    Result->RenderRect       = CreateRect(RenderTargetOffset, RenderTargetSize);
    Result->Arena            = Arena;
    
    DArrayAppendValue(RenderState->RenderTargets, Result);
    return(Result);
}
