#if !defined(VENTURE_PLATFORM_H)
/* ========================================================================
   $File: venture_platform.h $
   $Date: Wed, 12 Mar 25: 12:37PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_PLATFORM_H
#include <venture_base_types.h>
#include <venture_base_memory.h>

struct memory_arena;
struct platform_data_handles;
struct string_u8;

#define HANDLE void*
struct platform_data_handles
{
    HANDLE WindowHandle;
    HANDLE DeviceContext;
    HANDLE DeviceInstance;
    HANDLE RenderingContext;
};

struct application_memory
{
    memory_pool PrimaryPool;
};

// TODO(Sleepster): What the fuck do I do with this? 
global bool8 AppIsRunning;
global ivec2 ClientSize;

// PLATFORM STUFF
#define PLATFORM_VIRTUAL_ALLOC(name)       void*  name(uint64 Size)
#define PLATFORM_HEAP_ALLOC(name)          void*  name(uint64 Size)
#define PLATFORM_HEAP_REALLOC(name)        void*  name(void *OldData, uint64 NewSize)
#define PLATFORM_HEAP_FREE(name)           void*  name(void *Data)

#define PLATFORM_READ_ENTIRE_FILE(name)    uint8* name(memory_arena *Arena, string_u8 Filepath, uint32 *FileSizeOut)
#define PLATFORM_POLL_EVENTS(name)         void   name(platform_data_handles *PlatformHandleData)
#define PLATFORM_SWAP_RENDER_BUFFERS(name) void   name(platform_data_handles *PlatformHandleData)
#define PLATFORM_GET_CLIENT_SIZE(name)     ivec2  name(platform_data_handles *PlatformHandleData)


internal inline PLATFORM_VIRTUAL_ALLOC(PlatformVirtualAlloc);
internal inline PLATFORM_HEAP_ALLOC(PlatformHeapAlloc);
internal inline PLATFORM_HEAP_REALLOC(PlatformHeapRealloc);
internal inline PLATFORM_HEAP_FREE(PlatformHeapFree);

internal PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile);
internal PLATFORM_POLL_EVENTS(PlatformPollEvents);

internal inline PLATFORM_SWAP_RENDER_BUFFERS(PlatformSwapRenderBuffers);
internal inline PLATFORM_GET_CLIENT_SIZE(PlatformGetClientSize);

#endif
