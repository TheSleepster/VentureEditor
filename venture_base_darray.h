#if !defined(VENTURE_BASE_DARRAY_H)
/* ========================================================================
   $File: venture_base_darray.h $
   $Date: Fri, 14 Mar 25: 01:25PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_BASE_DARRAY_H

/*
    Justin 3-14-2025:

    These use the platform heap allocators (like HeapAlloc on Windows)
    and not arenas, keep that in mind when making many of these. I
    figured this is fine since I don't use these that often, but if
    you do use them often keep that in mind. You may want to move to
    VirtualAlloc-like functionality with arenas if you use them often
    as the memory fragmentation caused by functions like HeapAlloc can
    be bad, but it's better than standard linear allocators. Also, If
    you you instead grow with HeapReAlloc(), the fragmentation almost
    doesn't exist. It's just annoying because now the allocations are
    out of your control and are left to the OS implementation...
*/

constexpr uint32 DefaultDArrayCapacity = 10;
constexpr uint32 DArrayResizeFactor = 2;

enum darray_header_data
{
    DARRAY_MAX_CAPACITY,
    DARRAY_ELEMENT_STRIDE,
    DARRAY_ELEMENT_USED,

    DARRAY_HEADER_SIZE,
};

#define DArrayGetCapacity(array)                    DArrayGetHeaderInfo(array, DARRAY_MAX_CAPACITY)
#define DArrayGetElementSize(array)                 DArrayGetHeaderInfo(array, DARRAY_ELEMENT_STRIDE)
#define DArrayGetUsedElementCount(array)            DArrayGetHeaderInfo(array, DARRAY_ELEMENT_USED)

#define DArraySetCapacity(array, new_value)         DArraySetHeaderInfo(array, DARRAY_MAX_CAPACITY, new_value)
#define DArraySetUsedElementCount(array, new_value) DArraySetHeaderInfo(array, DARRAY_ELEMENT_USED, new_value)
#define DArraySetArrayElementSize(array, new_size)  DArraySetHeaderInfo(array, DARRAY_ELEMENT_STRIDe, new_size)

// API MACROS
#define DArrayCreate(type, ...)                     DArrayCreate_(sizeof(type), ##__VA_ARGS__)
#define DArrayInsertAt(array, valueptr, index)      DArrayInsert_(array, valueptr, sizeof(*valueptr), index)

// TODO(Sleepster): This is only MSVC for now, we will need to port
// this to compilers like Clang and GCC. RANT: I don't know why the
// hell there's no standardization of getting the fucking TYPE OF
// SOMETHING PASSED. It's crazy to me. Templates suck ass.
#define DArrayAppendValue(array, value)                                              \
do{                                                                                  \
        decltype(value) Temp = value;                                                \
        array = (*(decltype(value)*)DArrayAppendValue_(array, &Temp, sizeof(value)));\
  }while(0);

internal inline uint64
DArrayGetHeaderInfo(void *Array, uint64 FieldIndex)
{
    uint64 *Header = (uint64 *)Array - DARRAY_HEADER_SIZE;
    uint64  HeaderValue = Header[FieldIndex];

    return(HeaderValue);
}

internal inline void
DArraySetHeaderInfo(void *Array, uint64 FieldIndex, uint64 Value)
{
    uint64 *Header = (uint64 *)Array - DARRAY_HEADER_SIZE;
    Header[FieldIndex] = Value;
}

internal void*
DArrayCreate_(uint64 TypeSize, uint64 InitialCapacity = DefaultDArrayCapacity)
{
    uint64 HeaderSize = DARRAY_HEADER_SIZE * sizeof(uint64);
    uint64 ArraySize  = TypeSize * InitialCapacity;

    uint64 *Array     = (uint64 *)PlatformHeapAlloc(ArraySize + HeaderSize);
    memset(Array, 0, ArraySize + HeaderSize);

    Array[DARRAY_MAX_CAPACITY]   = InitialCapacity;
    Array[DARRAY_ELEMENT_STRIDE] = TypeSize;
    Array[DARRAY_ELEMENT_USED]   = 0;

#ifdef INTERNAL_DEBUG
    Log(LOG_TRACE, "Dynamic Array with of size: '%d' has been created...", ArraySize + HeaderSize);
#endif

    return((void *)(Array + DARRAY_HEADER_SIZE));
}

internal inline void
DArrayDestroy(void *Array)
{
#ifdef INTERNAL_DEBUG
    uint64 Capacity    = DArrayGetCapacity(Array);
    uint64 ElementSize = DArrayGetElementSize(Array);
    Log(LOG_TRACE, "Dynamic Array with of size: '%d' has been freed...", Capacity * ElementSize);
#endif

    uint64 *Header = ((uint64 *)Array - DARRAY_HEADER_SIZE);
    PlatformHeapFree(Header);
}

internal void*
DArrayResize(void *Array)
{
    uint64 Capacity    = DArrayGetCapacity(Array);
    uint64 ElementSize = DArrayGetElementSize(Array);
    uint64 ArraySize   = (DARRAY_HEADER_SIZE * sizeof(uint64) + ((Capacity * DArrayResizeFactor) * ElementSize));

    uint64 *Header  = (uint64 *)Array - DARRAY_HEADER_SIZE;
    void *NewArray = PlatformHeapRealloc(Header, ArraySize + (DARRAY_HEADER_SIZE * sizeof(uint64)));

#ifdef INTERNAL_DEBUG
    Log(LOG_TRACE, "Dynamic Array has been resized from: '%d' to a new size of: '%d'...",
        Capacity * ElementSize, ArraySize);
#endif

    if(NewArray)
    {
        uint64 *NewHeader = (uint64 *)NewArray;
        NewHeader[DARRAY_MAX_CAPACITY] = Capacity * DArrayResizeFactor;
        NewArray = (void *)((uint64 *)NewArray + DARRAY_HEADER_SIZE);
    }
    else
    {
        Log(LOG_FATAL, "DArray Resize from size '%d' to size '%d' has failed...", ArraySize, ArraySize * DArrayResizeFactor);
        DebugHalt();
    }
    return(NewArray);
}

internal void*
DArrayAppendValue_(void *Array, void *Value, uint64 ElementSize)
{
    uint64 Capacity         = DArrayGetCapacity(Array);
    uint64 UsedElementCount = DArrayGetUsedElementCount(Array);
    uint64 ArrayElementSize = DArrayGetElementSize(Array);
    if(ElementSize == ArrayElementSize)
    {
        if(UsedElementCount + 1 >= Capacity)
        {
            Array = DArrayResize(Array);
        }

        uint64 BaseAddress = (uint64)Array;
        BaseAddress       += (UsedElementCount * ElementSize);
        memcpy((void *)BaseAddress, Value, ElementSize);

        DArraySetUsedElementCount(Array, ++UsedElementCount);
    }
    else
    {
        Log(LOG_ERROR,
            "You are attempting to insert an element of size '%d' into a DArray that only takes size '%d'...",
            ElementSize, ArrayElementSize);
        DebugHalt();
    }
    return(Array);
}

internal void*
DArrayPopLastIndex(void *Array, uint32 ReturnPointerDataSize)
{
    void *Result = 0;
    
    uint64 Capacity         = DArrayGetCapacity(Array);
    uint64 UsedElementCount = DArrayGetUsedElementCount(Array);
    uint64 ArrayElementSize = DArrayGetElementSize(Array);
    if(ReturnPointerDataSize != ArrayElementSize)
    {
        Log(LOG_WARNING, "You are returning a value of size '%d' to a pointer that wants a value of size '%'...",
            ArrayElementSize, ReturnPointerDataSize);
        DebugBreak();
    }

    uint64 BaseAddress = (uint64)Array;
    BaseAddress       += ((UsedElementCount - 1) * ArrayElementSize);
    memcpy(Result, (void *)BaseAddress, ArrayElementSize);
    DArraySetUsedElementCount(Array, --UsedElementCount);

    return(Result);
}

// NOTE(Sleepster): This doesn't grow by default. Maybe we can change that...
internal void
DArrayInsertAt_(void *Array, void *Value, uint32 ValueSize, uint64 Index)
{
    uint64 Capacity         = DArrayGetCapacity(Array);
    uint64 UsedElementCount = DArrayGetUsedElementCount(Array);
    uint64 ArrayElementSize = DArrayGetElementSize(Array);
    if(ValueSize != ArrayElementSize)
    {
        Log(LOG_ERROR, "You are attempting to insert a value at array index '%d', but the value has a size of '%' while the array is for elements of size '%'...",
            Index, ValueSize, ArrayElementSize);
    }

    if(Index <= Capacity)
    {
        uint64 BaseAddress = (uint64)Array;

        if(Index != UsedElementCount - 1)
        {
            memcpy((void*)(BaseAddress + ((Index + 1) * ArrayElementSize)),
                   (void*)(BaseAddress + (Index * ArrayElementSize)),
                   ArrayElementSize * (UsedElementCount - Index));
        }

        memcpy((void*)(BaseAddress + (Index * ArrayElementSize)), Value, ArrayElementSize);
        DArraySetUsedElementCount(Array, ++UsedElementCount);
    }
    else
    {
        Log(LOG_ERROR, "Cannot insert at index '%d' as the capacity for the array is '%d'...", Index, Capacity);
        DebugHalt();
        return;
    }
}

internal void*
DArrayRemoveAtIndex(void *Array, uint64 Index, uint32 ReturnPointerDataSize)
{
    void *Result = 0;
    
    uint64 Capacity         = DArrayGetCapacity(Array);
    uint64 UsedElementCount = DArrayGetUsedElementCount(Array);
    uint64 ArrayElementSize = DArrayGetElementSize(Array);
    if(ReturnPointerDataSize != ArrayElementSize)
    {
        Log(LOG_WARNING, "You are returning a value of size '%d' to a pointer that wants a value of size '%'...",
            ArrayElementSize, ReturnPointerDataSize);
        DebugBreak();
    }
    
    if(Index <= Capacity)
    {
        // Copy out the value
        uint64 BaseAddress = (uint64)Array;
        memcpy(Result, (void *)(BaseAddress + (Index * ArrayElementSize)), ArrayElementSize);

        // push-back the rest of the entries in front of the index
        if(Index != Capacity - 1)
        {
            memcpy((void *)(BaseAddress + (Index * ArrayElementSize)),
                   (void *)(BaseAddress + ((Index + 1) * ArrayElementSize)),
                   ArrayElementSize * (Capacity - Index));
        }

        DArraySetUsedElementCount(Array, --UsedElementCount);
    }
    else
    {
        Log(LOG_FATAL, "You cannot access index '%d' as the darray is of capacity '%'...", Index, Capacity);
        DebugBreak();
    }

    return(Result);
}

#endif
