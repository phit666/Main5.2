#pragma once

/*
    mu_wininet_curl_compat.h

    Small WinINet-like compatibility wrapper backed by libcurl.

    Goal:
      Let old code keep using common WinINet names like:
        HINTERNET
        INTERNET_PORT
        InternetOpen / InternetConnectA / HttpOpenRequest
        HttpSendRequest / HttpQueryInfo / InternetReadFile
        FtpFindFirstFileA / FtpOpenFileA / InternetCloseHandle

    Notes:
      - This is NOT a complete WinINet replacement.
      - It is designed for your downloader flow.
      - HTTP/HTTPS/FTP download is supported.
      - Uses libcurl internally.
*/

#include <stdint.h>
#include <stddef.h>

#ifndef _WIN32
    #include <string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------- */
/* Basic Windows-style types                                                  */
/* ------------------------------------------------------------------------- */
#ifndef _WIN32
#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BYTE
typedef unsigned char BYTE;
#endif

#ifndef DWORD
typedef uint32_t DWORD;
#endif

#ifndef DWORD_PTR
typedef uintptr_t DWORD_PTR;
#endif

#ifndef ULONGLONG
typedef uint64_t ULONGLONG;
#endif

#ifndef INTERNET_PORT
typedef unsigned short INTERNET_PORT;
#endif

#ifndef LPVOID
typedef void* LPVOID;
#endif

#ifndef LPCSTR
typedef const char* LPCSTR;
#endif

#ifndef LPSTR
typedef char* LPSTR;
#endif

#ifndef LPDWORD
typedef uint32_t* LPDWORD;
#endif

#ifndef HIBYTE
#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))
#endif

#ifndef LOBYTE
#define LOBYTE(w) ((BYTE)((WORD)(w) & 0xFF))
#endif

#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#endif

#endif
typedef void* HINTERNET;

typedef unsigned short int INTERNET_PORT;

#define INTERNET_MAX_URL_LENGTH 512
#define INTERNET_MAX_USER_NAME_LENGTH 20
#define INTERNET_MAX_PASSWORD_LENGTH 20


/* ------------------------------------------------------------------------- */
/* WinINet-like constants                                                     */
/* ------------------------------------------------------------------------- */

#ifndef INTERNET_OPEN_TYPE_PRECONFIG
#define INTERNET_OPEN_TYPE_PRECONFIG 0
#endif

#ifndef INTERNET_SERVICE_FTP
#define INTERNET_SERVICE_FTP 1
#endif

#ifndef INTERNET_SERVICE_HTTP
#define INTERNET_SERVICE_HTTP 3
#endif

#ifndef INTERNET_FLAG_RELOAD
#define INTERNET_FLAG_RELOAD 0x80000000
#endif

#ifndef INTERNET_FLAG_PASSIVE
#define INTERNET_FLAG_PASSIVE 0x08000000
#endif

#ifndef INTERNET_FLAG_TRANSFER_BINARY
#define INTERNET_FLAG_TRANSFER_BINARY 0x00000002
#endif

#ifndef GENERIC_READ
#define GENERIC_READ 0x80000000
#endif

#ifndef FTP_TRANSFER_TYPE_BINARY
#define FTP_TRANSFER_TYPE_BINARY 0x00000002
#endif

#ifndef HTTP_STATUS_OK
#define HTTP_STATUS_OK 200
#endif

#ifndef HTTP_QUERY_STATUS_CODE
#define HTTP_QUERY_STATUS_CODE 19
#endif

#ifndef HTTP_QUERY_CONTENT_LENGTH
#define HTTP_QUERY_CONTENT_LENGTH 5
#endif

/* ------------------------------------------------------------------------- */
/* Minimal WIN32_FIND_DATAA replacement used by your FTP connecter             */
/* ------------------------------------------------------------------------- */

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _WIN32
typedef struct _WIN32_FIND_DATAA
{
    DWORD dwFileAttributes;
    DWORD ftCreationTime_dwLowDateTime;
    DWORD ftCreationTime_dwHighDateTime;
    DWORD ftLastAccessTime_dwLowDateTime;
    DWORD ftLastAccessTime_dwHighDateTime;
    DWORD ftLastWriteTime_dwLowDateTime;
    DWORD ftLastWriteTime_dwHighDateTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    char  cFileName[MAX_PATH];
    char  cAlternateFileName[14];
} WIN32_FIND_DATAA;
#endif
/* ------------------------------------------------------------------------- */
/* Functions                                                                  */
/* ------------------------------------------------------------------------- */

HINTERNET InternetOpenA(
    LPCSTR lpszAgent,
    DWORD dwAccessType,
    LPCSTR lpszProxy,
    LPCSTR lpszProxyBypass,
    DWORD dwFlags
);

#ifndef InternetOpen
#define InternetOpen InternetOpenA
#endif

HINTERNET InternetConnectA(
    HINTERNET hInternet,
    LPCSTR lpszServerName,
    INTERNET_PORT nServerPort,
    LPCSTR lpszUserName,
    LPCSTR lpszPassword,
    DWORD dwService,
    DWORD dwFlags,
    DWORD_PTR dwContext
);

HINTERNET HttpOpenRequestA(
    HINTERNET hConnect,
    LPCSTR lpszVerb,
    LPCSTR lpszObjectName,
    LPCSTR lpszVersion,
    LPCSTR lpszReferrer,
    LPCSTR* lplpszAcceptTypes,
    DWORD dwFlags,
    DWORD_PTR dwContext
);

#ifndef HttpOpenRequest
#define HttpOpenRequest HttpOpenRequestA
#endif

BOOL HttpSendRequestA(
    HINTERNET hRequest,
    LPCSTR lpszHeaders,
    DWORD dwHeadersLength,
    LPVOID lpOptional,
    DWORD dwOptionalLength
);

#ifndef HttpSendRequest
#define HttpSendRequest HttpSendRequestA
#endif

BOOL HttpQueryInfoA(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD lpdwIndex
);

#ifndef HttpQueryInfo
#define HttpQueryInfo HttpQueryInfoA
#endif

HINTERNET FtpFindFirstFileA(
    HINTERNET hConnect,
    LPCSTR lpszSearchFile,
    WIN32_FIND_DATAA* lpFindFileData,
    DWORD dwFlags,
    DWORD_PTR dwContext
);

HINTERNET FtpOpenFileA(
    HINTERNET hConnect,
    LPCSTR lpszFileName,
    DWORD dwAccess,
    DWORD dwFlags,
    DWORD_PTR dwContext
);

BOOL InternetReadFile(
    HINTERNET hFile,
    LPVOID lpBuffer,
    DWORD dwNumberOfBytesToRead,
    LPDWORD lpdwNumberOfBytesRead
);

BOOL InternetQueryDataAvailable(
    HINTERNET hFile,
    LPDWORD lpdwNumberOfBytesAvailable,
    DWORD dwFlags,
    DWORD_PTR dwContext
);

BOOL InternetCloseHandle(HINTERNET hInternet);

#ifndef _WIN32
DWORD GetLastError(void);
#endif
#ifdef __cplusplus
}
#endif
