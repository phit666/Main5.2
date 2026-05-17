// MultiLanguage.cpp: implementation of the CMultiLanguage class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MultiLanguage.h"

#ifndef _WIN32
#include <cstdlib>
#include <string.h>
#include <string.h>
#include "wt.h"
#endif
#include "w_nuklear.h"
#include "mu_sdl.h"
#include "MU_UIRenderer.h"

CMultiLanguage* CMultiLanguage::ms_Singleton = NULL;

#ifdef MU_USE_SDL
#include <cwchar>
#include <cstdlib>
#include <cstdint>

int MU_MultiByteToWideChar(
		uint32_t CodePage,
		uint32_t dwFlags,
		const char* lpMBStr,
		int cbMB,
		wchar_t* lpWCStr,
		int cchWC)
{
	(void)CodePage;
	(void)dwFlags;

	if (!lpMBStr)
		return 0;

	// Windows behavior:
	// cbMB == -1 means null-terminated input.
	if (cbMB == -1)
	{
		size_t need = mbstowcs(nullptr, lpMBStr, 0);

		if (need == (size_t)-1)
			return 0;

		need += 1; // include null terminator

		if (!lpWCStr || cchWC == 0)
			return (int)need;

		size_t written = mbstowcs(lpWCStr, lpMBStr, cchWC);

		if (written == (size_t)-1)
			return 0;

		if ((int)written < cchWC)
			lpWCStr[written] = L'\0';

		return (int)written;
	}

	// Explicit byte count
	std::string temp(lpMBStr, lpMBStr + cbMB);

	size_t need = mbstowcs(nullptr, temp.c_str(), 0);

	if (need == (size_t)-1)
		return 0;

	if (!lpWCStr || cchWC == 0)
		return (int)need;

	size_t written = mbstowcs(lpWCStr, temp.c_str(), cchWC);

	if (written == (size_t)-1)
		return 0;

	if ((int)written < cchWC)
		lpWCStr[written] = L'\0';

	return (int)written;
}

int MU_WideCharToMultiByte(
		uint32_t CodePage,
		uint32_t dwFlags,
		const wchar_t* lpWCStr,
		int cchWC,
		char* lpMBStr,
		int cbMB,
		const char* lpDefChar,
		bool* lpUsedDefChar)
{
	(void)CodePage;
	(void)dwFlags;
	(void)lpDefChar;

	if (lpUsedDefChar)
		*lpUsedDefChar = false;

	if (!lpWCStr)
		return 0;

	// Windows behavior: -1 means include null terminator.
	if (cchWC == -1)
	{
		size_t need = wcstombs(nullptr, lpWCStr, 0);
		if (need == (size_t)-1)
			return 0;

		need += 1;

		if (!lpMBStr || cbMB == 0)
			return (int)need;

		size_t written = wcstombs(lpMBStr, lpWCStr, cbMB);
		if (written == (size_t)-1)
			return 0;

		if ((int)written < cbMB)
			lpMBStr[written] = '\0';

		return (int)written;
	}

	// cchWC is explicit character count.
	std::wstring temp(lpWCStr, lpWCStr + cchWC);

	size_t need = wcstombs(nullptr, temp.c_str(), 0);
	if (need == (size_t)-1)
		return 0;

	if (!lpMBStr || cbMB == 0)
		return (int)need;

	size_t written = wcstombs(lpMBStr, temp.c_str(), cbMB);
	if (written == (size_t)-1)
		return 0;

	if ((int)written < cbMB)
		lpMBStr[written] = '\0';

	return (int)written;
}
#else
#define MU_MultiByteToWideChar MultiByteToWideChar
#define MU_WideCharToMultiByte WideCharToMultiByte
#endif


CMultiLanguage::CMultiLanguage(std::string strSelectedML)
{
	ms_Singleton = this;

	if (_stricmp(strSelectedML.c_str(), "ENG") == 0)
	{
		byLanguage = 0;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	else if(_stricmp(strSelectedML.c_str(), "POR") == 0)
	{	
		byLanguage = 1;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	else if(_stricmp(strSelectedML.c_str(), "SPN") == 0)
	{	
		byLanguage = 2;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
	else
	{
		byLanguage = 0;
		iCodePage = 1252;
		iNumByteForOneCharUTF8 = 2;
	}
}

BYTE CMultiLanguage::GetLanguage()
{
	return byLanguage;
}

int CMultiLanguage::GetCodePage()
{
	return iCodePage;
}

int CMultiLanguage::GetNumByteForOneCharUTF8()
{
	return iNumByteForOneCharUTF8;
}

BOOL CMultiLanguage::IsCharUTF8(const char* pszText)
{
    if (pszText == NULL || strlen(pszText) <= 0)
        return TRUE;
    
    const char* pbyCurr = pszText;
	BOOL        bUTF8   = TRUE;
    
    while ( (*pbyCurr != 0x00) && bUTF8)
    {
        // U+0000 ~ U+007F (0 ~ 127)
        if ((*pbyCurr & 0x80) == 0x00)
        {
            pbyCurr++;
        }
        // U+0080 ~ U+07FF (128 ~ 2,047)
        else if (((*pbyCurr & 0xE0) == 0xC0) && ((*(pbyCurr+1) & 0xC0) == 0x80))
        {
            pbyCurr += 2;
        }
        // U+0800 ~ U+FFFF (2,048 ~ 65,535)
        else if (((*pbyCurr & 0xF0) == 0xE0) && ((*(pbyCurr+1) & 0xC0) == 0x80) && ((*(pbyCurr+2) & 0xC0) == 0x80))
        {
            pbyCurr += 3;
        }
        // U+10000 ~ U+10FFFF (65,536 ~ 1,114,111)
        else if (((*pbyCurr & 0xF8) == 0xF0) && 
            ((*(pbyCurr+1) & 0xC0) == 0x80) && ((*(pbyCurr+2) & 0xC0) == 0x80) && ((*(pbyCurr+3) & 0xC0) == 0x80))
        {
            pbyCurr += 4;
        }
        // not UTF-8
        else
        {
            bUTF8 = FALSE;
        }
    }
    return bUTF8;
}

int CMultiLanguage::ConvertCharToWideStr(std::wstring& wstrDest, LPCSTR lpString)
{
    wstrDest = L"";

    if (lpString == NULL || strlen(lpString) <= 0)
        return 0;

    int nLenOfWideCharStr;
	int iConversionType;

	iConversionType = (IsCharUTF8(lpString)) ? CP_UTF8 : iCodePage;

    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = MU_MultiByteToWideChar(iConversionType, 0, lpString, -1, NULL, 0);
    // memory allocation
    wchar_t* pwszStr = new wchar_t[nLenOfWideCharStr];
    
    // convert the multi-byte string to a wide-character string.
	MU_MultiByteToWideChar(iConversionType, 0, lpString, -1, pwszStr, nLenOfWideCharStr);
    
    //assign
    wstrDest += pwszStr;
    
    // release the allocated memory.
    delete[] pwszStr;
    
    return nLenOfWideCharStr-1;     
}

int CMultiLanguage::ConvertWideCharToStr(std::string& strDest, LPCWSTR lpwString, int iConversionType)
{
    strDest = "";

    if (lpwString == NULL || wcslen(lpwString) <= 0)
        return 0;

    int nLenOfWideCharStr;
    
    // calculate the number of characters needed to hold the wide-character version of the string.
    nLenOfWideCharStr = MU_WideCharToMultiByte(iConversionType, 0, lpwString, -1, NULL, 0, 0, 0);
    // memory allocation
    char* pszStr = new char[nLenOfWideCharStr];
    
    // convert the multi-byte string to a wide-character string.
	MU_WideCharToMultiByte(iConversionType, 0, lpwString, -1, pszStr, nLenOfWideCharStr, 0, 0);

    //assign
    strDest += pszStr;
    
    // release the allocated memory.
    delete[] pszStr;

    return nLenOfWideCharStr-1;
}

void CMultiLanguage::ConvertANSIToUTF8OrViceVersa(std::string& strDest, LPCSTR lpString)
{
	std::wstring wstrUTF16 = L"";
	int iDestType = (IsCharUTF8(lpString)) ? CP_ACP : CP_UTF8;

	ConvertCharToWideStr(wstrUTF16, lpString);
	ConvertWideCharToStr(strDest, wstrUTF16.c_str(), iDestType);
}

int	CMultiLanguage::GetClosestBlankFromCenter(const std::wstring wstrTarget)
{
	int iLength = wstrTarget.length();
	
	std::wstring wstrText1 = wstrTarget.substr(iLength/2, std::wstring::npos);
	std::wstring wstrText2 = wstrTarget.substr(0, iLength/2);
	
	int iPosLastBlankFromFirstHalf = wstrText2.find_last_of(L" ");
	int iPosFirstBlankFromSecondHalf = wstrText1.find_first_of(L" ", 1);
	int iClosestBlankFromCenter = 0;
	
	if (iPosLastBlankFromFirstHalf == std::wstring::npos && iPosFirstBlankFromSecondHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iLength/2;
	}
	else if (iPosLastBlankFromFirstHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf+iLength/2;
	}
	else if (iPosFirstBlankFromSecondHalf == std::wstring::npos)
	{
		iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
	}
	else if (iPosFirstBlankFromSecondHalf <= iLength/2-iPosLastBlankFromFirstHalf)
	{
		iClosestBlankFromCenter = iPosFirstBlankFromSecondHalf+iLength/2;
	}
	else
	{
		iClosestBlankFromCenter = iPosLastBlankFromFirstHalf;
	}
	
	return iClosestBlankFromCenter;
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCWSTR lpString, int cbString, LPSIZE lpSize)
{
	if (!lpString || !lpSize) return false;

	std::string ss;
	this->ConvertWideCharToStr(ss, lpString);

	MU_TextSize sz =
		MU_GetTextSizeEx(g_hFont, ss.c_str());
	lpSize->cx = sz.w;
	lpSize->cy = sz.h;

	return true;
}

BOOL CMultiLanguage::_GetTextExtentPoint32(HDC hdc, LPCSTR lpString, int cbString, LPSIZE lpSize)
{
	if (!lpString || !lpSize) return false;

	MU_TextSize sz =
		MU_GetTextSizeEx(g_hFont, lpString);

	lpSize->cx = sz.w;
	lpSize->cy = sz.h;

	return true;
}

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCWSTR lpString, int cbString)
{
	std::string ss;
	this->ConvertWideCharToStr(ss, lpString);
	return _TextOut(hdc, nXStart, nYStart, ss.c_str(), cbString);
}

static uint32_t canvasctr = 1;

BOOL CMultiLanguage::_TextOut(HDC hdc, int nXStart, int nYStart, LPCSTR lpString, int cbString)
{
	/*
	std::string canvastitle = std::to_string(canvasctr++); if (canvasctr >= 2000000000)canvasctr = 1;

	if (nk_begin(g_nk_ctx, canvastitle.c_str(), nk_rect(0, 0, WindowWidth, WindowHeight), NK_WINDOW_NO_SCROLLBAR)) {

		struct nk_command_buffer *canvas = nk_window_get_canvas(g_nk_ctx);

		//struct nk_user_font *font = g_nk_ctx->style.font;

		struct nk_rect text_rect = nk_rect(nXStart, nYStart, 200, 20);

		nk_draw_text(canvas, text_rect, lpString, 5, g_nk_ctx->style.font,
					 nk_rgb(255,255,255), nk_rgba(0,0,0,0));
	}

	nk_end(g_nk_ctx);
	*/
	return 1;
}
	
WPARAM CMultiLanguage::ConvertFulltoHalfWidthChar(DWORD wParam)
{
	wchar_t Char = (wchar_t)(wParam);

	if (Char >= 0xFF01 && Char <= 0xFF5A) 
		wParam -= 0xFEE0;
	else if (Char == 0x3000)
		wParam = 0x0020;

	return wParam;
}