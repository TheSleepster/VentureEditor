/* ========================================================================
   $File: venture_renderer.cpp $
   $Date: Wed, 12 Mar 25: 03:29PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
constexpr uint64 MAX_VERTICES = 10000;

struct vertex
{
    vec2  Position;
    vec4  Color;
    vec2  TexCoords;
};

struct shader_program
{
    GLuint ID;
    GLuint ViewMatrixUBOID;
    GLuint TransformVectorUBOID;
    GLuint SamplerUBOID;
};

struct bitmap
{
    uint32 ChannelCount;
    uint32 Width;
    uint32 Height;
    uint32 PixelStride;

    void  *Data;
};

struct texture
{
    uint32 TexID;
    uint32 Width;
    uint32 Height;

    void  *Data;
};

struct render_state
{
    GLuint         VAOID;
    GLuint         VBOID;
    shader_program Shader;
};

internal bitmap
VentureBitmapCreate(memory_arena *Arena, uint32 Width, uint32 Height, uint32 Channels)
{
    bitmap Result;
    Result.ChannelCount = Channels;
    Result.Width        = Width;
    Result.Height       = Height;
    Result.PixelStride  = sizeof(uint8) * Channels;
    Result.Data         = ArenaPushArray(Arena, uint8, (Width * Height) * Channels);

    return Result;
}

internal texture
VentureCreateTextureFromBitmap(memory_arena *Arena, bitmap *Bitmap)
{
    texture Result;
    glGenTextures(1, &Result.TexID);
    glBindTexture(GL_TEXTURE_2D, Result.TexID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8,
                 Bitmap->Width, Bitmap->Height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, Bitmap->Data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return Result;
}

const char *VentureVertexShaderSource = R"FOO(
#version 330

uniform mat2 ViewMatrixTransform;
uniform vec2 ScreenSpaceTranslation;

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec4 vColor;
layout(location = 2) in vec2 vTexCoords;

out vec4 vOutColor;
out vec2 vOutTexCoords;

void main()
{
    vOutColor     = vColor;
    vOutTexCoords = vTexCoords;
    gl_Position   = vec4(ViewMatrixTransform * (vPosition - ScreenSpaceTranslation), 0, 1); 
}
)FOO";

const char *VentureFragmentShaderSource = R"FOO(
#version 330

uniform sampler2D FontAtlas;

in vec4 vOutColor;
in vec2 vOutTexCoords;

out vec4 vFragColor;
void main()
{
    vec4 BlendColor;
    vec4 TextureColor = texture(FontAtlas, vOutTexCoords, 0);
    if(TextureColor.rgb == 0.0)
    {
        BlendColor = vec4(1.0);
    }
    else
    {
        if(TextureColor.a == 0.0)
        {
            discard;
        }
        BlendColor = TextureColor;
    }

    vFragColor = vOutColor * BlendColor;
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
        glEnable(GL_BLEND);        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_FUNC_ADD);

        glEnable(GL_SCISSOR_TEST);

        glEnable(GL_FRAMEBUFFER_SRGB);
        glDisable(0x809D); // Disabling multisampling

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
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * MAX_VERTICES, 0, GL_STREAM_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)GetMemberOffset(vertex, Position));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)GetMemberOffset(vertex, Color));
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void *)GetMemberOffset(vertex, TexCoords));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // SHADER INIT
    {
        RenderState->Shader = VentureLoadShader();
    }
}

internal void
VentureRenderOneFrame(render_state *RenderState)
{
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(0, 0, ClientSize.X, ClientSize.Y);
    glScissor( 0, 0, ClientSize.X, ClientSize.Y);

    mat2 ViewSpaceMatrix =
    {
        2.f / ClientSize.X, 0.0f,
        0.0f, -2.f / ClientSize.Y
    };

    vertex Vertices[] =
    {
        {.Position = {-100.0, -100.0}, .Color = {1.0, 0.0, 0.0, 1.0}, .TexCoords = {0.0, 0.0}},
        {.Position = { 100.0, -100.0}, .Color = {0.0, 1.0, 0.0, 1.0}, .TexCoords = {0.0, 0.0}},
        {.Position = { 100.0,  100.0}, .Color = {0.0, 0.0, 1.0, 1.0}, .TexCoords = {0.0, 0.0}},

        {.Position = {-100.0, -100.0}, .Color = {1.0, 0.0, 0.0, 1.0}, .TexCoords = {0.0, 0.0}},
        {.Position = { 100.0,  100.0}, .Color = {0.0, 1.0, 0.0, 1.0}, .TexCoords = {0.0, 0.0}},
        {.Position = {-100.0,  100.0}, .Color = {0.0, 0.0, 1.0, 1.0}, .TexCoords = {0.0, 0.0}},
    };

    glUseProgram(RenderState->Shader.ID);
    glUniform2f(RenderState->Shader.TransformVectorUBOID, ClientSize.X / 2.f, ClientSize.Y /2.f);
    glUniformMatrix2fv(RenderState->Shader.ViewMatrixUBOID, 1, GL_FALSE, &ViewSpaceMatrix.Elements[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, RenderState->VBOID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, ArrayCount(Vertices) * sizeof(vertex), &Vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glFlush();
}
