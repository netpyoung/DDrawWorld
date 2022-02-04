#pragma once
#include "ddrawlib/precompiled.h"
#include "Common/math.h"

using namespace Common;

namespace ImageData
{
	class CImageData;
} // ImageData

namespace DDrawLib
{
	class SHARED_API CDDraw final
	{
	private:
		::HWND m_hWnd = nullptr;

		::IDirectDraw* m_dd = nullptr;
		::IDirectDraw7* m_dd7 = nullptr;
		::IDirectDrawSurface7* m_ddPrimary = nullptr;
		::IDirectDrawSurface7* m_ddBack = nullptr;
		::IDirectDrawClipper* m_ddClipper = nullptr;
		::RECT m_rectWindow = {0, };
		int m_width = 0;
		int m_height = 0;
	private:
		std::byte* m_lockedBackBuffer = nullptr;
		int m_lockedBackBufferPitch = 0;
	private:
		int m_FPS = 0;
		int m_infoTextLen = 0;
		::WCHAR m_infoText[64] = {0, };
		::ULONGLONG m_lastDrawTick = 0;
		int m_frameCount = 0;
	public:
		CDDraw();
		~CDDraw();
	public:
		inline int GetWidth() const
		{
			return m_width;
		}
		inline int GetHeight() const
		{
			return m_height;
		}
	public:
		bool Initialize(const ::HWND hWnd);
		void UpdateWindowPos(RECT* outRect);
		bool CreateBackBuffer(const int width, const int height, IDirectDrawSurface7** const backbuffer);
		bool LockBackBuffer(std::byte** const outBits, int* const outWidth, int* const outHeight, int* const outPitch);
		void UnlockBackBuffer();
	public:
		bool BeginDraw();
		void Clear();
		void EndDraw();
	public:
		void OnUpdateWindowSize();
		void OnUpdateWindowPos();
	public:
		bool BeginGDI(::HDC* const outHdc);
		void DrawInfo(const ::HDC hdc);
		void EndGDI(const ::HDC hdc);
		void OnDraw();
		bool CheckFPS();
	public:
		bool DrawBitmap(const int startX, const int startY, const int imgWidth, const int imgHeight, const std::byte* const imgBytes);
		bool CalcClipArea(int2* const pivOutSrcStart, int2* const pivOutDestStart, int2* const pivOutDestSize, const int2& pivPos, const int2& pivImageSize);
	public:
		bool DrawImageData(const int startX, const int startY, const ImageData::CImageData* const imageData);
	public:
		uint32_t CaptureBackBuffer(FILE* const fp);
	public:
		void DrawRect(const int startX, const int startY, const int width, const int height, const uint32_t color);
		void UpdateInfoText();
		bool DrawBitmapWithColorKey(
			const int sx, const int sy, const int iBitmapWidth, const int iBitmapHeight,
			std::byte* const pBits,
			const uint32_t colorKey);
		void WriteText(const WCHAR* const wchTxt, const DWORD dwLen, const int x, const int y, const uint32_t color, const HDC hdc);
		bool GetFontSize(int* const outWidth, int* const outHeight, const ::WCHAR* const wchString, const uint32_t dwStrLen, const ::HDC hDC);
	};
}