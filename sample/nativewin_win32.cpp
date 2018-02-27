#include "nativewin.h"
#include <windows.h>
#include <windowsx.h>

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    unsigned int key = 0;
    // Handle relevant messages individually
    switch(uMsg)
    {
    case WM_ACTIVATE:
    case WM_SETFOCUS:
        return 0;
    case WM_SIZE:
        OnNativeWinResize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
        switch (wParam)
        {
            case VK_ESCAPE:
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                return 0;
            default:
                break;
        }
        break;
    case WM_MOUSEMOVE:
        OnNativeWinMouseMove( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (wParam & MK_LBUTTON) != 0);
        break;
    default:
        break;
    }

    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

bool OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out)
{
    *nativedisp_out = (EGLNativeDisplayType) NULL;
    return true;
}

void CloseNativeDisplay(EGLNativeDisplayType nativedisp)
{
}

bool CreateNativeWin(EGLNativeDisplayType nativedisp, int width, int height, int visid, EGLNativeWindowType* nativewin_out)
{
    bool result = true;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HWND hWnd = NULL;
    DWORD dwExtStyle;
    DWORD dwWindStyle;

    TCHAR szWindowName[50] =  TEXT("OpenGL ES Sample");
    TCHAR szClassName[50]  =  TEXT("OGL_CLASS");

    // setup window class
    WNDCLASS wndClass;
    wndClass.lpszClassName = szClassName;                // Set the name of the Class
    wndClass.lpfnWndProc   = (WNDPROC)WndProc;
    wndClass.hInstance     = hInstance;              // Use this module for the module handle
    wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);// Pick the default mouse cursor
    wndClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);// Pick the default windows icons
    wndClass.hbrBackground = NULL;                       // No Background
    wndClass.lpszMenuName  = NULL;                       // No menu for this window
    wndClass.style         = CS_HREDRAW | CS_OWNDC |     // set styles for this class, specifically to catch
                                    CS_VREDRAW;               // window redraws, unique DC, and resize
    wndClass.cbClsExtra    = 0;                          // Extra class memory
    wndClass.cbWndExtra    = 0;                          // Extra window memory

    // Register the newly defined class
    if(!RegisterClass( &wndClass ))
    {
        result = false;
    }

    if(result)
    {
        dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwWindStyle = WS_OVERLAPPEDWINDOW;
        ShowCursor(TRUE);

        RECT windowRect;
        windowRect.left   = 0;
        windowRect.right  = width;
        windowRect.top    = 0;
        windowRect.bottom = height;

        // Setup window width and height
        AdjustWindowRectEx(&windowRect, dwWindStyle, FALSE, dwExtStyle);

        //Adjust for adornments
        int nWindowWidth  = windowRect.right  - windowRect.left;
        int nWindowHeight = windowRect.bottom - windowRect.top;

        // Create window
        hWnd = CreateWindowEx(
            dwExtStyle,      // Extended style
            szClassName,     // class name
            szWindowName,    // window name
            dwWindStyle |
            WS_CLIPSIBLINGS |
            WS_CLIPCHILDREN, // window stlye
            0,               // window position, x
            0,               // window position, y
            nWindowWidth,    // height
            nWindowHeight,   // width
            NULL,            // Parent window
            NULL,            // menu
            hInstance,       // instance
            NULL);           // pass this to WM_CREATE

        ShowWindow(hWnd, SW_SHOWDEFAULT);
    }
    *nativewin_out = (EGLNativeWindowType) hWnd;
    return result;
}

void DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
    WINDOWINFO info;
    GetWindowInfo((HWND) nativewin, &info);
    DestroyWindow((HWND) nativewin);
    UnregisterClass((LPCTSTR) info.atomWindowType, GetModuleHandle(NULL));
}

bool UpdateNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
    bool result = true;
    // Peek or wait for messages
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message==WM_QUIT)
        {
            result = false;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }
    return result;
}
