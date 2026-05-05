#ifndef _RDTSC_H_
#define _RDTSC_H_

#include <stdint.h>
#include <chrono>
#include <thread>

namespace leaf {

// ===== Android / portable implementation =====

	inline bool IsSupportRdtsc()
	{
		return false; // not applicable on Android
	}

	inline int64_t GetClockCount()
	{
		return std::chrono::high_resolution_clock::now()
				.time_since_epoch().count();
	}

	inline int64_t GetCPUFrequency(unsigned int /*uiMeasureMSecs*/)
	{
		// Not meaningful on Android
		return 0;
	}

// Optional: helper (recommended)
	inline uint64_t GetTimeMS()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now().time_since_epoch()
		).count();
	}

} // namespace leaf

#endif