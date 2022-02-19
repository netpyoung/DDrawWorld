#include "Util/precompiled_util.h"
#include "Util/QueryPerfCounter.h"

namespace Util::QueryPerfCounter
{
	::LARGE_INTEGER g_Frequency = {0, };

	void QCInit()
	{
		QueryPerformanceFrequency(&g_Frequency);
	}

	::LARGE_INTEGER QCGetCounter()
	{
		::LARGE_INTEGER curCounter;
		QueryPerformanceCounter(&curCounter);
		return curCounter;
	}

	float QCMeasureElapsedTick(const ::LARGE_INTEGER currCounter, const ::LARGE_INTEGER prevCounter)
	{
#ifdef _DEBUG
		if (!g_Frequency.QuadPart)
		{
			__debugbreak();
		}
#endif // _DEBUG

		const UINT64 elapsedCounter = currCounter.QuadPart - prevCounter.QuadPart;
		const float elapsedSec = ((float)elapsedCounter / (float)g_Frequency.QuadPart);
		const float elapsedMilSec = elapsedSec * 1000.0f;

		return elapsedMilSec;
	}

	::LARGE_INTEGER QCCounterAddTick(const ::LARGE_INTEGER counter, const float tick)
	{
#ifdef _DEBUG
		if (!g_Frequency.QuadPart)
		{
			__debugbreak();
		}
#endif // _DEBUG

		::LARGE_INTEGER	result = counter;

		const float sec = tick / 1000.0f;
		const UINT64 elapsedCounter = (UINT64)(sec * (float)g_Frequency.QuadPart);
		result.QuadPart += elapsedCounter;

		return result;
	}

	::LARGE_INTEGER QCCounterSubTick(const ::LARGE_INTEGER counter, const float tick)
	{
#ifdef _DEBUG
		if (!g_Frequency.QuadPart)
		{
			__debugbreak();
		}
#endif // _DEBUG
		::LARGE_INTEGER	result = counter;

		const float sec = tick / 1000.0f;
		const UINT64 elapsedCounter = (UINT64)(sec * (float)g_Frequency.QuadPart);
		result.QuadPart -= elapsedCounter;

		return result;
	}
} // Util::QueryPerfCounter