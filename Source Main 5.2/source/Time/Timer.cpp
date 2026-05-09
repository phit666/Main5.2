// Timer.cpp: implementation of the CTimer class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Timer.h"
#ifdef _WIN32
CTimer::CTimer()
{
	if (::QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency) == FALSE)
	{
		m_bUsePerformanceCounter = FALSE;

		TIMECAPS Caps;
		::timeGetDevCaps(&Caps, sizeof(Caps));
		
		if (::timeBeginPeriod(Caps.wPeriodMin) == TIMERR_NOCANDO)
		{
#ifdef __TIMER_DEBUG
			__TraceF(TEXT("timeBeginPeriod(...) Error\n"));
			//CDebug::OutputDebugString("timeBeginPeriod(...) Error");
#endif //__TIMER_DEBUG 
		}

		m_mmAbsTimerStart = m_mmTimerStart = ::timeGetTime();
	}
	else
	{
		m_bUsePerformanceCounter = TRUE;

		::QueryPerformanceCounter((LARGE_INTEGER*)&m_pcTimerStart);

		m_pcAbsTimerStart = m_pcTimerStart;
		m_resolution = (float)(1.0 / (double)m_frequency) * 1000.0f;
	}
}

CTimer::~CTimer()
{
	if (!m_bUsePerformanceCounter)
	{
		TIMECAPS Caps;
		::timeGetDevCaps(&Caps, sizeof(Caps));
		::timeEndPeriod(Caps.wPeriodMin);
	}
}

double CTimer::GetTimeElapsed()
{
	__int64 timeElapsed;

	if (m_bUsePerformanceCounter)	// if using Performance Counter
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&timeElapsed);
		timeElapsed -= m_pcTimerStart;
		return (double)timeElapsed * (double)m_resolution;
	}
	else	// if not
	{
		timeElapsed = ::timeGetTime() - m_mmTimerStart;
		return (double)timeElapsed;
	}
}

double CTimer::GetAbsTime()
{
	__int64 absTime;

	if (m_bUsePerformanceCounter)	// if using Performance Counter
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&absTime);
		return (double)absTime * (double)m_resolution;
	}
	else	// if not
	{
		absTime = ::timeGetTime();
		return (double)absTime;
	}
}

void CTimer::ResetTimer()
{
	// start time
	if (m_bUsePerformanceCounter)	// if using Performance Counter
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_pcTimerStart);
	else	// if not
		m_mmTimerStart = ::timeGetTime();
}
#else
#include <time.h>
#include <stdint.h>

// Use int64_t for m_frequency and m_pcTimerStart to store nanoseconds
CTimer::CTimer()
{
	// Android (Linux) high-res timers are almost always available
	m_bUsePerformanceCounter = TRUE;

	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
		// Fallback for extremely old/broken kernels, though rare on Android
		m_bUsePerformanceCounter = FALSE;
		return;
	}

	// Convert timespec to a single 64-bit integer (nanoseconds)
	m_pcTimerStart = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
	m_pcAbsTimerStart = m_pcTimerStart;

	// Frequency is constant 1GHz (nanoseconds)
	m_frequency = 1000000000LL;

	// Resolution in ms: (1.0 / 1,000,000,000) * 1,000 = 0.000001 ms per count
	m_resolution = 1.0f / 1000000.0f;
}

CTimer::~CTimer()
{
	if (!m_bUsePerformanceCounter)
	{

	}
}

double CTimer::GetTimeElapsed()
{
	int64_t currentTime;

	if (m_bUsePerformanceCounter)
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		// Convert to nanoseconds
		currentTime = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;

		int64_t timeElapsed = currentTime - m_pcTimerStart;

		// m_resolution was calculated as (1.0 / 1,000,000,000) * 1000
		// which simplifies to timeElapsed / 1,000,000.0 to get milliseconds
		return (double)timeElapsed * (double)m_resolution;
	}
	else
	{
		// Fallback: clock_gettime is standard, but if you must mimic timeGetTime():
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		// Convert directly to milliseconds
		currentTime = (int64_t)ts.tv_sec * 1000LL + (ts.tv_nsec / 1000000LL);

		return (double)(currentTime - m_mmTimerStart);
	}
}

double CTimer::GetAbsTime()
{
	int64_t absTime;

	if (m_bUsePerformanceCounter)
	{
		struct timespec ts;
		// Use CLOCK_MONOTONIC to match the behavior of QueryPerformanceCounter
		clock_gettime(CLOCK_MONOTONIC, &ts);

		// Convert to total nanoseconds
		absTime = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;

		// Multiply by m_resolution (0.000001) to return milliseconds
		return (double)absTime * (double)m_resolution;
	}
	else
	{
		struct timespec ts;
		clock_gettime(CLOCK_MONOTONIC, &ts);

		// Convert to total milliseconds to match timeGetTime()
		return (double)((int64_t)ts.tv_sec * 1000LL + (ts.tv_nsec / 1000000LL));
	}
}

void CTimer::ResetTimer()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	if (m_bUsePerformanceCounter)
	{
		// Store current time in nanoseconds
		m_pcTimerStart = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
	}
	else
	{
		// Store current time in milliseconds (matches timeGetTime)
		m_mmTimerStart = (int64_t)ts.tv_sec * 1000LL + (ts.tv_nsec / 1000000LL);
	}
}
#endif