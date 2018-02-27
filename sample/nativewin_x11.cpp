#include "nativewin.h"
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out)
{
    *nativedisp_out = (EGLNativeDisplayType) XOpenDisplay(NULL);
    return *nativedisp_out != 0;
}

void CloseNativeDisplay(EGLNativeDisplayType nativedisp)
{
    XCloseDisplay((Display*) nativedisp);
}

bool CreateNativeWin(EGLNativeDisplayType nativedisp, int width, int height, int visid, EGLNativeWindowType* nativewin_out)
{
    bool success = true;
    Display* xdisplay = (Display*) nativedisp;
    int screen;
    int n;
    Window xroot;
    XVisualInfo vistemp;
    XVisualInfo *visinfo;
    Visual* visual;
    Colormap colormap;
    XSetWindowAttributes swa;
    Window xwin;
    Atom wmdelete;
    // find the screen and the root window
    screen = XDefaultScreen(xdisplay);
    xroot = XRootWindow(xdisplay, screen);
    // attempt to find the visual mode
    visual = NULL;
    vistemp.visualid = visid;
    visinfo = XGetVisualInfo(xdisplay, VisualIDMask, &vistemp, &n);
    if(visinfo != NULL)
    {
        // found a visual id matching the input
        visual = visinfo->visual;
    }
    else if(visid == 0)
    {
        // if the provided visual id isn't valid, use the default
        visual = XDefaultVisual(xdisplay, screen);
    }
    if(visual == NULL)
    {
        success = false;
    }
    if(success)
    {
        colormap = XCreateColormap(xdisplay, xroot, visual, AllocNone);
    }
    // create a window
    if(success)
    {
        swa.background_pixel = 0;
        swa.border_pixel = 0;
        swa.colormap = colormap;
        swa.event_mask =
            ExposureMask |
            FocusChangeMask |
            KeyPressMask |
            KeyReleaseMask |
            ButtonPressMask |
            ButtonReleaseMask |
            PointerMotionMask |
            StructureNotifyMask;
        xwin = XCreateWindow(
            xdisplay,
            xroot,
            20,
            20,
            width,
            height,
            0,
            XDefaultDepth(xdisplay, screen),
            InputOutput,
            visual,
            CWBackPixel|CWBorderPixel|CWColormap|CWEventMask,
            &swa);
        wmdelete = XInternAtom(xdisplay, "WM_DELETE_WINDOW", True);
        XSetWMProtocols(xdisplay, xwin, &wmdelete, 1);
        // make the window visible
        XMapWindow(xdisplay, xwin);
        // make sure an initial resize event is provided to the application
        OnNativeWinResize(width, height);
        // set out param
        *nativewin_out = (EGLNativeWindowType) xwin;
    }
    return success;
}

void DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
    Display* xdisplay = (Display*) nativedisp;
    Window xwin = (Window) nativewin;
    XWindowAttributes attr;
    XGetWindowAttributes(xdisplay, xwin, &attr);
    XUnmapWindow(xdisplay, xwin);
    XDestroyWindow(xdisplay, xwin);
    XFreeColormap(xdisplay, attr.colormap);
}

bool UpdateNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
    bool result = true;
    Display* xdisplay = (Display*) nativedisp;
    Window xwin = (Window) nativewin;
    XEvent evt;
    int w;
    int h;

    while(XEventsQueued(xdisplay, QueuedAfterFlush))
    {
        XNextEvent(xdisplay, &evt);
        switch(evt.type)
        {
        case ClientMessage:
            // close window
            if(evt.xmotion.window == xwin)
            {
                long wmdel;
                wmdel = (long) XInternAtom(xdisplay, "WM_DELETE_WINDOW", True);
                if(evt.xclient.data.l[0] == wmdel)
                {
                    result = false;
                }
            }
            break;
        case ConfigureNotify:
            // assume it's a window resize
            if(evt.xconfigure.window == xwin)
            {
                w = evt.xconfigure.width;
                h = evt.xconfigure.height;
                OnNativeWinResize(w, h);
            }
            break;
        case MotionNotify:
            // mouse cursor moved
            if(evt.xmotion.window == xwin)
            {
                int cursorx = evt.xmotion.x;
                int cursory = evt.xmotion.y;
                bool button1 = (evt.xmotion.state & Button1Mask) != 0;
                OnNativeWinMouseMove(cursorx, cursory, button1);
            }
            break;
        }
    }
    return result;
}

