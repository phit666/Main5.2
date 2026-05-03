// Starter bridge for replacing the old Win32 WndProc input with SDL2 events.
// Include this in non-Windows builds and call MU_ProcessSDLEvent(event) from your main loop.
#include "stdafx.h"

#ifndef _WIN32
#include "win_compat.h"
#include <SDL.h>

// These globals exist in the MU client. Keep as extern so this file links against the existing client.
extern bool g_bWndActive;
extern BOOL g_bUseWindowMode;
extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;
extern bool MouseMButton, MouseMButtonPop, MouseMButtonPush;
extern int MouseWheel;
extern float MouseX, MouseY;
extern float g_fScreenRate_x, g_fScreenRate_y;
extern int g_iNoMouseTime;
extern int g_iMousePopPosition_x, g_iMousePopPosition_y;
extern void SetEnterPressed(bool pressed);
extern void KillGLWindow();

static void MU_ResetMouseOnFocusLost() {
    if (g_bUseWindowMode == TRUE) {
        MouseLButton = MouseLButtonPop = MouseLButtonPush = false;
        MouseRButton = MouseRButtonPop = MouseRButtonPush = false;
        MouseMButton = MouseMButtonPop = MouseMButtonPush = false;
        MouseLButtonDBClick = false;
        MouseWheel = 0;
    }
}

void MU_ProcessSDLEvent(const SDL_Event& e) {
    MouseLButtonDBClick = false;
    if (MouseLButtonPop == true && (g_iMousePopPosition_x != (int)MouseX || g_iMousePopPosition_y != (int)MouseY))
        MouseLButtonPop = false;

    switch (e.type) {
        case SDL_QUIT:
            KillGLWindow();
            break;
        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) g_bWndActive = true;
            else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) { g_bWndActive = false; MU_ResetMouseOnFocusLost(); }
            break;
        case SDL_MOUSEMOTION:
            MouseX = (float)e.motion.x / g_fScreenRate_x;
            MouseY = (float)e.motion.y / g_fScreenRate_y;
            if (MouseX < 0) MouseX = 0; if (MouseX > 640) MouseX = 640;
            if (MouseY < 0) MouseY = 0; if (MouseY > 480) MouseY = 480;
            break;
        case SDL_MOUSEBUTTONDOWN:
            g_iNoMouseTime = 0;
            if (e.button.button == SDL_BUTTON_LEFT) { MouseLButtonPop = false; if (!MouseLButton) MouseLButtonPush = true; MouseLButton = true; }
            if (e.button.button == SDL_BUTTON_RIGHT){ MouseRButtonPop = false; if (!MouseRButton) MouseRButtonPush = true; MouseRButton = true; }
            if (e.button.button == SDL_BUTTON_MIDDLE){MouseMButtonPop = false; if (!MouseMButton) MouseMButtonPush = true; MouseMButton = true; }
            break;
        case SDL_MOUSEBUTTONUP:
            g_iNoMouseTime = 0;
            if (e.button.button == SDL_BUTTON_LEFT) { MouseLButtonPush=false; MouseLButtonPop=true; MouseLButton=false; g_iMousePopPosition_x=(int)MouseX; g_iMousePopPosition_y=(int)MouseY; }
            if (e.button.button == SDL_BUTTON_RIGHT){ MouseRButtonPush=false; if(MouseRButton) MouseRButtonPop=true; MouseRButton=false; }
            if (e.button.button == SDL_BUTTON_MIDDLE){MouseMButtonPush=false; if(MouseMButton) MouseMButtonPop=true; MouseMButton=false; }
            break;
        case SDL_MOUSEWHEEL:
            MouseWheel = e.wheel.y;
            break;
        case SDL_KEYDOWN:
            if (e.key.keysym.sym == SDLK_RETURN) SetEnterPressed(true);
            break;
    }
}
#endif
