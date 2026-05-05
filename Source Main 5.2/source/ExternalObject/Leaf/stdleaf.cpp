
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "stdleaf.h"

#ifdef __ANDROID__
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#endif

#include "mu_sdl.h"


#ifdef __ANDROID__

bool leaf::CreateDirectoryIncSub(const std::string& path)
{
	std::string cur;

	for (size_t i = 0; i < path.size(); ++i)
	{
		char ch = path[i];
		cur += ch;

		if (ch == '/' || ch == '\\' || i == path.size() - 1)
		{
			if (cur.empty())
				continue;

			mkdir(cur.c_str(), 0777);
		}
	}

	return true;
}

bool leaf::DeleteDirectoryIncSub(const std::string& path)
{
	DIR* dir = opendir(path.c_str());
	if (!dir)
		return false;

	struct dirent* ent;

	while ((ent = readdir(dir)) != nullptr)
	{
		if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;

		std::string full = path + "/" + ent->d_name;

		struct stat st;
		if (stat(full.c_str(), &st) == 0)
		{
			if (S_ISDIR(st.st_mode))
				DeleteDirectoryIncSub(full);
			else
				remove(full.c_str());
		}
	}

	closedir(dir);
	return rmdir(path.c_str()) == 0;
}

bool leaf::GetFileSizeQW(const HANDLE hFile, QWORD& qwSize)
{
	qwSize = 0;
	return false;
}

void leaf::GetAbsolutePath(const std::string& path, std::string& abspath)
{
	char buf[PATH_MAX];

	if (realpath(path.c_str(), buf))
		abspath = buf;
	else
		abspath = path;
}

void leaf::GetAbsoluteFilePath(const std::string& path, std::string& abspath, std::string& filename)
{
	GetAbsolutePath(path, abspath);

	size_t pos = abspath.find_last_of("/\\");
	filename = (pos == std::string::npos) ? abspath : abspath.substr(pos + 1);
}

void leaf::SplitFileName(const std::string& filepath, std::string& filename, bool bIncludeExt)
{
	size_t slash = filepath.find_last_of("/\\");
	filename = (slash == std::string::npos) ? filepath : filepath.substr(slash + 1);

	if (!bIncludeExt)
	{
		size_t dot = filename.find_last_of('.');
		if (dot != std::string::npos)
			filename = filename.substr(0, dot);
	}
}

void leaf::SplitDirectoryPath(const std::string& filepath, std::string& dir)
{
	size_t slash = filepath.find_last_of("/\\");
	dir = (slash == std::string::npos) ? "" : filepath.substr(0, slash + 1);
}

void leaf::SplitExt(const std::string& filepath, std::string& ext, bool bIncludeDot)
{
	size_t slash = filepath.find_last_of("/\\");
	size_t dot = filepath.find_last_of('.');

	if (dot == std::string::npos || (slash != std::string::npos && dot < slash))
	{
		ext.clear();
		return;
	}

	ext = bIncludeDot ? filepath.substr(dot) : filepath.substr(dot + 1);
}

void leaf::ExchangeExt(const std::string& in_filepath, const std::string& ext, std::string& out_filepath)
{
	size_t dot = in_filepath.find_last_of('.');
	size_t slash = in_filepath.find_last_of("/\\");

	if (dot != std::string::npos && (slash == std::string::npos || dot > slash))
		out_filepath = in_filepath.substr(0, dot);
	else
		out_filepath = in_filepath;

	out_filepath += ".";
	out_filepath += ext;
}

bool leaf::GetFileCreationTime(const std::string&, SYSTEMTIME& systime, bool)
{
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	return false;
}

bool leaf::GetFileLastModifiedTime(const std::string& path, SYSTEMTIME& systime, bool)
{
	ZeroMemory(&systime, sizeof(SYSTEMTIME));

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	struct tm tmv;
	localtime_r(&st.st_mtime, &tmv);

	systime.wYear = tmv.tm_year + 1900;
	systime.wMonth = tmv.tm_mon + 1;
	systime.wDay = tmv.tm_mday;
	systime.wHour = tmv.tm_hour;
	systime.wMinute = tmv.tm_min;
	systime.wSecond = tmv.tm_sec;
	systime.wMilliseconds = 0;

	return true;
}

bool leaf::GetFileLastAccessedTime(const std::string& path, SYSTEMTIME& systime, bool)
{
	ZeroMemory(&systime, sizeof(SYSTEMTIME));

	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;

	struct tm tmv;
	localtime_r(&st.st_atime, &tmv);

	systime.wYear = tmv.tm_year + 1900;
	systime.wMonth = tmv.tm_mon + 1;
	systime.wDay = tmv.tm_mday;
	systime.wHour = tmv.tm_hour;
	systime.wMinute = tmv.tm_min;
	systime.wSecond = tmv.tm_sec;
	systime.wMilliseconds = 0;

	return true;
}

void leaf::GetTimeString(std::string& str)
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	struct tm tmv;
	localtime_r(&tv.tv_sec, &tmv);

	char szTime[256];

	sprintf(szTime, "%d/%d/%d %d:%02d:%02d:%03ld",
		1900 + tmv.tm_year,
		tmv.tm_mon + 1,
		tmv.tm_mday,
		tmv.tm_hour,
		tmv.tm_min,
		tmv.tm_sec,
		tv.tv_usec / 1000);

	str = szTime;
}

#else

// GetTimeString
#include <time.h>
#include <sys/timeb.h>

bool leaf::CreateDirectoryIncSub(const std::string& path) {
	
	for(int i=1; i<(int)path.size(); i++) {
		if(path[i] == '/' || path[i] == '\\') {
			std::string subpath = path.substr(0, i+1/* length */);
			
			if(GetFileAttributes(subpath.c_str()) == 0xFFFFFFFF) {
				if(!CreateDirectory(subpath.c_str(), NULL))
					return false;
			}
		}
		else if(i == (int)(path.size())-1) {
			if(GetFileAttributes(path.c_str()) == 0xFFFFFFFF) {
				if(!CreateDirectory(path.c_str(), NULL))
					return false;
			}
		}
	}
	return true;
}
bool leaf::DeleteDirectoryIncSub(const std::string& path) {
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	
	std::string	finddir = path;
	finddir += "/*";
	hFind = FindFirstFile(finddir.data(), &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE)
		return false;
	
	HRESULT hr = true;
	do{
		std::string filename = path;
		filename += '/';
		filename += FindFileData.cFileName;
		
		if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) == FILE_ATTRIBUTE_ARCHIVE)
			hr = DeleteFile(filename.data());
		
		else if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			if(stricmp(FindFileData.cFileName, ".") && stricmp(FindFileData.cFileName, ".."))
				hr = DeleteDirectoryIncSub(filename.data());
			
			if(!hr){
				FindClose(hFind);
				return false;
			}
	}
	while(FindNextFile(hFind, &FindFileData));
	
	FindClose(hFind);
	
	if(!RemoveDirectory(path.c_str()))
		return false;
	
	return true;
}

bool leaf::GetFileSizeQW(const HANDLE hFile, QWORD &qwSize) 
{
	if(hFile == NULL)
		return false;
	
	DWORD dwLo = 0, dwHi = 0;
	dwLo = GetFileSize(hFile, &dwHi);
	
	if(dwLo == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) {
		qwSize = 0;
		return false;
	}
	qwSize = MAKEQWORD(dwLo, dwHi);
	return true;
}

void leaf::GetAbsolutePath(IN const std::string& path, OUT std::string& abspath) {
	char* pszFileName = NULL;
	char szFullPath[_MAX_PATH] = {0, };
	GetFullPathName(path.c_str(), _MAX_PATH, szFullPath, &pszFileName);
	abspath = szFullPath;
}
void leaf::GetAbsoluteFilePath(IN const std::string& path, OUT std::string& abspath, OUT std::string& filename) {
	char* pszFileName = NULL;
	char szFullPath[_MAX_PATH] = {0, };
	GetFullPathName(path.c_str(), _MAX_PATH, szFullPath, &pszFileName);
	abspath = szFullPath;
	filename = pszFileName;
}

void leaf::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt) {
	char __fname[_MAX_FNAME] = {0, };
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
	filename = __fname;
	if(bIncludeExt)
		filename += __ext;
}
void leaf::SplitDirectoryPath(IN const std::string& filepath, OUT std::string& dir) {
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	_splitpath(filepath.c_str(), __drive, __dir, NULL, NULL);
	dir = __drive;
	dir += __dir;
}
void leaf::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot) {
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
	if(bIncludeDot) {
		ext = __ext;
	}
	else {
		if((__ext[0] == '.') && __ext[1])
			ext = __ext+1;
	}
}

void leaf::ExtractDirectoryName(IN const std::string& path, OUT std::string& dirname) {
	int start = 0;
	int end = path.size()-1;
	for(int i=end; i>=start; i--) {
		if(path[i] == '\\' || path[i] == '/') {
			if(i == end) {
				end = i-1;
			}
			else {
				start = i+1;
				dirname = path.substr(start, end-start+1);
				break;
			}
		}
		else if(i == start) {
			dirname = path.substr(start, end-start+1);
		}
	}
}
void leaf::ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath) {
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	char __fname[_MAX_FNAME] = {0, };
	_splitpath(in_filepath.c_str(), __drive, __dir, __fname, NULL);
	
	out_filepath = __drive;
	out_filepath += __dir;
	out_filepath += __fname;
	out_filepath += '.';
	out_filepath += ext;
}

bool leaf::CompareFilePath(IN const std::string& path1, IN const std::string& path2, size_t size)
{
	bool bIdentity = true;
	for(int i=0; i<(int)size; i++) {
		if((path1[i] == '\\' || path1[i] == '/') && 
			(path2[i] == '\\' || path2[i] == '/')) continue;
		if(_tolower(path1[i]) != _tolower(path2[i])) {
			bIdentity = false;
			break;
		}
	}
	return bIdentity;
}
void leaf::AppendFilePath(IN const std::string& dir, IN const std::string& to_append, OUT std::string& out_path)
{
	if(dir.empty()) {	// 'dir' is empty.
		if(!to_append.empty()) {	//. 'to_append' is NOT empty.
			char ch = to_append[0];
			if(ch == '\\' || ch == '/')
				out_path = to_append.substr(1,to_append.size()-1);
			else
				out_path = to_append;
		}
	}
	else {	// 'dir' is NOT empty.
		char ch = dir[dir.size()-1];
		if(ch == '\\' || ch == '/') {
			out_path = dir.substr(0,dir.size()-1);
		}
		else {
			out_path = dir;
		}

		if(!to_append.empty()) {	// 'to_append' is NOT empty.
			ch = to_append[0];
			if(ch != '\\' && ch != '/')
				out_path += '\\';
			out_path += to_append;
		}
	}
}

bool leaf::GetFileCreationTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftCreationTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}
bool leaf::GetFileLastModifiedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF || dwAttr == FILE_ATTRIBUTE_DIRECTORY)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftLastWriteTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}
bool leaf::GetFileLastAccessedTime(IN const std::string& path, OUT SYSTEMTIME& systime, bool bLocalTime) {
	ZeroMemory(&systime, sizeof(SYSTEMTIME));
	DWORD dwAttr = ::GetFileAttributes(path.c_str());
	if(dwAttr == 0xFFFFFFFF || dwAttr == FILE_ATTRIBUTE_DIRECTORY)
		return false;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;

	FindClose(hFind);

	FILETIME ftime = FindFileData.ftLastAccessTime;
	if(bLocalTime)
		FileTimeToLocalFileTime(&FindFileData.ftLastWriteTime, &ftime);

	FileTimeToSystemTime(&ftime, &systime);

	return true;
}

const char* leaf::FormatString(const char* pFormat, ...)
{
	static char buf[8][2048];
	static DWORD index = 0;

	va_list marker;
	va_start(marker, pFormat);
	vsprintf(buf[index&7], pFormat, marker);
	va_end(marker);

	return ((const char*)buf[index++ & 7]);
}
void leaf::GetTimeString(OUT std::string& str) {
	_timeb ftime;
	_ftime(&ftime);
	tm* plocaltime = localtime(&ftime.time);
	
	char szTime[256] = {0, };
	sprintf(szTime, "%d/%d/%d %d:%02d:%02d:%003d", 1900+plocaltime->tm_year,plocaltime->tm_mon+1,plocaltime->tm_mday, 
		plocaltime->tm_hour,plocaltime->tm_min,plocaltime->tm_sec,ftime.millitm);
	str = szTime;
}
#endif