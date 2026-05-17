#pragma once

/*
    MU_EditControl.h

    Custom SDL edit control for MU project.
    Uses MU_TextRenderer for:
        MU_FillRect
        MU_DrawRectEx
        MU_TextOutEx
        MU_GetTextWidthEx
        MU_GetTextSizeEx

    Features:
        - Focus state
        - SDL_StartTextInput / SDL_StopTextInput
        - Mouse click focus
        - Finger touch focus
        - SDL_TEXTINPUT
        - Backspace
        - Enter/Escape blur
        - Password mode
        - Caret blink
        - Placeholder text
        - Move-box helper: MU_EditAnyFocused()
*/

#include <SDL.h>
#include <stdbool.h>

#include "MU_UIRenderer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MU_EDIT_TEXT_MAX 256

struct MU_EditStyle
{
    unsigned char bgR, bgG, bgB, bgA;
    unsigned char borderR, borderG, borderB, borderA;
    unsigned char focusBorderR, focusBorderG, focusBorderB, focusBorderA;
    unsigned char textR, textG, textB, textA;
    unsigned char placeholderR, placeholderG, placeholderB, placeholderA;
    unsigned char caretR, caretG, caretB, caretA;

    float borderThickness;
    int paddingX;
    int paddingY;
};

struct MU_EditControl
{
    int x;
    int y;
    int w;
    int h;

    char text[MU_EDIT_TEXT_MAX];
    int len;
    int maxLen;

    char placeholder[MU_EDIT_TEXT_MAX];

    int fontSlot;

    bool focused;
    bool pressed;
    bool password;
    bool readonly;
    bool visible;
    bool enabled;

    bool showCaret;
    Uint64 lastCaretBlink;
    Uint32 caretBlinkMs;

    MU_EditStyle style;
};

/*
    Setup default style.
*/
MU_EditStyle MU_EditDefaultStyle();

/*
    Initialize edit control.
*/
void MU_EditInit(MU_EditControl* e,
                 int x, int y, int w, int h,
                 int maxLen,
                 int fontSlot,
                 bool password);

/*
    Change position/size.
*/
void MU_EditSetRect(MU_EditControl* e, int x, int y, int w, int h);

/*
    Set optional placeholder.
*/
void MU_EditSetPlaceholder(MU_EditControl* e, const char* placeholder);

/*
    Text operations.
*/
void MU_EditSetText(MU_EditControl* e, const char* text);
const char* MU_EditGetText(const MU_EditControl* e);
void MU_EditClear(MU_EditControl* e);

/*
    Focus operations.
*/
void MU_EditSetFocus(MU_EditControl* e);
void MU_EditClearFocus();
bool MU_EditHasFocus(const MU_EditControl* e);
bool MU_EditAnyFocused();
MU_EditControl* MU_EditGetFocused();

/*
    Event handling.
    Returns true if event was consumed by this edit control.
*/
bool MU_EditHandleEvent(MU_EditControl* e, const SDL_Event* ev);

/*
    Render edit control.
    Must be called between:
        MU_2DRenderer_Begin(...)
        MU_2DRenderer_End()
*/
void MU_EditRender(MU_EditControl* e);

/*
    Helpers.
*/
bool MU_EditHitTest(const MU_EditControl* e, int mx, int my);
void MU_EditAppendText(MU_EditControl* e, const char* input);
void MU_EditBackspace(MU_EditControl* e);


#ifdef __cplusplus
}
#endif
