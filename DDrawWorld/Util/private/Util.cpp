#include "Util/precompiled_util.h"
#include "Util/Util.h"

namespace Util
{
	bool CalcClipArea(
		int2* const pivOutSrcStart,
		int2* const pivOutDestStart,
		int2* const pivOutDestSize,
		const int2& pivPos,
		const int2& pivImageSize,
		const int2& pivBufferSize
	)
	{
		int dest_start_x = std::max(pivPos.x, 0);
		dest_start_x = std::min(dest_start_x, pivBufferSize.x);
		int dest_end_x = std::max(pivPos.x + pivImageSize.x, 0);
		dest_end_x = std::min(dest_end_x, pivBufferSize.x);
		const int width = dest_end_x - dest_start_x;
		if (width <= 0)
		{
			return false;
		}

		int dest_start_y = std::max(pivPos.y, 0);
		dest_start_y = std::min(dest_start_y, pivBufferSize.y);
		int dest_end_y = std::max(pivPos.y + pivImageSize.y, 0);
		dest_end_y = std::min(dest_end_y, pivBufferSize.y);
		const int height = dest_end_y - dest_start_y;
		if (height <= 0)
		{
			return false;
		}

		const int src_start_x = dest_start_x - pivPos.x;
		const int src_start_y = dest_start_y - pivPos.y;
		pivOutSrcStart->x = src_start_x;
		pivOutSrcStart->y = src_start_y;
		pivOutDestStart->x = dest_start_x;
		pivOutDestStart->y = dest_start_y;
		pivOutDestSize->x = width;
		pivOutDestSize->y = height;
		return true;
	}

	bool IsCollisionRectVsRect(const int2* const pv3MinA, const int2* const pv3MaxA, const int2* const pv3MinB, const int2* const pv3MaxB)
	{
		const int* const a_min = &pv3MinA->x;
		const int* const a_max = &pv3MaxA->x;
		const int* const b_min = &pv3MinB->x;
		const int* const b_max = &pv3MaxB->x;

		for (int i = 0; i < 2; ++i)
		{
			if (a_min[i] > b_max[i] || a_max[i] < b_min[i])
			{
				return false;
			}
		}
		return true;
	}
} // Util