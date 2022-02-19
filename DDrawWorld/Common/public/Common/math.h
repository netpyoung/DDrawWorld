#pragma once

namespace Common
{
	struct int2
	{
		int x;
		int y;
		inline	bool	operator==(const int2& v);
		inline	int2	operator +(const int2& v);
		inline	int2	operator -(const int2& v);
		inline	int2	operator *(float f);
		inline	int2	operator /(float f);
		inline	int2	operator *(const int2& v);
	};

	inline bool int2::operator==(const int2& v)
	{
		return (this->x == v.x && this->y == v.y);
	}

	inline int2 int2::operator +(const int2& v)
	{
		int2 r;
		r.x = this->x + v.x;
		r.y = this->y + v.y;
		return r;
	}

	inline int2 int2::operator -(const int2& v)
	{
		int2 r;
		r.x = this->x - v.x;
		r.y = this->y - v.y;
		return r;
	}

	inline int2 int2::operator *(float f)
	{
		int2 r;
		r.x = static_cast<int>(this->x * f);
		r.y = static_cast<int>(this->y * f);
		return r;
	}

	inline int2 int2::operator /(float f)
	{
		int2 r;
		r.x = static_cast<int>(this->x / f);
		r.y = static_cast<int>(this->y / f);
		return r;
	}

	inline int2 int2::operator *(const int2& v)
	{
		int2 r;
		r.x = this->x * v.x;
		r.y = this->y * v.y;
		return r;
	}

	struct int2_rect
	{
		int2 min;
		int2 max;
	};
}