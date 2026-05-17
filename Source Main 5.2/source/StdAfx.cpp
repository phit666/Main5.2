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

static BYTE bBuxCode2[3] = { 0x9c, 0xef, 0xb8 };

static void BuxConvert2(BYTE* Buffer, int Size)
{
    for (int i = 0; i < Size; i++)
        Buffer[i] ^= bBuxCode2[i % 3];
}

#pragma pack(push, 1)
struct QuickLoginData
{
    char username[11];
    char password[11];
};
#pragma pack(pop)

std::string GetQuickLoginPath()
{
#ifdef __ANDROID__
    const char* base = SDL_AndroidGetInternalStoragePath();
    return std::string(base) + "/ql.dat";
#else
    return "data/ql.dat";
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

    MU_FILE* fp = MU_fopen(path.c_str(), "wb");
    if (!fp)
        return false;

    MU_fwrite(&data, 1, sizeof(data), fp);
    MU_fclose(fp);

    return true;
}

bool LoadQuickLogin(char* usernameOut, int usernameOutSize,
    char* passwordOut, int passwordOutSize)
{
    std::string path = GetQuickLoginPath();

    MU_FILE* fp = MU_fopen(path.c_str(), "rb");
    if (!fp)
        return false;

    QuickLoginData data;
    size_t readSize = MU_fread(&data, 1, sizeof(data), fp);
    MU_fclose(fp);

    if (readSize != sizeof(data))
        return false;

    BuxConvert2((BYTE*)&data, sizeof(data));

    data.username[sizeof(data.username) - 1] = '\0';
    data.password[sizeof(data.password) - 1] = '\0';

    strncpy(usernameOut, data.username, usernameOutSize - 1);
    strncpy(passwordOut, data.password, passwordOutSize - 1);

    usernameOut[usernameOutSize - 1] = '\0';
    passwordOut[passwordOutSize - 1] = '\0';

    return true;
}