#pragma once
#include "ddrawsample/precompiled_ddrawsample.h"
#include "Common/math.h"

using namespace Common;

namespace ImageData
{
	class CImageData;
} // ImageData

namespace DDrawSample
{
	enum class FLIGHT_OBJECT_STATUS
	{
		ALIVE,
		DEAD,
	};

	class CFlightObject final
	{
	private:
		int2 m_oldPos = { 0, 0 };
		int2 m_curPos = { 0, 0 };
		int2 m_interpolatedPos = { 0, 0 };

		const ImageData::CImageData* m_imageData = nullptr;
		FLIGHT_OBJECT_STATUS m_status = FLIGHT_OBJECT_STATUS::ALIVE;
		ULONGLONG m_deadTick = 0;
		int m_speed = 0;
	public:
		CFlightObject() = default;
		~CFlightObject() = default;
	public:
		inline ULONGLONG GetDeadTick() const
		{
			return m_deadTick;
		}
		inline void SetDeadTick(const ULONGLONG deadTick)
		{
			m_deadTick = deadTick;
		}
		inline FLIGHT_OBJECT_STATUS GetStatus() const
		{
			return m_status;
		}
		inline void SetStatus(const FLIGHT_OBJECT_STATUS status)
		{
			m_status = status;
		}
		inline const ImageData::CImageData* GetImageData() const
		{
			return m_imageData;
		}
		inline void SetImageData(const ImageData::CImageData* const imageData)
		{
			m_imageData = imageData;
		}
		inline int GetSpeed() const
		{
			return m_speed;
		}
		inline void SetSpeed(int iSpeed)
		{
			m_speed = iSpeed;
		}
		inline const int2& GetInterpolatedPos() const
		{
			return m_interpolatedPos;
		}
		inline const int2& GetPos() const
		{
			return m_curPos;
		}
	public:
		int GetWidth() const;
		int GetHeight() const;
		void SetPos(const int x, const int y, const bool isInterpolation);
		void SetPos(const int2& pos, const bool isInterpolation);
		void FixPos();
		void Interpolate(const float inAlpha);
	};
} // DDrawSample