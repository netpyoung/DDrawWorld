#include "Util/precompiled.h"

namespace Util::QueryPerfCounter
{
	SHARED_API void QCInit();
	SHARED_API ::LARGE_INTEGER QCGetCounter();
	SHARED_API float QCMeasureElapsedTick(const ::LARGE_INTEGER currCounter, const ::LARGE_INTEGER prevCounter);
	SHARED_API ::LARGE_INTEGER QCCounterAddTick(const ::LARGE_INTEGER counter, const float tick);
	SHARED_API ::LARGE_INTEGER QCCounterSubTick(const ::LARGE_INTEGER counter, const float tick);
} // Util::QueryPerfCounter