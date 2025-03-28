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

enum keycodeID 
{ 
    // Mouse buttons
    KEY_LEFT_MOUSE = VK_LBUTTON,      // 0x01
    KEY_MIDDLE_MOUSE = VK_MBUTTON,    // 0x04
    KEY_RIGHT_MOUSE = VK_RBUTTON,     // 0x02
    
    // Letters (A-Z), using ASCII values which match VK codes
    KEY_A = 'A',    // 0x41
    KEY_B = 'B',    // 0x42
    KEY_C = 'C',    // 0x43
    KEY_D = 'D',    // 0x44
    KEY_E = 'E',    // 0x45
    KEY_F = 'F',    // 0x46
    KEY_G = 'G',    // 0x47
    KEY_H = 'H',    // 0x48
    KEY_I = 'I',    // 0x49
    KEY_J = 'J',    // 0x4A
    KEY_K = 'K',    // 0x4B
    KEY_L = 'L',    // 0x4C
    KEY_M = 'M',    // 0x4D
    KEY_N = 'N',    // 0x4E
    KEY_O = 'O',    // 0x4F
    KEY_P = 'P',    // 0x50
    KEY_Q = 'Q',    // 0x51
    KEY_R = 'R',    // 0x52
    KEY_S = 'S',    // 0x53
    KEY_T = 'T',    // 0x54
    KEY_U = 'U',    // 0x55
    KEY_V = 'V',    // 0x56
    KEY_W = 'W',    // 0x57
    KEY_X = 'X',    // 0x58
    KEY_Y = 'Y',    // 0x59
    KEY_Z = 'Z',    // 0x5A
    
    // Numbers (0-9), using ASCII values which match VK codes
    KEY_0 = '0',    // 0x30
    KEY_1 = '1',    // 0x31
    KEY_2 = '2',    // 0x32
    KEY_3 = '3',    // 0x33
    KEY_4 = '4',    // 0x34
    KEY_5 = '5',    // 0x35
    KEY_6 = '6',    // 0x36
    KEY_7 = '7',    // 0x37
    KEY_8 = '8',    // 0x38
    KEY_9 = '9',    // 0x39
    
    // Special keys and punctuation (using VK constants)
    KEY_SPACE = VK_SPACE,             // 0x20
    KEY_TICK = VK_OEM_3,              // 0xC0, backtick/tilde (`/~) on US keyboards
    KEY_MINUS = VK_OEM_MINUS,         // 0xBD, minus (-)
    KEY_EQUAL = VK_OEM_PLUS,          // 0xBB, equal (=)
    KEY_LEFT_BRACKET = VK_OEM_4,      // 0xDB, left bracket ([)
    KEY_RIGHT_BRACKET = VK_OEM_6,     // 0xDD, right bracket (])
    KEY_SEMICOLON = VK_OEM_1,         // 0xBA, semicolon (;)
    KEY_QUOTE = VK_OEM_7,             // 0xDE, single quote (')
    KEY_COMMA = VK_OEM_COMMA,         // 0xBC, comma (,)
    KEY_PERIOD = VK_OEM_PERIOD,       // 0xBE, period (.)
    KEY_FORWARD_SLASH = VK_OEM_2,     // 0xBF, forward slash (/)
    KEY_BACKWARD_SLASH = VK_OEM_5,    // 0xDC, backward slash (\)
    KEY_TAB = VK_TAB,                 // 0x09
    KEY_ESCAPE = VK_ESCAPE,           // 0x1B
    KEY_PAUSE = VK_PAUSE,             // 0x13
    KEY_UP = VK_UP,                   // 0x26
    KEY_DOWN = VK_DOWN,               // 0x28
    KEY_LEFT = VK_LEFT,               // 0x25
    KEY_RIGHT = VK_RIGHT,             // 0x27
    KEY_BACKSPACE = VK_BACK,          // 0x08
    KEY_RETURN = VK_RETURN,           // 0x0D
    KEY_DELETE = VK_DELETE,           // 0x2E
    KEY_INSERT = VK_INSERT,           // 0x2D
    KEY_HOME = VK_HOME,               // 0x24
    KEY_END = VK_END,                 // 0x23
    KEY_PAGE_UP = VK_PRIOR,           // 0x21
    KEY_PAGE_DOWN = VK_NEXT,          // 0x22
    KEY_CAPS_LOCK = VK_CAPITAL,       // 0x14
    KEY_NUM_LOCK = VK_NUMLOCK,        // 0x90
    KEY_SCROLL_LOCK = VK_SCROLL,      // 0x91
    KEY_MENU = VK_APPS,               // 0x5D, context menu key
    KEY_SHIFT = VK_SHIFT,             // 0x10, general Shift key
    KEY_CONTROL = VK_CONTROL,         // 0x11, general Control key
    KEY_ALT = VK_MENU,                // 0x12, Alt key
    KEY_COMMAND = VK_LWIN,            // 0x5B, left Windows key
    
    // Function keys
    KEY_F1 = VK_F1,      // 0x70
    KEY_F2 = VK_F2,      // 0x71
    KEY_F3 = VK_F3,      // 0x72
    KEY_F4 = VK_F4,      // 0x73
    KEY_F5 = VK_F5,      // 0x74
    KEY_F6 = VK_F6,      // 0x75
    KEY_F7 = VK_F7,      // 0x76
    KEY_F8 = VK_F8,      // 0x77
    KEY_F9 = VK_F9,      // 0x78
    KEY_F10 = VK_F10,    // 0x79
    KEY_F11 = VK_F11,    // 0x7A
    KEY_F12 = VK_F12,    // 0x7B
    
    // Numpad keys
    KEY_NUMPAD_0 = VK_NUMPAD0,     // 0x60
    KEY_NUMPAD_1 = VK_NUMPAD1,     // 0x61
    KEY_NUMPAD_2 = VK_NUMPAD2,     // 0x62
    KEY_NUMPAD_3 = VK_NUMPAD3,     // 0x63
    KEY_NUMPAD_4 = VK_NUMPAD4,     // 0x64
    KEY_NUMPAD_5 = VK_NUMPAD5,     // 0x65
    KEY_NUMPAD_6 = VK_NUMPAD6,     // 0x66
    KEY_NUMPAD_7 = VK_NUMPAD7,     // 0x67
    KEY_NUMPAD_8 = VK_NUMPAD8,     // 0x68
    KEY_NUMPAD_9 = VK_NUMPAD9,     // 0x69
    
    KEY_NUMPAD_STAR = VK_MULTIPLY,   // 0x6A
    KEY_NUMPAD_PLUS = VK_ADD,        // 0x6B
    KEY_NUMPAD_MINUS = VK_SUBTRACT,  // 0x6D
    KEY_NUMPAD_DOT = VK_DECIMAL,     // 0x6E
    KEY_NUMPAD_SLASH = VK_DIVIDE,    // 0x6F
    
    // Total count for array indexing
    KEY_COUNT = 256
};

#endif
