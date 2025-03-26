/* ========================================================================
   $File: venture_renderer.cpp $
   $Date: Wed, 12 Mar 25: 03:29PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
#include <venture_opengl.h>
#include <venture_render_group.h>

internal venture_bitmap
VentureBitmapCreate(memory_arena *Arena, uint32 Width, uint32 Height, uint32 Channels)
{
    venture_bitmap Result;
    Result.ChannelCount = Channels;
    Result.Width        = Width;
    Result.Height       = Height;
    Result.PixelStride  = sizeof(uint8) * Channels;
    Result.Data         = ArenaPushArray(Arena, uint8, (Width * Height) * Channels);

    return(Result);
}

internal venture_texture
VentureCreateTextureFromBitmap(venture_bitmap *Bitmap)
{
    venture_texture Result;
    glGenTextures(1, &Result.TexID);
    glBindTexture(GL_TEXTURE_2D, Result.TexID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
                 Bitmap->Width, Bitmap->Height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, Bitmap->Data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return(Result);
}

internal void
VentureUpdateTextureFromBitmap(venture_texture *Result, venture_bitmap *Bitmap)
{
    glBindTexture(GL_TEXTURE_2D, Result->TexID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
                 Bitmap->Width, Bitmap->Height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, Bitmap->Data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

const char *VentureVertexShaderSource = R"FOO(
#version 330

uniform mat2 ViewMatrixTransform;
uniform vec2 ScreenSpaceTranslation;

layout(location = 0) in vec2  vPosition;
layout(location = 1) in vec2  vTexCoords;
layout(location = 2) in vec4  vColor;
layout(location = 3) in float vHalfThickness;

smooth out vec4  vOutColor;
smooth out vec2  vOutTexCoords;
smooth out float vOutHalfThickness;

void main()
{
    vOutColor         = vColor;
    vOutTexCoords     = vTexCoords;
    vOutHalfThickness = vHalfThickness;

    gl_Position       = vec4(ViewMatrixTransform * (vPosition - ScreenSpaceTranslation), 0.0, 1.0); 
}
)FOO";

const char *VentureFragmentShaderSource = R"FOO(
#version 330

uniform sampler2D FontAtlas;

smooth in vec4  vOutColor;
smooth in vec2  vOutTexCoords;
smooth in float vOutHalfThickness;

out vec4 vFragColor;
void main()
{
    vec4 BlendColor;

    vec2 NormalizedTexCoords = vOutTexCoords / vec2(2048, 2048); 
    vec4 TextureColor        = texture(FontAtlas, NormalizedTexCoords);
    float AdjustedAlpha      = clamp(TextureColor.a + vOutHalfThickness * 0.1, 0.0, 1.0);

    if(TextureColor.rgb == 0.0)
    {
        if(TextureColor.a == 0.0)
        {
            discard;
        }
        BlendColor = vec4(1.0);
    }
    else
    {
        BlendColor = TextureColor;
    }

    vFragColor = vec4(vOutColor.xyz * BlendColor.xyz, vOutColor.a * AdjustedAlpha);
}
)FOO";

internal shader_program
VentureLoadShader()
{
    shader_program Program;
    
    GLuint cVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cVertexShader, 1, &VentureVertexShaderSource, 0);
    glCompileShader(cVertexShader);

    GLuint cFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cFragmentShader, 1, &VentureFragmentShaderSource, 0);
    glCompileShader(cFragmentShader);

    Program.ID = glCreateProgram();
    glAttachShader(Program.ID, cVertexShader);
    glAttachShader(Program.ID, cFragmentShader);
    glLinkProgram(Program.ID);
    glValidateProgram(Program.ID);

    GLint Success = false;
    glGetProgramiv(Program.ID, GL_LINK_STATUS, &Success);
    if(!Success)
    {
        GLsizei Ignore = 0;
        char VertexErrors[KB(4)];
        char FragmentErrors[KB(4)];
        char ProgramErrors[KB(4)];
        glGetShaderInfoLog(cVertexShader, sizeof(VertexErrors), &Ignore, VertexErrors);
        glGetShaderInfoLog(cFragmentShader, sizeof(FragmentErrors), &Ignore, FragmentErrors);
        glGetProgramInfoLog(Program.ID, sizeof(ProgramErrors), &Ignore, ProgramErrors);

        Log(LOG_FATAL, "Shader compilation failed...");

        if(VertexErrors[0] != '\0')
        {
            Log(LOG_ERROR, "Vertex Shader errors:\n %s", VertexErrors);
        }

        if(FragmentErrors[0] != '\0')
        {
            Log(LOG_ERROR, "Fragment Shader errors:\n %s", FragmentErrors);
        }
        DebugHalt();
    }
    glDeleteShader(cVertexShader);
    glDeleteShader(cFragmentShader);

    Program.ViewMatrixUBOID      = glGetUniformLocation(Program.ID, "ViewMatrixTransform");
    Program.TransformVectorUBOID = glGetUniformLocation(Program.ID, "ScreenSpaceTranslation");
    Program.SamplerUBOID         = glGetUniformLocation(Program.ID, "FontAtlas");
    
    return Program;
}

internal void
VentureGLDebugErrorCallback(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const char *Message, const void *UserParam){
    switch (ID){
        case 131218:
        {
            // NOTE(Sleepster): Performance warning. Shut up.
        }break;
        default:
        {
            Log(LOG_INFO, "DEBUG CALLBACK MESSAGE: \n %s", Message);
        }break;
    }
}

internal void
VentureInitOpenGLRenderer(render_state *RenderState)
{
    // STATE INIT
    {
        glDisable(GL_DEPTH);
        
        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_FRAMEBUFFER_SRGB);

#if INTERNAL_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        if (glDebugMessageControl)
        {
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, false);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, 0, false);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, 0, true);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, 0, true);
        }
        
        if (glDebugMessageCallback)
        {
            glDebugMessageCallback(VentureGLDebugErrorCallback, 0);
        }
#endif
    }

    // BUFFER INIT
    {
        glGenVertexArrays(1, &RenderState->VAOID);
        glBindVertexArray(RenderState->VAOID);

        glGenBuffers(1, &RenderState->VBOID);
        glBindBuffer(GL_ARRAY_BUFFER, RenderState->VBOID);
        glBufferData(GL_ARRAY_BUFFER, sizeof(render_vertex) * MAX_VERTICES, 0, GL_STREAM_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)GetMemberOffset(render_vertex, Position));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)GetMemberOffset(render_vertex, TexCoords));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)GetMemberOffset(render_vertex, Color));
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(render_vertex), (void *)GetMemberOffset(render_vertex, HalfThickness));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // SHADER INIT
    {
        RenderState->Shader = VentureLoadShader();
    }

    // DATA INIT
    {
        RenderState->Vertices    = ArenaPushArray(&RenderState->Arena, render_vertex, MAX_VERTICES);
        RenderState->VertexCount = 0;
    }
}

internal void
VentureRenderOneFrame(render_state *RenderState)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, ClientSize.X, ClientSize.Y);
    glScissor( 0, 0, ClientSize.X, ClientSize.Y);

    // render_vertex Vertices[] =
    // {
    //     {.Position = {-100.0, -100.0}, .TexCoords = {0.0, 0.0}, .Color = {1.0, 0.0, 0.0, 1.0}},
    //     {.Position = { 100.0, -100.0}, .TexCoords = {0.0, 0.0}, .Color = {0.0, 1.0, 0.0, 1.0}},
    //     {.Position = { 100.0,  100.0}, .TexCoords = {0.0, 0.0}, .Color = {0.0, 0.0, 1.0, 1.0}},

    //     {.Position = {-100.0, -100.0}, .TexCoords = {0.0, 0.0}, .Color = {1.0, 0.0, 0.0, 1.0}},
    //     {.Position = { 100.0,  100.0}, .TexCoords = {0.0, 0.0}, .Color = {0.0, 1.0, 0.0, 1.0}},
    //     {.Position = {-100.0,  100.0}, .TexCoords = {0.0, 0.0}, .Color = {0.0, 0.0, 1.0, 1.0}},
    // };

    glUseProgram(RenderState->Shader.ID);
    glBindTexture(GL_TEXTURE_2D, RenderState->ActivePixelSize->FirstPage->AtlasTexture.TexID);

    mat2 ViewSpaceMatrix =
    {
        2.f / ClientSize.X, 0.0f,
        0.0f, -2.f / ClientSize.Y
    };

    glUniform2f(RenderState->Shader.TransformVectorUBOID, ClientSize.X / 2.f, ClientSize.Y / 2.f);
    glUniformMatrix2fv(RenderState->Shader.ViewMatrixUBOID, 1, GL_FALSE, &ViewSpaceMatrix.Elements[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, RenderState->VBOID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, RenderState->VertexCount * sizeof(render_vertex), &RenderState->Vertices[0]);

    // GL_TRIANGLE_STRIP???? & 4 VERTICES???
    glDrawArrays(GL_TRIANGLES, 0, RenderState->VertexCount);
    glFlush();
}
