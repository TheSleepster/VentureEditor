#if !defined(VENTURE_BASE_MEMORY_H)
/* ========================================================================
   $File: venture_base_memory.h $
   $Date: Wed, 19 Mar 25: 03:54PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_BASE_MEMORY_H
#include <venture_base_types.h>

struct memory_pool
{
    void  *Data;
    uint8 *Offset;
    uint64 Used;
    uint64 PoolSize;
};

struct memory_arena
{
    uint64 Capacity;
    uint64 Allocated;
    uint8 *Base;
};

#define KB(x) (x     * 1000)
#define MB(x) (KB(x) * 1000)
#define GB(x) (MB(x) * 1000)

#define ArenaAllocateDEBUG(arena, size, ...) ArenaAllocate(arena, size, __FILE__, __LINE__, ##__VA_ARGS__)

#define ArenaPushSize(Arena, size, ...)                 ArenaAllocateDEBUG(Arena, size, ##__VA_ARGS__)
#define ArenaPushStruct(Arena, type, ...)       (type *)ArenaAllocateDEBUG(Arena, sizeof(type), ##__VA_ARGS__)
#define ArenaPushArray(Arena, type, Count, ...) (type *)ArenaAllocateDEBUG(Arena, sizeof(type) * (Count), ##__VA_ARGS__)

internal void* ArenaAllocate(memory_arena *Arena, uint64 Size, const char *File = null, int32 Line = -1, uint32 Alignment = 4);

#endif
