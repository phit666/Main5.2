#include "stdafx.h"
#include "wzAudio.h"

#ifdef MU_USE_SDL_AUDIO

#include <SDL.h>
#include <SDL_mixer.h>
#include <cstring>
#include "mu_sdl.h"

static Mix_Music* g_wzMusic = nullptr;
static int g_wzVolume = 100;
static char g_wzCurrentFile[512] = {};

int wzAudioCreate(HWND)
{
    if (!gIsMixerInit) {

        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
            return -1;

        Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);


#ifdef _WIN32
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) < 0)
            return E_FAIL;

#else
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 8192) < 0)
            return E_FAIL;
#endif

        gIsMixerInit = true;
    }

    Mix_VolumeMusic(MIX_MAX_VOLUME);
    return 0;
}

void wzAudioDestroy()
{
    if (g_wzMusic)
    {
        Mix_FreeMusic(g_wzMusic);
        g_wzMusic = nullptr;
    }

    Mix_CloseAudio();
    Mix_Quit();
}

void wzAudioPlay(char* szFilename, int numRepeat)
{
    if (!szFilename)
        return;

    if (g_wzMusic)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(g_wzMusic);
        g_wzMusic = nullptr;
    }
    std::string f = MU_NormalizePath(szFilename);
    g_wzMusic = Mix_LoadMUS(f.c_str());
    if (!g_wzMusic)
        return;

    std::strncpy(g_wzCurrentFile, szFilename, sizeof(g_wzCurrentFile) - 1);

    int loops = 0;

    if (numRepeat < 0)
        loops = -1;
    else if (numRepeat > 1)
        loops = numRepeat - 1;
    else
        loops = 0;

    Mix_PlayMusic(g_wzMusic, loops);
}

void wzAudioPause()
{
    Mix_PauseMusic();
}

void wzAudioStop()
{
    Mix_HaltMusic();
}

void wzAudioSetVolume(int numVolume)
{
    if (numVolume < 0) numVolume = 0;
    if (numVolume > 100) numVolume = 100;

    g_wzVolume = numVolume;

    Mix_VolumeMusic((numVolume * MIX_MAX_VOLUME) / 100);
}

int wzAudioGetVolume()
{
    return g_wzVolume;
}

void wzAudioVolumeUp()
{
    wzAudioSetVolume(g_wzVolume + 5);
}

void wzAudioVolumeDown()
{
    wzAudioSetVolume(g_wzVolume - 5);
}

int wzAudioOpenFile(char*)
{
    return 0; // no file dialog on SDL/mobile
}

void wzAudioSeek(int nPosition)
{
    // SDL_mixer seek is seconds-based and format-dependent.
    // Leave as no-op for now.
}

int wzAudioGetStreamOffsetRange()
{
    return 0;
}

int wzAudioGetStreamOffsetSec()
{
    return 0;
}

void wzAudioSetMixerMode(int)
{
}

void wzAudioGetStreamInfo(char* lpszBitrate, char* lpszFrequency)
{
    if (lpszBitrate)
        std::strcpy(lpszBitrate, "0");

    if (lpszFrequency)
        std::strcpy(lpszFrequency, "0");
}

void wzAudioOption(int, int)
{
}

void wzAudioSetEqualizer(const int Slider[MAX_EQ_BANKSLOTS])
{
}

#endif
