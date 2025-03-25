#if !defined(VENTURE_OPENGL_H)
/* ========================================================================
   $File: venture_opengl.h $
   $Date: Mon, 24 Mar 25: 01:21PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_OPENGL_H
#include <venture_base_types.h>

constexpr uint64 MAX_VERTICES = 10000;

struct vertex
{
    vec2  Position;
    vec4  Color;
    vec2  TexCoords;
    //int32 TexIndex;
};

struct shader_program
{
    GLuint ID;
    GLuint ViewMatrixUBOID;
    GLuint TransformVectorUBOID;
    GLuint SamplerUBOID;
};

struct venture_bitmap
{
    uint32 ChannelCount;
    uint32 Width;
    uint32 Height;
    uint32 PixelStride;

    void  *Data;
};

struct venture_texture
{
    uint32 TexID;
    uint32 Width;
    uint32 Height;

    void  *Data;
};

#endif
