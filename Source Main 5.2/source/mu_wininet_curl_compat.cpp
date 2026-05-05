#include "stdafx.h"
#include "mu_wininet_curl_compat.h"

#ifdef _WIN32
#define CURL_STATICLIB
#endif
#include <curl/curl.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

/*
    This file intentionally mimics only the WinINet calls used by your old
    HTTPConnecter.cpp / FTPConnecter.cpp downloader flow.
*/

namespace
{
    enum class HandleType
    {
        Session,
        Connection,
        Request,
        Find
    };

    struct CurlInternetHandle
    {
        HandleType type = HandleType::Session;

        CURL* curl = nullptr;
        bool ownsCurl = false;

        std::string agent;
        std::string scheme;
        std::string server;
        INTERNET_PORT port = 0;
        std::string user;
        std::string password;
        std::string userpwd;
        std::string baseUrl;
        std::string fullUrl;
        std::string objectName;
        DWORD service = 0;
        DWORD flags = 0;

        bool requestPrepared = false;
        bool requestPerformed = false;
        bool headerOnly = false;
        bool passiveFtp = false;

        long responseCode = 0;
        curl_off_t contentLength = -1;
        CURLcode lastCurlCode = CURLE_OK;

        std::vector<unsigned char> data;
        size_t readOffset = 0;
    };

    static std::mutex g_curlMutex;
    static int g_curlUsers = 0;
    static DWORD g_lastError = 0;

    static void SetCompatError(DWORD err)
    {
        g_lastError = err;
    }

    static CurlInternetHandle* AsHandle(HINTERNET h)
    {
        return reinterpret_cast<CurlInternetHandle*>(h);
    }

    static std::string SafeString(const char* s)
    {
        return s ? std::string(s) : std::string();
    }

    static bool StartsWithScheme(const std::string& s)
    {
        return s.rfind("http://", 0) == 0 ||
               s.rfind("https://", 0) == 0 ||
               s.rfind("ftp://", 0) == 0 ||
               s.rfind("ftps://", 0) == 0;
    }

    static std::string TrimLeadingSlash(std::string s)
    {
        while (!s.empty() && (s[0] == '/' || s[0] == '\\'))
            s.erase(s.begin());
        return s;
    }

    static std::string MakeBaseUrl(const CurlInternetHandle* h)
    {
        std::string server = h->server;

        if (StartsWithScheme(server))
            return server;

        std::string url = h->scheme;
        url += "://";
        url += server;

        if (h->port != 0)
        {
            bool defaultHttp  = (h->scheme == "http"  && h->port == 80);
            bool defaultHttps = (h->scheme == "https" && h->port == 443);
            bool defaultFtp   = (h->scheme == "ftp"   && h->port == 21);

            if (!defaultHttp && !defaultHttps && !defaultFtp)
            {
                url += ":";
                url += std::to_string(h->port);
            }
        }

        return url;
    }

    static std::string MakeFullUrl(const CurlInternetHandle* conn, const char* objectName)
    {
        std::string obj = TrimLeadingSlash(SafeString(objectName));

        if (StartsWithScheme(obj))
            return obj;

        std::string url = conn->baseUrl;

        if (!obj.empty())
        {
            if (!url.empty() && url.back() != '/')
                url += "/";
            url += obj;
        }

        return url;
    }

    static size_t WriteToVectorCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        CurlInternetHandle* h = static_cast<CurlInternetHandle*>(userdata);
        const size_t total = size * nmemb;

        if (!h || !ptr || total == 0)
            return 0;

        const unsigned char* begin = reinterpret_cast<const unsigned char*>(ptr);
        h->data.insert(h->data.end(), begin, begin + total);
        return total;
    }

    static bool PrepareCurl(CurlInternetHandle* h, bool headerOnly)
    {
        if (!h || !h->curl)
        {
            SetCompatError(1);
            return false;
        }

        h->data.clear();
        h->readOffset = 0;
        h->responseCode = 0;
        h->contentLength = -1;
        h->lastCurlCode = CURLE_OK;
        h->headerOnly = headerOnly;

        curl_easy_reset(h->curl);

        curl_easy_setopt(h->curl, CURLOPT_URL, h->fullUrl.c_str());
        curl_easy_setopt(h->curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(h->curl, CURLOPT_CONNECTTIMEOUT, 30L);
        curl_easy_setopt(h->curl, CURLOPT_USERAGENT, h->agent.empty() ? "mu-curl-wininet-compat" : h->agent.c_str());

        if (!h->userpwd.empty())
            curl_easy_setopt(h->curl, CURLOPT_USERPWD, h->userpwd.c_str());

        if (h->passiveFtp)
        {
            /* libcurl FTP is passive by default. EPSV enabled usually helps NAT. */
            curl_easy_setopt(h->curl, CURLOPT_FTP_USE_EPSV, 1L);
        }

        if (headerOnly)
        {
            curl_easy_setopt(h->curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(h->curl, CURLOPT_HEADER, 0L);
        }
        else
        {
            curl_easy_setopt(h->curl, CURLOPT_NOBODY, 0L);
            curl_easy_setopt(h->curl, CURLOPT_WRITEFUNCTION, WriteToVectorCallback);
            curl_easy_setopt(h->curl, CURLOPT_WRITEDATA, h);
        }

        h->requestPrepared = true;
        return true;
    }

    static bool PerformRequest(CurlInternetHandle* h, bool headerOnly)
    {
        if (!PrepareCurl(h, headerOnly))
            return false;

        CURLcode res = curl_easy_perform(h->curl);
        h->lastCurlCode = res;
        h->requestPerformed = true;

        if (res != CURLE_OK)
        {
            SetCompatError(static_cast<DWORD>(res));
            return false;
        }

        curl_easy_getinfo(h->curl, CURLINFO_RESPONSE_CODE, &h->responseCode);

#if LIBCURL_VERSION_NUM >= 0x073700
        curl_easy_getinfo(h->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &h->contentLength);
#else
        double len = -1.0;
        curl_easy_getinfo(h->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &len);
        h->contentLength = static_cast<curl_off_t>(len);
#endif

        SetCompatError(0);
        return true;
    }

    static HINTERNET MakeRequestFromConnection(CurlInternetHandle* conn, const char* objectName)
    {
        if (!conn)
        {
            SetCompatError(1);
            return nullptr;
        }

        CurlInternetHandle* req = new CurlInternetHandle();
        req->type = HandleType::Request;
        req->curl = curl_easy_init();
        req->ownsCurl = true;

        if (!req->curl)
        {
            delete req;
            SetCompatError(2);
            return nullptr;
        }

        req->agent = conn->agent;
        req->scheme = conn->scheme;
        req->server = conn->server;
        req->port = conn->port;
        req->user = conn->user;
        req->password = conn->password;
        req->userpwd = conn->userpwd;
        req->baseUrl = conn->baseUrl;
        req->objectName = SafeString(objectName);
        req->fullUrl = MakeFullUrl(conn, objectName);
        req->service = conn->service;
        req->flags = conn->flags;
        req->passiveFtp = conn->passiveFtp;

        SetCompatError(0);
        return reinterpret_cast<HINTERNET>(req);
    }
}

extern "C"
{

HINTERNET InternetOpenA(
    LPCSTR lpszAgent,
    DWORD,
    LPCSTR,
    LPCSTR,
    DWORD)
{
    std::lock_guard<std::mutex> lock(g_curlMutex);

    if (g_curlUsers == 0)
        curl_global_init(CURL_GLOBAL_DEFAULT);

    ++g_curlUsers;

    CurlInternetHandle* h = new CurlInternetHandle();
    h->type = HandleType::Session;
    h->agent = SafeString(lpszAgent);
    h->curl = nullptr;
    h->ownsCurl = false;

    SetCompatError(0);
    return reinterpret_cast<HINTERNET>(h);
}

HINTERNET InternetConnectA(
    HINTERNET hInternet,
    LPCSTR lpszServerName,
    INTERNET_PORT nServerPort,
    LPCSTR lpszUserName,
    LPCSTR lpszPassword,
    DWORD dwService,
    DWORD dwFlags,
    DWORD_PTR)
{
    CurlInternetHandle* session = AsHandle(hInternet);

    if (!session || session->type != HandleType::Session)
    {
        SetCompatError(1);
        return nullptr;
    }

    CurlInternetHandle* conn = new CurlInternetHandle();
    conn->type = HandleType::Connection;
    conn->agent = session->agent;
    conn->server = SafeString(lpszServerName);
    conn->port = nServerPort;
    conn->user = SafeString(lpszUserName);
    conn->password = SafeString(lpszPassword);
    conn->service = dwService;
    conn->flags = dwFlags;
    conn->passiveFtp = (dwFlags & INTERNET_FLAG_PASSIVE) != 0;

    if (dwService == INTERNET_SERVICE_FTP)
        conn->scheme = "ftp";
    else
        conn->scheme = "http";

    if (!conn->user.empty())
        conn->userpwd = conn->user + ":" + conn->password;

    conn->baseUrl = MakeBaseUrl(conn);

    SetCompatError(0);
    return reinterpret_cast<HINTERNET>(conn);
}

HINTERNET HttpOpenRequestA(
    HINTERNET hConnect,
    LPCSTR,
    LPCSTR lpszObjectName,
    LPCSTR,
    LPCSTR,
    LPCSTR*,
    DWORD dwFlags,
    DWORD_PTR)
{
    CurlInternetHandle* conn = AsHandle(hConnect);
    HINTERNET h = MakeRequestFromConnection(conn, lpszObjectName);

    CurlInternetHandle* req = AsHandle(h);
    if (req)
        req->flags = dwFlags;

    return h;
}

BOOL HttpSendRequestA(
    HINTERNET hRequest,
    LPCSTR,
    DWORD,
    LPVOID,
    DWORD)
{
    CurlInternetHandle* req = AsHandle(hRequest);

    if (!req || req->type != HandleType::Request)
    {
        SetCompatError(1);
        return FALSE;
    }

    /* Your old code calls HttpSendRequest before HttpQueryInfo. Use HEAD here
       to get status/content-length without downloading the body yet. */
    return PerformRequest(req, true) ? TRUE : FALSE;
}

BOOL HttpQueryInfoA(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD)
{
    CurlInternetHandle* req = AsHandle(hRequest);

    if (!req || !lpBuffer || !lpdwBufferLength)
    {
        SetCompatError(1);
        return FALSE;
    }

    if (!req->requestPerformed)
    {
        if (!PerformRequest(req, true))
            return FALSE;
    }

    std::string value;

    if (dwInfoLevel == HTTP_QUERY_STATUS_CODE)
    {
        value = std::to_string(req->responseCode);
    }
    else if (dwInfoLevel == HTTP_QUERY_CONTENT_LENGTH)
    {
        if (req->contentLength < 0)
        {
            SetCompatError(3);
            return FALSE;
        }

        value = std::to_string(static_cast<unsigned long long>(req->contentLength));
    }
    else
    {
        SetCompatError(4);
        return FALSE;
    }

    DWORD required = static_cast<DWORD>(value.size());

    if (*lpdwBufferLength <= required)
    {
        *lpdwBufferLength = required + 1;
        SetCompatError(5);
        return FALSE;
    }

    std::memcpy(lpBuffer, value.c_str(), value.size() + 1);
    *lpdwBufferLength = required;

    SetCompatError(0);
    return TRUE;
}

HINTERNET FtpFindFirstFileA(
    HINTERNET hConnect,
    LPCSTR lpszSearchFile,
    WIN32_FIND_DATAA* lpFindFileData,
    DWORD,
    DWORD_PTR)
{
    CurlInternetHandle* conn = AsHandle(hConnect);
    HINTERNET h = MakeRequestFromConnection(conn, lpszSearchFile);
    CurlInternetHandle* req = AsHandle(h);

    if (!req)
        return nullptr;

    if (!PerformRequest(req, true))
    {
        InternetCloseHandle(h);
        return nullptr;
    }

    if (lpFindFileData)
    {
        std::memset(lpFindFileData, 0, sizeof(WIN32_FIND_DATAA));

        ULONGLONG len = req->contentLength >= 0 ? static_cast<ULONGLONG>(req->contentLength) : 0;
        lpFindFileData->nFileSizeHigh = static_cast<DWORD>((len >> 32) & 0xFFFFFFFFu);
        lpFindFileData->nFileSizeLow  = static_cast<DWORD>(len & 0xFFFFFFFFu);

        std::string name = SafeString(lpszSearchFile);
        size_t pos = name.find_last_of("/\\");
        if (pos != std::string::npos)
            name = name.substr(pos + 1);

        std::strncpy(lpFindFileData->cFileName, name.c_str(), MAX_PATH - 1);
    }

    SetCompatError(0);
    return h;
}

HINTERNET FtpOpenFileA(
    HINTERNET hConnect,
    LPCSTR lpszFileName,
    DWORD,
    DWORD dwFlags,
    DWORD_PTR)
{
    CurlInternetHandle* conn = AsHandle(hConnect);
    HINTERNET h = MakeRequestFromConnection(conn, lpszFileName);

    CurlInternetHandle* req = AsHandle(h);
    if (req)
        req->flags = dwFlags;

    return h;
}

BOOL InternetReadFile(
    HINTERNET hFile,
    LPVOID lpBuffer,
    DWORD dwNumberOfBytesToRead,
    LPDWORD lpdwNumberOfBytesRead)
{
    CurlInternetHandle* req = AsHandle(hFile);

    if (!req || !lpBuffer || !lpdwNumberOfBytesRead)
    {
        SetCompatError(1);
        return FALSE;
    }

    *lpdwNumberOfBytesRead = 0;

    if (!req->requestPerformed || req->headerOnly)
    {
        if (!PerformRequest(req, false))
            return FALSE;
    }

    if (req->readOffset >= req->data.size())
    {
        *lpdwNumberOfBytesRead = 0;
        SetCompatError(0);
        return TRUE;
    }

    size_t remaining = req->data.size() - req->readOffset;
    size_t toCopy = std::min<size_t>(remaining, dwNumberOfBytesToRead);

    std::memcpy(lpBuffer, req->data.data() + req->readOffset, toCopy);
    req->readOffset += toCopy;

    *lpdwNumberOfBytesRead = static_cast<DWORD>(toCopy);
    SetCompatError(0);
    return TRUE;
}

BOOL InternetQueryDataAvailable(
    HINTERNET hFile,
    LPDWORD lpdwNumberOfBytesAvailable,
    DWORD,
    DWORD_PTR)
{
    CurlInternetHandle* req = AsHandle(hFile);

    if (!req || !lpdwNumberOfBytesAvailable)
    {
        SetCompatError(1);
        return FALSE;
    }

    if (!req->requestPerformed || req->headerOnly)
    {
        if (!PerformRequest(req, false))
            return FALSE;
    }

    size_t remaining = req->readOffset < req->data.size()
        ? req->data.size() - req->readOffset
        : 0;

    *lpdwNumberOfBytesAvailable = static_cast<DWORD>(std::min<size_t>(remaining, 0xFFFFFFFFu));
    SetCompatError(0);
    return TRUE;
}

BOOL InternetCloseHandle(HINTERNET hInternet)
{
    CurlInternetHandle* h = AsHandle(hInternet);

    if (!h)
        return FALSE;

    bool wasSession = (h->type == HandleType::Session);

    if (h->ownsCurl && h->curl)
    {
        curl_easy_cleanup(h->curl);
        h->curl = nullptr;
    }

    delete h;

    if (wasSession)
    {
        std::lock_guard<std::mutex> lock(g_curlMutex);
        if (g_curlUsers > 0)
            --g_curlUsers;

        if (g_curlUsers == 0)
            curl_global_cleanup();
    }

    SetCompatError(0);
    return TRUE;
}
#ifndef _WIN32
DWORD GetLastError(void)
{
    return g_lastError;
}
#endif
} // extern "C"
