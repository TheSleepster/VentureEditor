/* ========================================================================
   $File: venture_memory.cpp $
   $Date: Wed, 12 Mar 25: 06:53AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#if !defined(VENTURE_MEMORY_CPP)
#define VENTURE_MEMORY_CPP

#include <venture_base_memory.h>
#include <venture_platform.h>

internal memory_pool
MemoryPoolCreate(uint64 Size)
{
    memory_pool Result;
    Result.PoolSize = Size;
    Result.Data     = PlatformVirtualAlloc(Size);
    if(Result.Data)
    {
        Result.Offset = (uint8 *)Result.Data;
    }
    else
    {
        Log(LOG_ERROR, "Allocation of size '%d' Failed...\n", Size);

        memory_pool Error = {};
        return(Error);
    }
    
    return(Result);
}

internal memory_arena
ArenaCreate(memory_pool *ParentPool, uint64 ArenaSize)
{
    memory_arena Result;
    Result.Capacity  = ArenaSize;
    Result.Allocated = 0;
    Result.Base      = ParentPool->Offset;

    ParentPool->Offset += ArenaSize;
    return(Result);
}

internal uint32
ArenaGetAlignmentOffset(memory_arena *Arena, uint32 Alignment = 4)
{
    size_t Offset = (size_t)Arena->Base + Arena->Allocated;
    uint32 AlignmentMask   = Alignment - 1;
    uint32 AlignmentOffset = 0;

    if(Offset & AlignmentMask) // Checking if alignment is a power of two
    {
        AlignmentOffset = Alignment - (Offset & AlignmentMask); // If not, make it one.
    }

    return(AlignmentOffset);
}

internal void*
ArenaAllocate(memory_arena *Arena, uint64 Size, uint32 Alignment)
{
    void *Result;
    
    uint32 AlignmentOffset = ArenaGetAlignmentOffset(Arena, Alignment);
    Size += AlignmentOffset;

    Assert((Arena->Allocated + Size) <= Arena->Capacity,
           "Arena allocation with size '%d' would exceed the capacity of '%d'", Size, Arena->Capacity);
    Result = (void *)(Arena->Base + Arena->Allocated + AlignmentOffset);

    Arena->Allocated += Size;
    return(Result);
}

internal memory_arena
ArenaCreateSubArena(memory_arena *ParentArena, uint64 Capacity, uint32 Alignment = 4)
{
    Assert(ParentArena->Allocated + Capacity < ParentArena->Capacity,
           "Allocation of Subarena with size '%d' would Exceed the parent arena's capacity of '%d'...",
           Capacity,
           ParentArena->Capacity);
    
    memory_arena Result;
    Result.Capacity = Capacity;
    Result.Base = (uint8 *)ArenaPushSize(ParentArena, Capacity, Alignment);

    return(Result);
}

internal inline void
ArenaClear(memory_arena *Arena)
{
    Arena->Allocated = 0;
}

#endif
