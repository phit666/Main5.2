#include "MU_EditControl.h"

#include <cstring>

static MU_EditControl* g_muFocusedEdit = nullptr;

static int MU_StrLenSafe(const char* s)
{
    if (!s)
        return 0;

    return (int)std::strlen(s);
}

static void MU_CopyLimited(char* dst, int dstSize, const char* src)
{
    if (!dst || dstSize <= 0)
        return;

    if (!src)
        src = "";

    std::strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
}

MU_EditStyle MU_EditDefaultStyle()
{
    MU_EditStyle s;

    s.bgR = 0;
    s.bgG = 0;
    s.bgB = 0;
    s.bgA = 165;

    s.borderR = 130;
    s.borderG = 130;
    s.borderB = 130;
    s.borderA = 255;

    s.focusBorderR = 255;
    s.focusBorderG = 210;
    s.focusBorderB = 90;
    s.focusBorderA = 255;

    s.textR = 255;
    s.textG = 255;
    s.textB = 255;
    s.textA = 255;

    s.placeholderR = 150;
    s.placeholderG = 150;
    s.placeholderB = 150;
    s.placeholderA = 220;

    s.caretR = 255;
    s.caretG = 255;
    s.caretB = 255;
    s.caretA = 255;

    s.borderThickness = 1.0f;
    s.paddingX = 6;
    s.paddingY = 0;

    return s;
}

void MU_EditInit(MU_EditControl* e,
                 int x, int y, int w, int h,
                 int maxLen,
                 int fontSlot,
                 bool password)
{
    if (!e)
        return;

    std::memset(e, 0, sizeof(MU_EditControl));

    e->x = x;
    e->y = y;
    e->w = w;
    e->h = h;

    if (maxLen <= 0)
        maxLen = MU_EDIT_TEXT_MAX - 1;

    if (maxLen >= MU_EDIT_TEXT_MAX)
        maxLen = MU_EDIT_TEXT_MAX - 1;

    e->maxLen = maxLen;
    e->fontSlot = fontSlot;
    e->password = password;

    e->focused = false;
    e->pressed = false;
    e->readonly = false;
    e->visible = true;
    e->enabled = true;

    e->showCaret = true;
    e->lastCaretBlink = SDL_GetTicks64();
    e->caretBlinkMs = 500;

    e->style = MU_EditDefaultStyle();

    e->text[0] = '\0';
    e->placeholder[0] = '\0';
    e->len = 0;
}

void MU_EditSetRect(MU_EditControl* e, int x, int y, int w, int h)
{
    if (!e)
        return;

    e->x = x;
    e->y = y;
    e->w = w;
    e->h = h;

    if (MU_EditHasFocus(e))
    {
        SDL_Rect r = { e->x, e->y, e->w, e->h };
        SDL_SetTextInputRect(&r);
    }
}

void MU_EditSetPlaceholder(MU_EditControl* e, const char* placeholder)
{
    if (!e)
        return;

    MU_CopyLimited(e->placeholder, MU_EDIT_TEXT_MAX, placeholder);
}

void MU_EditSetText(MU_EditControl* e, const char* text)
{
    if (!e)
        return;

    MU_CopyLimited(e->text, MU_EDIT_TEXT_MAX, text);

    e->len = MU_StrLenSafe(e->text);

    if (e->len > e->maxLen)
    {
        e->len = e->maxLen;
        e->text[e->len] = '\0';
    }
}

const char* MU_EditGetText(const MU_EditControl* e)
{
    if (!e)
        return "";

    return e->text;
}

void MU_EditClear(MU_EditControl* e)
{
    if (!e)
        return;

    e->text[0] = '\0';
    e->len = 0;
}

void MU_EditSetFocus(MU_EditControl* e)
{
    if (!e || !e->enabled || !e->visible)
        return;

    if (g_muFocusedEdit && g_muFocusedEdit != e)
    {
        g_muFocusedEdit->focused = false;
        g_muFocusedEdit->pressed = false;
    }

    g_muFocusedEdit = e;

    e->focused = true;
    e->pressed = true;
    e->showCaret = true;
    e->lastCaretBlink = SDL_GetTicks64();

    SDL_Rect r = { e->x, e->y, e->w, e->h };
    SDL_SetTextInputRect(&r);
    SDL_StartTextInput();
}

void MU_EditClearFocus()
{
    if (g_muFocusedEdit)
    {
        g_muFocusedEdit->focused = false;
        g_muFocusedEdit->pressed = false;
    }

    g_muFocusedEdit = nullptr;
    SDL_StopTextInput();
}

bool MU_EditHasFocus(const MU_EditControl* e)
{
    return e && e->focused && g_muFocusedEdit == e;
}

bool MU_EditAnyFocused()
{
    return g_muFocusedEdit != nullptr && g_muFocusedEdit->focused;
}

MU_EditControl* MU_EditGetFocused()
{
    return g_muFocusedEdit;
}

bool MU_EditHitTest(const MU_EditControl* e, int mx, int my)
{
    if (!e || !e->visible || !e->enabled)
        return false;

    return mx >= e->x &&
           mx <= e->x + e->w &&
           my >= e->y &&
           my <= e->y + e->h;
}

void MU_EditAppendText(MU_EditControl* e, const char* input)
{
    if (!e || !input || e->readonly)
        return;

    /*
        This appends bytes from SDL_TEXTINPUT.
        For pure ASCII username/password this is perfect.
        For full UTF-8 editing, backspace/cursor would need UTF-8 aware handling.
    */
    while (*input && e->len < e->maxLen && e->len < MU_EDIT_TEXT_MAX - 1)
    {
        unsigned char c = (unsigned char)*input++;

        /*
            Optional basic filter:
            skip control characters.
        */
        if (c < 32)
            continue;

        e->text[e->len++] = (char)c;
    }

    e->text[e->len] = '\0';

    e->showCaret = true;
    e->lastCaretBlink = SDL_GetTicks64();
}

void MU_EditBackspace(MU_EditControl* e)
{
    if (!e || e->readonly)
        return;

    if (e->len <= 0)
        return;

    /*
        ASCII-safe backspace.
        If you need UTF-8 deletion later, we can add it.
    */
    e->len--;
    e->text[e->len] = '\0';

    e->showCaret = true;
    e->lastCaretBlink = SDL_GetTicks64();
}

bool MU_EditHandleEvent(MU_EditControl* e, const SDL_Event* ev)
{
    if (!e || !ev || !e->visible || !e->enabled)
        return false;

    switch (ev->type)
    {
    case SDL_MOUSEBUTTONDOWN:
    {
        if (ev->button.button == SDL_BUTTON_LEFT)
        {
            int mx = ev->button.x;
            int my = ev->button.y;

            if (MU_EditHitTest(e, mx, my))
            {
                MU_EditSetFocus(e);
                return true;
            }

            if (MU_EditHasFocus(e))
                MU_EditClearFocus();
        }
    }
    break;

    case SDL_MOUSEBUTTONUP:
    {
        if (MU_EditHasFocus(e))
            e->pressed = false;
    }
    break;

    case SDL_FINGERDOWN:
    {
        /*
            SDL finger coordinates are normalized 0..1.
            We need the current window size.
        */
        SDL_Window* win = SDL_GetMouseFocus();
        int ww = 0;
        int wh = 0;

        if (win)
            SDL_GetWindowSize(win, &ww, &wh);

        if (ww <= 0 || wh <= 0)
            return false;

        int mx = (int)(ev->tfinger.x * (float)ww);
        int my = (int)(ev->tfinger.y * (float)wh);

        if (MU_EditHitTest(e, mx, my))
        {
            MU_EditSetFocus(e);
            return true;
        }

        if (MU_EditHasFocus(e))
            MU_EditClearFocus();
    }
    break;

    case SDL_FINGERUP:
    {
        if (MU_EditHasFocus(e))
            e->pressed = false;
    }
    break;

    case SDL_TEXTINPUT:
    {
        if (MU_EditHasFocus(e))
        {
            MU_EditAppendText(e, ev->text.text);
            return true;
        }
    }
    break;

    case SDL_KEYDOWN:
    {
        if (!MU_EditHasFocus(e))
            break;

        switch (ev->key.keysym.sym)
        {
        case SDLK_BACKSPACE:
            MU_EditBackspace(e);
            return true;

        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_ESCAPE:
            MU_EditClearFocus();
            return true;

        default:
            break;
        }
    }
    break;

    default:
        break;
    }

    return false;
}

static void MU_EditBuildDrawText(MU_EditControl* e, char* out, int outSize)
{
    if (!e || !out || outSize <= 0)
        return;

    out[0] = '\0';

    if (e->password)
    {
        int count = e->len;

        if (count > outSize - 1)
            count = outSize - 1;

        for (int i = 0; i < count; ++i)
            out[i] = '*';

        out[count] = '\0';
    }
    else
    {
        MU_CopyLimited(out, outSize, e->text);
    }
}

void MU_EditRender(MU_EditControl* e)
{
    if (!e || !e->visible)
        return;

    MU_EditStyle* s = &e->style;

    /*
        Background.
    */
    MU_FillRect((float)e->x, (float)e->y, (float)e->w, (float)e->h,
                s->bgR, s->bgG, s->bgB, s->bgA);

    /*
        Border.
    */
    if (MU_EditHasFocus(e))
    {
        MU_DrawRectEx((float)e->x, (float)e->y, (float)e->w, (float)e->h,
                      s->borderThickness,
                      s->focusBorderR, s->focusBorderG, s->focusBorderB, s->focusBorderA);
    }
    else
    {
        MU_DrawRectEx((float)e->x, (float)e->y, (float)e->w, (float)e->h,
                      s->borderThickness,
                      s->borderR, s->borderG, s->borderB, s->borderA);
    }

    char drawText[MU_EDIT_TEXT_MAX];
    MU_EditBuildDrawText(e, drawText, MU_EDIT_TEXT_MAX);

    bool hasText = drawText[0] != '\0';

    const char* renderText = hasText ? drawText : e->placeholder;

    int textW = 0;
    int textH = 0;
    MU_MeasureTextEx(e->fontSlot, renderText, &textW, &textH);

    int textX = e->x + s->paddingX;
    int textY = e->y + ((e->h - textH) / 2) + s->paddingY;

    /*
        Draw text or placeholder.
    */
    if (renderText && renderText[0] != '\0')
    {
        if (hasText)
        {
            MU_TextOutEx(e->fontSlot, (float)textX, (float)textY, renderText,
                         s->textR, s->textG, s->textB, s->textA);
        }
        else
        {
            MU_TextOutEx(e->fontSlot, (float)textX, (float)textY, renderText,
                         s->placeholderR, s->placeholderG, s->placeholderB, s->placeholderA);
        }
    }

    /*
        Caret.
    */
    if (MU_EditHasFocus(e))
    {
        Uint64 now = SDL_GetTicks64();

        if (now - e->lastCaretBlink >= e->caretBlinkMs)
        {
            e->showCaret = !e->showCaret;
            e->lastCaretBlink = now;
        }

        if (e->showCaret)
        {
            int caretX = textX;

            if (hasText)
                caretX += MU_GetTextWidthEx(e->fontSlot, drawText);

            caretX += 2;

            int maxCaretX = e->x + e->w - s->paddingX;

            if (caretX > maxCaretX)
                caretX = maxCaretX;

            int caretY = e->y + 5;
            int caretH = e->h - 10;

            if (caretH < 4)
                caretH = e->h;

            MU_FillRect((float)caretX, (float)caretY, 1.0f, (float)caretH,
                        s->caretR, s->caretG, s->caretB, s->caretA);
        }
    }
}
