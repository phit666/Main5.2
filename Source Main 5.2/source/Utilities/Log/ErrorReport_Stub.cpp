#include "stdafx.h"
#include "ErrorReport.h"

CErrorReport g_ErrorReport;

#ifdef __ANDROID__

CErrorReport::CErrorReport() {}
CErrorReport::~CErrorReport() {}

void CErrorReport::Clear() {}
void CErrorReport::Create(char*) {}
void CErrorReport::Destroy() {}
void CErrorReport::CutHead() {}

char* CErrorReport::CheckHeadToCut(char* buffer, DWORD)
{
    return buffer;
}

BOOL CErrorReport::WriteFile(HANDLE, void*, DWORD, LPDWORD, LPOVERLAPPED)
{
    return TRUE;
}

void CErrorReport::WriteDebugInfoStr(char*) {}

void CErrorReport::Write(const char*, ...)
{
}

void CErrorReport::HexWrite(void*, int) {}
void CErrorReport::AddSeparator() {}
void CErrorReport::WriteLogBegin() {}
void CErrorReport::WriteCurrentTime(BOOL) {}
void CErrorReport::WriteSystemInfo(ER_SystemInfo*) {}
void CErrorReport::WriteOpenGLInfo() {}
void CErrorReport::WriteImeInfo(HWND) {}
void CErrorReport::WriteSoundCardInfo() {}

void GetSystemInfo(ER_SystemInfo* si)
{
    if (!si) return;

    memset(si, 0, sizeof(ER_SystemInfo));
    strcpy(si->m_lpszCPU, "Android");
    strcpy(si->m_lpszOS, "Android");
    strcpy(si->m_lpszDxVersion, "OpenGL ES");
}

#endif