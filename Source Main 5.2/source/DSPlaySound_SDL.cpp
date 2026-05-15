#include "stdafx.h"
#include "DSPlaySound.h"

#ifdef MU_USE_SDL_AUDIO

#include <SDL.h>
#include <SDL_mixer.h>
#include <cstring>
#include "WSclient.h"
#include "mu_sdl.h"
#include "mu_file.h"

bool g_EnableSound = false;
bool g_Enable3DSound = false;
int  SoundLoadCount = 0;

static Mix_Chunk* g_SoundChunks[MAX_BUFFER] = {};
static int g_SoundChannels[MAX_BUFFER][MAX_CHANNEL] = {};
static char BufferName[MAX_BUFFER][64] = {};
static int BufferChannel[MAX_BUFFER] = {};
static int MaxBufferChannel[MAX_BUFFER] = {};
static bool Enable3DSound[MAX_BUFFER] = {};

static long g_MasterVolume = MIX_MAX_VOLUME;

static int DSVolumeToMixVolume(long vol)
{
    // DirectSound volume usually: 0 = max, -10000 = silent.
    if (vol >= 0)
        return MIX_MAX_VOLUME;

    if (vol <= -10000)
        return 0;

    return (int)((10000 + vol) * MIX_MAX_VOLUME / 10000);
}


HRESULT InitDirectSound(HWND)
{
    if (g_EnableSound)
        return S_OK;

    if (!gIsMixerInit) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
            return E_FAIL;

        int flags = MIX_INIT_OGG | MIX_INIT_MP3;
        Mix_Init(flags);

#ifdef _WIN32
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) < 0)
            return E_FAIL;

#else
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 8192) < 0)
            return E_FAIL;
#endif
        gIsMixerInit = true;
    }

    Mix_AllocateChannels(16);
    Mix_ReserveChannels(4);

    memset(g_SoundChunks, 0, sizeof(g_SoundChunks));
    memset(g_SoundChannels, -1, sizeof(g_SoundChannels));
    memset(BufferChannel, 0, sizeof(BufferChannel));
    memset(MaxBufferChannel, 0, sizeof(MaxBufferChannel));
    memset(Enable3DSound, 0, sizeof(Enable3DSound));

    g_EnableSound = true;
    return S_OK;
}

void SetEnableSound(bool b)
{
    g_EnableSound = b;
}

void FreeDirectSound()
{
    if (!g_EnableSound)
        return;

    for (int i = 0; i < MAX_BUFFER; ++i)
    {
        if (g_SoundChunks[i])
        {
            Mix_FreeChunk(g_SoundChunks[i]);
            g_SoundChunks[i] = nullptr;
        }
    }

    //Mix_CloseAudio();
    //Mix_Quit();

    g_EnableSound = false;
}

void LoadWaveFile(int Buffer, TCHAR* strFileName, int MaxChannel, bool Enable)
{
    if (!g_EnableSound)
        return;

    if (Buffer < 0 || Buffer >= MAX_BUFFER || !strFileName)
        return;

    if (g_SoundChunks[Buffer])
        return;

#ifdef UNICODE
    char fileName[512];
    WideCharToMultiByte(CP_UTF8, 0, strFileName, -1, fileName, sizeof(fileName), NULL, NULL);
#else
    const char* fileName = strFileName;
#endif

    std::string fixedsoundfile = MU_NormalizePath(fileName);
    Mix_Chunk* chunk = Mix_LoadWAV(fixedsoundfile.c_str());
    if (!chunk)
    {
        g_ErrorReport.Write("SDL_mixer failed to load sound [%s]: %s\r\n", fixedsoundfile.c_str(), Mix_GetError());
        return;
    }

    g_SoundChunks[Buffer] = chunk;
    BufferChannel[Buffer] = 0;
    MaxBufferChannel[Buffer] = MaxChannel;
    Enable3DSound[Buffer] = Enable;

    strncpy(BufferName[Buffer], fileName, sizeof(BufferName[Buffer]) - 1);
    BufferName[Buffer][sizeof(BufferName[Buffer]) - 1] = '\0';

    ++SoundLoadCount;

    SetVolume(Buffer, g_MasterVolume);
}

HRESULT PlayBuffer(int Buffer, OBJECT* Object, BOOL bLooped)
{
    if (!g_EnableSound)
        return E_FAIL;

    if (Buffer < 0 || Buffer >= MAX_BUFFER)
        return E_FAIL;

    if (!g_SoundChunks[Buffer])
        return E_FAIL;

    int maxCh = MaxBufferChannel[Buffer];

    if (maxCh <= 0 || maxCh > MAX_CHANNEL)
        maxCh = MAX_CHANNEL;

    // If this is a looped sound and already playing, do not restart it.
    if (bLooped)
    {
        for (int i = 0; i < maxCh; ++i)
        {
            int ch = g_SoundChannels[Buffer][i];

            if (ch >= 0 && Mix_Playing(ch))
                return S_OK;
        }
    }

    int slot = BufferChannel[Buffer];

    if (slot < 0 || slot >= maxCh)
        slot = 0;

    int oldChannel = g_SoundChannels[Buffer][slot];

    // For one-shot sounds, only reuse dead slots.
    if (oldChannel >= 0 && Mix_Playing(oldChannel))
    {
        if (!bLooped)
        {
            // allow overlap instead of cutting old sound
            oldChannel = -1;
        }
    }

    int channel = Mix_PlayChannel(-1, g_SoundChunks[Buffer], bLooped ? -1 : 0);

    if (channel < 0)
        return E_FAIL;

    g_SoundChannels[Buffer][slot] = channel;

    BufferChannel[Buffer] = slot + 1;

    if (BufferChannel[Buffer] >= maxCh)
        BufferChannel[Buffer] = 0;

    return S_OK;
}

void StopBuffer(int Buffer, BOOL bResetPosition)
{
    if (!g_EnableSound)
        return;

    if (Buffer < 0 || Buffer >= MAX_BUFFER)
        return;

    for (int i = 0; i < MAX_CHANNEL; i++)
    {
        int ch = g_SoundChannels[Buffer][i];

        if (ch >= 0)
        {
            if (bResetPosition)
            {
                // stop completely (rewind)
                Mix_HaltChannel(ch);
                g_SoundChannels[Buffer][i] = -1;
            }
            else
            {
                // pause (keep position)
                Mix_Pause(ch);
            }
        }
    }
}

void AllStopSound(void)
{
    if (!g_EnableSound)
        return;

    Mix_HaltChannel(-1);

    memset(g_SoundChannels, -1, sizeof(g_SoundChannels));
}

HRESULT ReleaseBuffer(int Buffer)
{
    if (!g_EnableSound)
        return E_FAIL;

    if (Buffer < 0 || Buffer >= MAX_BUFFER)
        return E_FAIL;

    StopBuffer(Buffer, TRUE);

    if (g_SoundChunks[Buffer])
    {
        Mix_FreeChunk(g_SoundChunks[Buffer]);
        g_SoundChunks[Buffer] = nullptr;
    }

    MaxBufferChannel[Buffer] = 0;

    if (SoundLoadCount > 0)
        --SoundLoadCount;

    return S_OK;
}

HRESULT RestoreBuffers(int, int)
{
    return S_OK;
}

void SetVolume(int Buffer, long vol)
{
    if (!g_EnableSound)
        return;

    if (Buffer < 0 || Buffer >= MAX_BUFFER)
        return;

    if (!g_SoundChunks[Buffer])
        return;

    int mixVol = DSVolumeToMixVolume(vol);
    Mix_VolumeChunk(g_SoundChunks[Buffer], mixVol);
}

void SetMasterVolume(long vol)
{
    if (!g_EnableSound)
        return;

    g_MasterVolume = vol;

    for (int i = 0; i < MAX_BUFFER; ++i)
        SetVolume(i, vol);
}

void Set3DSoundPosition()
{
    // DirectSound 3D replacement can be approximated later using Mix_SetPosition().
}

#endif