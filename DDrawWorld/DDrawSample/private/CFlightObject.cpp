#include "ddrawsample/precompiled_ddrawsample.h"
#include "Common/math.h"
#include "ImageData/CImageData.h"
#include "ddrawsample/CFlightObject.h"

namespace DDrawSample
{
	int CFlightObject::GetWidth() const
	{
		return m_imageData->GetWidth();
	}
	int CFlightObject::GetHeight() const
	{
		return m_imageData->GetHeight();
	}

	void CFlightObject::SetPos(const int x, const int y, const bool isInterpolation)
	{
		if (isInterpolation)
		{
			m_oldPos = m_curPos;
		}
		else
		{
			m_oldPos.x = x;
			m_oldPos.y = y;
			m_interpolatedPos.x = x;
			m_interpolatedPos.y = y;
		}
		m_curPos.x = x;
		m_curPos.y = y;
	}
	void CFlightObject::SetPos(const int2& pos, const bool isInterpolation)
	{
		if (isInterpolation)
		{
			m_oldPos = m_curPos;
		}
		else
		{
			m_oldPos = pos;
			m_interpolatedPos = pos;
		}
		m_curPos = pos;
	}
	void CFlightObject::FixPos()
	{
		m_oldPos = m_curPos;
		m_interpolatedPos = m_curPos;
	}
	void CFlightObject::Interpolate(const float inAlpha)
	{
		const float alpha = std::clamp(inAlpha, 0.0f, 1.0f);
		const float x_velocity = (float)(m_curPos.x - m_oldPos.x);
		const float y_velocity = (float)(m_curPos.y - m_oldPos.y);
		
		m_interpolatedPos.x = (int)((float)m_oldPos.x + x_velocity * alpha);
		m_interpolatedPos.y = (int)((float)m_oldPos.y + y_velocity * alpha);
	}
} // DDrawSample