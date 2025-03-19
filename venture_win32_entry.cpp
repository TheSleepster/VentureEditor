/* ========================================================================
   $File: win32_venture.cpp $
   $Date: Wed, 12 Mar 25: 05:03AM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */
// WINDOWS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

// GLAD
#define GLAD_OPENGL_IMPL
#include <glad/glad.h> 

// WINDOWS OPENGL
#include <glcorearb.h>
#include <glext.h>
#include <wglext.h>

// FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H

// BASE DATA
#include <venture_base_types.h>
#include <venture_base_debug.h>
#include <venture_base_memory.h>
#include <venture_base_string.h>
#include <venture_base_math.h>

// PLATFORM DATA
#include <venture_win32.h>
#include <venture_platform.h>

// CORE
#include <venture_base_memory.cpp>
#include <venture_main.cpp>

internal inline
PLATFORM_HEAP_ALLOC(PlatformHeapAlloc)
{
    void *Result = {};
    
    HANDLE HeapHandle = GetProcessHeap();
    if(HeapHandle)
    {
        Result = HeapAlloc(HeapHandle, HEAP_ZERO_MEMORY, Size);
        if(Result)
        {
            return(Result);
        }
        HRESULT Error = HRESULT_FROM_WIN32(GetLastError());
        Log(LOG_FATAL, "HeapAlloc has failed...");
        DebugHalt();

        int x = 0;
    }
    return(nullptr);
}

internal inline
PLATFORM_HEAP_REALLOC(PlatformHeapRealloc)
{
    void *Result;
    
    HANDLE HeapHandle = GetProcessHeap();
    if(HeapHandle)
    {
        Result = HeapReAlloc(HeapHandle, 0, OldData, NewSize);
        if(Result)
        {
            return(Result);
        }
        HRESULT Error = GetLastError();
        Log(LOG_FATAL, "HeapReAlloc has failed");
        DebugHalt();

        int x = 0;
    }
    else
    {
        Log(LOG_FATAL, "Failed to get the heap...");
        DebugHalt();
    }

    Result = 0;
    return(Result);
}

internal inline 
PLATFORM_HEAP_FREE(PlatformHeapFree)
{
    HANDLE HeapHandle = GetProcessHeap();
    if(HeapHandle)
    {
        BOOL Success = HeapFree(HeapHandle, 0, Data);
        HRESULT Error = GetLastError();
        if(!Success)
        {
            Log(LOG_FATAL, "HeapFree has failed...");
        }        
    }
    else
    {
        Log(LOG_FATAL, "Failed to get the heap...");
        DebugHalt();
    }
}

internal inline
PLATFORM_VIRTUAL_ALLOC(PlatformVirtualAlloc)
{
    return(VirtualAlloc(0, Size, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE));
}

internal
PLATFORM_READ_ENTIRE_FILE(PlatformReadEntireFile)
{
    uint8 *Result = {};
    FileSizeOut   = 0;
    HANDLE FileHandle = CreateFileA(c_str(Filepath),
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   0,
                                   OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL,
                                   0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD FileSize = GetFileSize(FileHandle, 0);
        if(FileSize != 0)
        {
            *FileSizeOut = FileSize;

            if(Arena) Result = ArenaPushArray(Arena,  uint8,   FileSize);
            else      Result = (uint8 *)PlatformHeapAlloc(sizeof(uint8) * FileSize);
            Assert(Result, "Failed to allocate '%d' from our arena...\n", Result);

            BOOL Success = ReadFile(FileHandle, Result, FileSize, 0, 0);
            if(!Success)
            {
                HRESULT Error = HRESULT_FROM_WIN32(GetLastError());
                DebugHalt();
            }
        }
        else
        {
            HRESULT Error = HRESULT_FROM_WIN32(GetLastError());
            DebugHalt();
        }
    }
    else
    {
        HRESULT Error = HRESULT_FROM_WIN32(GetLastError());
        DebugHalt();
    }

    CloseHandle(FileHandle);
    return(Result);
}

internal void
Win32LoadWGLFunctions(WNDCLASS WindowClass, HINSTANCE hInstance, wgl_functions *WGLFunctions)
{
    HWND DummyWindow = 
        CreateWindow(WindowClass.lpszClassName,
                     "OpenGLFunctionGetter",
                     WS_OVERLAPPEDWINDOW,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     CW_USEDEFAULT,
                     0,
                     0,
                     hInstance,
                     0);
    HDC DummyContext = GetDC(DummyWindow);
    
    PIXELFORMATDESCRIPTOR DFormat = {};
    DFormat.nSize      = sizeof(DFormat);
    DFormat.nVersion   = 1;
    DFormat.dwFlags    = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
    DFormat.iPixelType = PFD_TYPE_RGBA;
    DFormat.cColorBits = 32;
    DFormat.cAlphaBits = 8;
    DFormat.cDepthBits = 24;
    
    int32 dPixelFormat = ChoosePixelFormat(DummyContext, &DFormat);
    PIXELFORMATDESCRIPTOR DSuggestedFormat;
    DescribePixelFormat(DummyContext, dPixelFormat, sizeof(DSuggestedFormat), &DSuggestedFormat);
    SetPixelFormat(DummyContext, dPixelFormat,  &DSuggestedFormat);
    
    HGLRC TempRC = wglCreateContext(DummyContext);
    wglMakeCurrent(DummyContext, TempRC);

    WGLFunctions->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
        wglGetProcAddress("wglChoosePixelFormatARB");
    WGLFunctions->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
        wglGetProcAddress("wglCreateContextAttribsARB");
    WGLFunctions->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
        wglGetProcAddress("wglSwapIntervalEXT");
    if(!WGLFunctions->wglChoosePixelFormatARB||
       !WGLFunctions->wglCreateContextAttribsARB||
       !WGLFunctions->wglSwapIntervalEXT)
    {
        Log(LOG_FATAL, "Failed to load the windows OpenGL functions...");
    }

    wglMakeCurrent(DummyContext, 0);
    wglDeleteContext(TempRC);
    ReleaseDC(DummyWindow, DummyContext);
    DestroyWindow(DummyWindow);
}

internal HGLRC
PlatformLoadGL(platform_data_handles *PlatformHandleData, wgl_functions *WGLFunctions)
{
    HDC    DeviceContext   = (HDC)PlatformHandleData->DeviceContext;
    HANDLE WindowHandle = (HWND)PlatformHandleData->WindowHandle;
    HGLRC  Result;
    
    // NOTE(Sleepster): Init OpenGL
    {
        const int32 PixelAttributes[] =
        {
            WGL_DRAW_TO_WINDOW_ARB,           GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB,           GL_TRUE,
            WGL_ACCELERATION_ARB,             WGL_FULL_ACCELERATION_ARB,
            WGL_DOUBLE_BUFFER_ARB,            GL_FALSE,
            WGL_PIXEL_TYPE_ARB,               WGL_TYPE_RGBA_ARB,
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
            0
        };
                
        const int32 ContextAttributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB| WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
                
        UINT NumPixelFormats;
        int32 PixelFormat = 0;
        if(!WGLFunctions->wglChoosePixelFormatARB(DeviceContext, PixelAttributes, 0, 1, &PixelFormat, &NumPixelFormats))
        {
            Log(LOG_FATAL, "Failed to choose the main pixel format!");
        }
                
        PIXELFORMATDESCRIPTOR MainPixelFormat;
        DescribePixelFormat(DeviceContext, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &MainPixelFormat);
        SetPixelFormat(DeviceContext, PixelFormat, &MainPixelFormat);
                
        Result = WGLFunctions->wglCreateContextAttribsARB(DeviceContext, 0, ContextAttributes);
        if(Result)
        {
            wglMakeCurrent(DeviceContext, Result);
            gladLoadGL();

            // VSYNC
            WGLFunctions->wglSwapIntervalEXT(1);
            // VSYNC

            const uint8 *VendorInfo    = glGetString(GL_VENDOR);
            const uint8 *RendererInfo  = glGetString(GL_RENDERER);
            const uint8 *VersionInfo   = glGetString(GL_VERSION);

            Log(LOG_INFO, "OpenGL Vendor:     %s", VendorInfo);
            Log(LOG_INFO, "OpenGL Renderer:   %s", RendererInfo);
            Log(LOG_INFO, "OpenGL Version:    %s", VersionInfo);
        }
        else
        {
            Log(LOG_FATAL, "Failed to create the main OpenGL RC");
        }
    }

    return Result;
}

internal
PLATFORM_POLL_EVENTS(PlatformPollEvents)
{
    MSG Message = {};
    while(PeekMessageA(&Message, (HWND)PlatformHandleData->WindowHandle, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_SIZE:
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }break;
        }
    }
}

internal inline
PLATFORM_SWAP_RENDER_BUFFERS(PlatformSwapRenderBuffers)
{
    SwapBuffers((HDC)PlatformHandleData->DeviceContext);
}

internal inline
PLATFORM_GET_CLIENT_SIZE(PlatformGetClientSize)
{
    ivec2 Result;
    
    RECT Rect = {};
    GetClientRect((HWND)PlatformHandleData->WindowHandle, &Rect);
    Result.X  = int32(Rect.right - Rect.left);
    Result.Y  = int32(Rect.bottom - Rect.top);

    return(Result);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND WindowHandle, UINT Message,
                        WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = {};
    switch(Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            AppIsRunning = false;
        }break;
        default:
        {
            Result = DefWindowProc(WindowHandle, Message, wParam, lParam);
        }break;
    }
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR     lpCmdLine,
        int32     nShowCmd)
{
    WNDCLASSA             WindowClass  = {};
    wgl_functions         WGLFunctions = {};
    application_memory    AppMemory    = {};
    platform_data_handles PlatformHandleData = {};
    
    WindowClass.style         = CS_OWNDC|CS_VREDRAW|CS_HREDRAW;
    WindowClass.lpfnWndProc   = Win32MainWindowCallback;
    WindowClass.hInstance     = hInstance;
    WindowClass.hCursor       = LoadCursor(0, IDC_ARROW); 
    WindowClass.lpszClassName = "Window";
    if(RegisterClass(&WindowClass))
    {
        ivec2 SizeData = {600, 600};

        RECT rect = {0, 0, SizeData.Width, SizeData.Height};
        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
        SizeData.Width  = rect.right  - rect.left;
        SizeData.Height = rect.bottom - rect.top;

        Win32LoadWGLFunctions(WindowClass, hInstance, &WGLFunctions);
        PlatformHandleData.DeviceInstance = hInstance;

        HWND WindowHandle = CreateWindowEx(WS_EX_CLIENTEDGE,
                                           WindowClass.lpszClassName,
                                          "Venture",
                                           WS_OVERLAPPEDWINDOW|WS_VISIBLE|CS_OWNDC,
                                           CW_USEDEFAULT, CW_USEDEFAULT,
                                           SizeData.Width, SizeData.Height,
                                           0, 0,
                                           hInstance,
                                           0);
        if(WindowHandle)
        {
#ifdef INTERNAL_DEBUG
            // NOTE(Sleepster): CONSOLE INIT 
            if(AllocConsole())
            {
                FILE *fp;
                freopen_s(&fp, "CONOUT$", "w", stdout);
                freopen_s(&fp, "CONOUT$", "w", stderr);
                freopen_s(&fp, "CONIN$",  "r", stdin);

                HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
                DWORD dwMode;
                GetConsoleMode(hOut, &dwMode);

                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
                
                SetConsoleTitle("Venture Debug Output");
            }
#endif
            
            HDC DeviceContext                   = GetDC(WindowHandle);
            PlatformHandleData.DeviceContext    = DeviceContext;
            PlatformHandleData.WindowHandle     = WindowHandle;
            PlatformHandleData.RenderingContext = PlatformLoadGL(&PlatformHandleData, &WGLFunctions);

            // NOTE(Sleepster): Init Memory Data
            {
                AppMemory.PrimaryPool   = MemoryPoolCreate(MB(100));
            }

            AppIsRunning = true;
            VentureMain(&PlatformHandleData, &AppMemory);
        }
        else
        {
            MessageBox(0, "[ERROR]: Venture failed to create the window...", 0, MB_OK|MB_ICONERROR);
        }
    }
    else
    {
        MessageBox(0, "[ERROR]: Venture failed to register the window class...", 0, MB_OK|MB_ICONERROR);
    }
    ExitProcess(0);
}
