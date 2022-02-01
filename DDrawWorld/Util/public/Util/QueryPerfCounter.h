#include "Util/precompiled.h"

namespace Util::QueryPerfCounter
{
	void QCInit();
	::LARGE_INTEGER QCGetCounter();
	float QCMeasureElapsedTick(const ::LARGE_INTEGER currCounter, const ::LARGE_INTEGER prevCounter);
	::LARGE_INTEGER QCCounterAddTick(const ::LARGE_INTEGER counter, const float tick);
	::LARGE_INTEGER QCCounterSubTick(const ::LARGE_INTEGER counter, const float tick);
} // Util::QueryPerfCounter