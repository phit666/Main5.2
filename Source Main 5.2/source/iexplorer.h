#ifndef _IEXPLORER_H_
#define _IEXPLORER_H_

#pragma once


namespace leaf {
	/* Open URL for default web-browser    */
	/* This function return process handle */

	inline bool OpenExplorer(const std::string& url)
	{
#ifdef _WIN32
		// Windows
    if (32 < (UINT)ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_NORMAL))
        return true;
    return false;

#elif defined(__ANDROID__)
		// Android (via JNI)
		// You MUST call Java to open URL
		// Stub here:
		return false;

#else
		// Linux / others
    std::string cmd = "xdg-open \"" + url + "\"";
    return system(cmd.c_str()) == 0;
#endif
	}


}


#endif // _IEXPLORER_H_