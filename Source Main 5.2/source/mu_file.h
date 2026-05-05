#pragma once

#ifdef MU_USE_SDL
#include <SDL.h>
#include <string>

inline std::string MU_NormalizePath(const char* path)
{
    std::string p = path ? path : "";

    for (char& c : p)
    {
        if (c == '\\')
            c = '/';
    }

    return p;
}

using MU_FILE = SDL_RWops;

inline MU_FILE* MU_fopen(const char* path, const char* mode)
{
    std::string fixed = MU_NormalizePath(path);
    return SDL_RWFromFile(fixed.c_str(), mode);
}

inline int MU_fclose(MU_FILE* fp)
{
    return fp ? SDL_RWclose(fp) : 0;
}

inline size_t MU_fread(void* ptr, size_t size, size_t count, MU_FILE* fp)
{
    if (!fp)
        return 0;

    return SDL_RWread(fp, ptr, size, count);
}

inline size_t MU_fwrite(const void* ptr, size_t size, size_t count, MU_FILE* fp)
{
    if (!fp)
        return 0;

    return SDL_RWwrite(fp, ptr, size, count);
}

inline int MU_fseek(MU_FILE* fp, long offset, int origin)
{
    if (!fp)
        return -1;

    return SDL_RWseek(fp, offset, origin) < 0 ? -1 : 0;
}

inline long MU_ftell(MU_FILE* fp)
{
    if (!fp)
        return -1;

    return (long)SDL_RWtell(fp);
}

#else

#include <cstdio>

using MU_FILE = FILE;

#define MU_fopen  fopen
#define MU_fclose fclose
#define MU_fread  fread
#define MU_fseek  fseek
#define MU_ftell  ftell
#define MU_fwrite fwrite

#endif