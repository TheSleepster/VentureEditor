#if !defined(VENTURE_WIN32_H)
/* ========================================================================
   $File: venture_win32.h $
   $Date: Wed, 12 Mar 25: 02:11PM $
   $Revision: $
   $Creator: Justin Lewis $
   ======================================================================== */

#define VENTURE_WIN32_H

#include <glcorearb.h>
#include <glext.h>
#include <wglext.h>

struct wgl_functions
{
    PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT;
};

#endif
