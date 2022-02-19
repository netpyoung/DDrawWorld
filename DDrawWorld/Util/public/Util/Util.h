#pragma once
#include "Common/sharedapi.h"
#include "Common/math.h"

using namespace Common;

namespace Util
{
	SHARED_API bool CalcClipArea(
		int2* const pivOutSrcStart,
		int2* const pivOutDestStart, 
		int2* const pivOutDestSize,
		const int2& pivPos, 
		const int2& pivImageSize,
		const int2& pivBufferSize
	);
	SHARED_API bool IsCollisionRectVsRect(
		const int2* const pv3MinA,
		const int2* const pv3MaxA,
		const int2* const pv3MinB,
		const int2* const pv3MaxB
	);
} // Util