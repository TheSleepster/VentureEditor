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
#include <venture_renderer.cpp>
#include <venture_dynamic_font.cpp>

internal void
VentureMain(platform_data_handles *PlatformHandles, application_memory *Memory)
{
    render_state RenderState;

    InitFontLoading();
    VentureInitOpenGLRenderer(&RenderState);

    while(AppIsRunning)
    {
        ClientSize = PlatformGetClientSize(PlatformHandles);
        PlatformPollEvents(PlatformHandles);
        
        VentureRenderOneFrame(&RenderState);
        PlatformSwapRenderBuffers(PlatformHandles);
    }
}
