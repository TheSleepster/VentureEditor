#if !defined(VENTURE_BASE_STRING_H)
/* ========================================================================
   $File: venture_base_string.h $
   $Date: Thu, 13 Mar 25: 03:04PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_BASE_STRING_H

#include <venture_base_types.h>
#include <venture_base_memory.h>

// NOTE(Sleepster): THESE ARE READ ONLY
struct string_u8
{
    uint8 *Data;
    uint32 Length;
};

#define str_lit(x) (StringCreate(x, ArrayCount(x)))
#define c_str(x)   ((const char *)x.Data)

#define stringify_(s) #s
#define stringify(s) stringify_(s)
#define glue__(a, b) a##b
#define glue(a, b)   glue__(a, b)

internal string_u8
StringCreate(char *StringData, uint32 Length)
{
    string_u8 Result;
    Result.Data   = (uint8 *)StringData;
    Result.Length = Length;

    return(Result);
}

internal string_u8
StringSubFromLeft(string_u8 String, uint32 Index)
{
    string_u8 Result;
    Result.Data   = String.Data + Index;
    Result.Length = String.Length - Index;

    return(Result);
}

internal string_u8
StringSubFromRight(string_u8 String, uint32 Index)
{
    string_u8 Result;
    Result.Data   = (String.Data + String.Length) - Index;
    Result.Length = Index;

    return(Result);
}

internal string_u8
StringSubstring(string_u8 String, uint32 FirstIndex, uint32 LastIndex)
{
    string_u8 Result;
    Result.Data   = (String.Data + FirstIndex);
    Result.Length = LastIndex - FirstIndex;

    return(Result);
}

internal string_u8
StringConcat(memory_arena *Arena, string_u8 A, string_u8 B)
{
    string_u8 Result;

    uint32 NewLength = A.Length + B.Length;
    Result.Data   = ArenaPushArray(Arena, uint8, NewLength);
    Result.Length = NewLength;

    memcpy(Result.Data, A.Data, A.Length);
    memcpy(Result.Data + A.Length, B.Data, B.Length);

    return(Result);
}

#endif
