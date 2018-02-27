#ifndef __NATIVEWIN_H__
#define __NATIVEWIN_H__

#include <EGL/egl.h>

void OnNativeWinResize(int width, int height);

void OnNativeWinMouseMove(int mousex, int mousey, bool lbutton);

bool OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out);

void CloseNativeDisplay(EGLNativeDisplayType nativedisp);

bool CreateNativeWin(EGLNativeDisplayType nativedisp, int width, int height, int visid, EGLNativeWindowType* nativewin_out);

void DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin);

bool UpdateNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin);

#endif // __NATIVEWIN_H__

