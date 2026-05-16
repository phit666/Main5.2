// stdafx.cpp : source file that includes just the standard includes
//	Online.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <SDL.h>
#include <SDL_system.h>
#include <stdio.h>
#include <string>
#include <vector>
#include "wt.h"

typedef unsigned char BYTE;

static BYTE bBuxCode[3] = { 0x9c, 0xef, 0xb8 };

static void BuxConvert2(BYTE* Buffer, int Size)
{
    for (int i = 0; i < Size; i++)
        Buffer[i] ^= bBuxCode[i % 3];
}

struct QuickLoginData
{
    char username[64];
    char password[64];
};

std::string GetQuickLoginPath()
{
#ifdef __ANDROID__
    const char* base = SDL_AndroidGetInternalStoragePath();
    return std::string(base) + "/quicklogin.dat";
#else
    return "quicklogin.dat";
#endif
}

bool SaveQuickLogin(const char* username, const char* password)
{
    QuickLoginData data;
    memset(&data, 0, sizeof(data));

    strncpy(data.username, username, sizeof(data.username) - 1);
    strncpy(data.password, password, sizeof(data.password) - 1);

    BuxConvert2((BYTE*)&data, sizeof(data));

    std::string path = GetQuickLoginPath();

    FILE* fp = fopen(path.c_str(), "wb");
    if (!fp)
        return false;

    fwrite(&data, 1, sizeof(data), fp);
    fclose(fp);

    return true;
}

bool LoadQuickLogin(char* usernameOut, int usernameOutSize,
    char* passwordOut, int passwordOutSize)
{
    std::string path = GetQuickLoginPath();

    FILE* fp = fopen(path.c_str(), "rb");
    if (!fp)
        return false;

    QuickLoginData data;
    size_t readSize = fread(&data, 1, sizeof(data), fp);
    fclose(fp);

    if (readSize != sizeof(data))
        return false;

    BuxConvert((BYTE*)&data, sizeof(data));

    data.username[sizeof(data.username) - 1] = '\0';
    data.password[sizeof(data.password) - 1] = '\0';

    strncpy(usernameOut, data.username, usernameOutSize - 1);
    strncpy(passwordOut, data.password, passwordOutSize - 1);

    usernameOut[usernameOutSize - 1] = '\0';
    passwordOut[passwordOutSize - 1] = '\0';

    return true;
}