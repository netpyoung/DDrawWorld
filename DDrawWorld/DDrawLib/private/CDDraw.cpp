#include "ddrawlib/precompiled.h"
#include "ddrawlib/CDDraw.h"
#include "Util/CTGAImage.h"
#include "Common/math.h"
#include "Util/Util.h"
#include "ImageData/CImageData.h"

using namespace Common;

namespace DDrawLib
{
	CDDraw::CDDraw()
	{
		memset(this, 0, sizeof(CDDraw));
	}

	CDDraw::~CDDraw()
	{
		if (m_ddBack != nullptr)
		{
			m_ddBack->Release();
			m_ddBack = nullptr;
		}
		if (m_ddClipper != nullptr)
		{
			m_ddClipper->Release();
			m_ddClipper = nullptr;
		}
		if (m_ddPrimary != nullptr)
		{
			m_ddPrimary->Release();
			m_ddPrimary = nullptr;
		}
		if (m_dd7 != nullptr)
		{
			m_dd7->Release();
			m_dd7 = nullptr;
		}
		if (m_dd != nullptr)
		{
			m_dd->Release();
			m_dd = nullptr;
		}
	}

	bool CDDraw::Initialize(const ::HWND hWnd)
	{
		m_hWnd = hWnd;

		DDSURFACEDESC2 ddsd = { 0, };
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

		if (::DirectDrawCreate(nullptr, &m_dd, nullptr) != DD_OK)
		{
			return false;
		}

		if (m_dd->QueryInterface(IID_IDirectDraw7, reinterpret_cast<void**>(&m_dd7)) != DD_OK)
		{
			return false;
		}

		if (m_dd7->SetCooperativeLevel(hWnd, DDSCL_NORMAL) != DD_OK)
		{
			return false;
		}

		if (m_dd7->CreateSurface(&ddsd, &m_ddPrimary, nullptr) != DD_OK)
		{
			return false;
		}

		if (m_dd7->CreateClipper(0, &m_ddClipper, nullptr) != DD_OK)
		{
			return false;
		}

		if (m_ddClipper->SetHWnd(0, hWnd) != DD_OK)
		{
			return false;
		}

		if (m_ddPrimary->SetClipper(m_ddClipper) != DD_OK)
		{
			return false;
		}

		UpdateWindowPos(&m_rectWindow);

		const int width = m_rectWindow.right - m_rectWindow.left;
		const int height = m_rectWindow.bottom - m_rectWindow.top;
		if (!CreateBackBuffer(width, height, &m_ddBack))
		{
#ifdef _DEBUG
			__debugbreak();
#endif //_DEBUG
			return false;
		}

		const ::HDC hdc = ::GetDC(m_hWnd);
		const int planes = ::GetDeviceCaps(hdc, PLANES);
		const int bp = ::GetDeviceCaps(hdc, BITSPIXEL);
		UNREFERENCED_PARAMETER(planes);
		UNREFERENCED_PARAMETER(bp);
		::ReleaseDC(m_hWnd, hdc);
		return true;
	}

	bool CDDraw::CreateBackBuffer(const int width, const int height, IDirectDrawSurface7** const outBackbuffer)
	{
		DDSURFACEDESC2 ddsd = { 0, };
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
		ddsd.dwWidth = width;
		ddsd.dwHeight = height;

		if (m_dd7->CreateSurface(&ddsd, outBackbuffer, nullptr) != DD_OK)
		{
			return false;
		}

		(*outBackbuffer)->GetSurfaceDesc(&ddsd);
		m_width = ddsd.dwWidth;
		m_height = ddsd.dwHeight;

		return true;
	}

	void CDDraw::UpdateWindowPos(RECT* outRect)
	{
		::GetClientRect(m_hWnd, outRect);
		::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->left));
		::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->right));
	}

	bool CDDraw::LockBackBuffer(std::byte** const outBits, int* const outWidth, int* const outHeight, int* const outPitch)
	{
		if (m_ddBack == nullptr)
		{
			return true;
		}

		DDSURFACEDESC2 ddsc;
		memset(&ddsc, 0, sizeof(DDSURFACEDESC2));
		ddsc.dwSize = sizeof(DDSURFACEDESC2);

		m_ddBack->Lock(nullptr, &ddsc, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr);

		*outBits = reinterpret_cast<std::byte*>(ddsc.lpSurface);
		*outWidth = ddsc.dwWidth;
		*outHeight = ddsc.dwHeight;
		*outPitch = ddsc.lPitch;
		return true;
	}

	void CDDraw::UnlockBackBuffer()
	{
		if (m_ddBack == nullptr)
		{
			return;
		}
		m_ddBack->Unlock(nullptr);
	}

	bool CDDraw::BeginDraw()
	{
		std::byte* bits = nullptr;
		int width;
		int height;
		int pitch;
		if (!LockBackBuffer(&bits, &width, &height, &pitch))
		{
			return false;
		}

		if (width != m_width)
		{
#ifdef _DEBUG
			__debugbreak();
#endif //_DEBUG
			return false;
		}

		if (height != m_height)
		{
#ifdef _DEBUG
			__debugbreak();
#endif //_DEBUG
			return false;
		}

		assert(bits != nullptr);
		m_lockedBackBuffer = bits;
		m_lockedBackBufferPitch = pitch;

		return true;
	}

	void CDDraw::EndDraw()
	{
		if (m_ddBack != nullptr)
		{
			m_ddBack->Unlock(nullptr);
		}
	}

	void CDDraw::Clear()
	{
		if (m_lockedBackBuffer == nullptr)
		{
#ifdef _DEBUG
			__debugbreak();
#endif // _DEBUG
			return;
		}

#define __RGB32BIT(a,r,g,b) (b+(g<<8)+(r<<16)+(a<<24))

		const int bytesPerLine = 4 * m_width;
		for (int y = 0; y < m_height; ++y)
		{
			memset(m_lockedBackBuffer + y * m_lockedBackBufferPitch, 0, bytesPerLine);
		}
	}

	// ----------------
	void CDDraw::OnUpdateWindowSize()
	{
		if (m_ddBack != nullptr)
		{
			m_ddBack->Release();
			m_ddBack = nullptr;
		}

		UpdateWindowPos(&m_rectWindow);
		const int width = m_rectWindow.right - m_rectWindow.left;
		const int height = m_rectWindow.bottom - m_rectWindow.top;
		if (!CreateBackBuffer(width, height, &m_ddBack))
		{
#ifdef _DEBUG
			__debugbreak();
#endif //_DEBUG
			return;
		}

		if (BeginDraw())
		{
			Clear();
			EndDraw();
		}
	}

	void CDDraw::OnUpdateWindowPos()
	{
		UpdateWindowPos(&m_rectWindow);
	}

	bool CDDraw::BeginGDI(::HDC* const outHdc)
	{
		::HDC hdc = nullptr;
		if (m_ddBack->GetDC(&hdc) != DD_OK)
		{
#ifdef _DEBUG
			__debugbreak();
#endif // _DEBUG
			return false;
		}
		*outHdc = hdc;
		return true;
	}

	void CDDraw::DrawInfo(const ::HDC hdc)
	{
		const WCHAR* const wchTxt = m_infoText;
		const DWORD len = m_infoTextLen;
		
		WriteText(wchTxt, len, 0, 0, 0xffff0000, hdc);
	}

	void CDDraw::EndGDI(const ::HDC hdc)
	{
		m_ddBack->ReleaseDC(hdc);
	}

	void CDDraw::OnDraw()
	{
		m_ddPrimary->Blt(&m_rectWindow, m_ddBack, nullptr, DDBLT_WAIT, nullptr);
	}
	// -------------------
	bool CDDraw::DrawBitmap(const int startX, const int startY, const int imgWidth, const int imgHeight, const std::byte* const imgBytes)
	{
#ifdef _DEBUG
		if (m_lockedBackBuffer == nullptr)
		{
			__debugbreak();
		}
#endif // _DEBUG

		int2 ivSrcStart = { 0, 0 };
		int2 ivDestStart = { 0, 0 };

		int2 ivPos = { startX, startY };
		int2 ivImageSize = { imgWidth, imgHeight };
		int2 ivDestSize = { 0, 0 };

		if (!CalcClipArea(&ivSrcStart, &ivDestStart, &ivDestSize, ivPos, ivImageSize))
		{
			return false;
		}

		const std::byte* pSrc = imgBytes + (ivSrcStart.x + ivSrcStart.y * imgWidth) * 4;
		std::byte* pDest = m_lockedBackBuffer + (ivDestStart.x * 4) + ivDestStart.y * m_lockedBackBufferPitch;

		for (int y = 0; y < ivDestSize.y; y++)
		{
			for (int x = 0; x < ivDestSize.x; x++)
			{
				*(uint32_t*)pDest = *(uint32_t*)pSrc;
				pSrc += 4;
				pDest += 4;
			}
			pSrc -= (ivDestSize.x * 4);
			pSrc += (imgWidth * 4);
			pDest -= (ivDestSize.x * 4);
			pDest += m_lockedBackBufferPitch;
		}
		return true;
	}

	bool CDDraw::CalcClipArea(int2* const pivOutSrcStart, int2* const pivOutDestStart, int2* const pivOutDestSize, const int2& pivPos, const int2& pivImageSize)
	{
		int2 ivBufferSize = { m_width, m_height };
		bool isSuccess = Util::CalcClipArea(pivOutSrcStart, pivOutDestStart, pivOutDestSize, pivPos, pivImageSize, ivBufferSize);
		return isSuccess;
	}
	// ---------------------------------
	bool CDDraw::DrawImageData(const int startX, const int startY, const ImageData::CImageData* const imageData)
	{
#ifdef _DEBUG
		if (!m_lockedBackBuffer)
		{
			__debugbreak();
		}
#endif // _DEBUG

		int iScreenWidth = m_width;

		int iBitmapWidth = imageData->GetWidth();
		int iBitmapHeight = imageData->GetHeight();

		int2 ivSrcStart;
		int2 ivDestStart;
		int2 ivDestSize;
		const int2 ivPos = { startX, startY };
		const int2 ivImageSize = { iBitmapWidth, iBitmapHeight };
		if (!CalcClipArea(&ivSrcStart, &ivDestStart, &ivDestSize, ivPos, ivImageSize))
		{
			return false;
		}

		const ImageData::COMPRESSED_LINE* pLineDesc = imageData->GetCompressedImage(ivSrcStart.y);
		std::byte* pDestPerLine = m_lockedBackBuffer + (ivDestStart.y) * m_lockedBackBufferPitch;

		for (int y = 0; y < ivDestSize.y; ++y)
		{
			for (uint32_t i = 0; i < pLineDesc->pixelStreamCount; ++i)
			{
				const ImageData::PIXEL_STREAM* const pStream = pLineDesc->data.pixelStreamPtr + i;
				const uint32_t dwPixelColor = pStream->color;
				int iPixelNum = (int)pStream->sameColorCount;

				int dest_x = startX + (int)pStream->posX;
				if (dest_x < 0)
				{
					iPixelNum += dest_x;
					dest_x = 0;
				}
				if (dest_x + iPixelNum > iScreenWidth)
				{
					iPixelNum = iScreenWidth - dest_x;
				}
				const std::byte* pDest = pDestPerLine + (static_cast<int64_t>(dest_x) * 4);
				for (int x = 0; x < iPixelNum; x++)
				{
					*(uint32_t*)pDest = dwPixelColor;
					pDest += 4;
				}
			}
			pLineDesc++;
			pDestPerLine += m_lockedBackBufferPitch;
		}
		return true;
	}

	// --------------


	uint32_t CDDraw::CaptureBackBuffer(FILE* const fp)
	{
		if (m_ddBack == nullptr)
		{
			return 0;
		}

		DDSURFACEDESC2 ddsc;
		memset(&ddsc, 0, sizeof(DDSURFACEDESC2));
		ddsc.dwSize = sizeof(DDSURFACEDESC2);

		m_ddBack->Lock(nullptr, &ddsc, DDLOCK_WAIT | DDLOCK_READONLY, nullptr);
		const uint32_t result = Util::WriteTGAImage(fp, (std::byte*)ddsc.lpSurface, ddsc.dwWidth, ddsc.dwHeight, ddsc.lPitch, 32);
		m_ddBack->Unlock(nullptr);
		return result;
	}

	// --------------
	void CDDraw::DrawRect(const int startX, const int startY, const int width, const int height, const uint32_t color)
	{
#ifdef _DEBUG
		if (m_lockedBackBuffer == nullptr)
		{
			__debugbreak();
		}
#endif // _DEBUG

		const int start_x = std::max(0, startX);
		const int start_y = std::max(0, startY);
		const int end_x = std::min(startX + width, m_width);
		const int end_y = std::min(startY + height, m_height);

		for (int y = start_y; y < end_y; ++y)
		{
			for (int x = start_x; x < end_x; ++x)
			{
				uint32_t* pDest = (uint32_t*)(m_lockedBackBuffer + x * 4 + y * m_lockedBackBufferPitch);
				*pDest = color;
			}
		}
	}

	bool CDDraw::DrawBitmapWithColorKey(
		const int startX, const int startY, const int bitmapWidth, const int bitmapHeight,
		std::byte* const bitmapBytes,
		const uint32_t colorKey)
	{
#ifdef _DEBUG
		if (m_lockedBackBuffer == nullptr)
		{
			__debugbreak();
		}
#endif // _DEBUG

		int2 srcStart = {};
		int2 dstStart = {};
		int2 dstSize = {};

		const int2 imagePos = { startX, startY };
		const int2 imageSize = { bitmapWidth, bitmapHeight };
		if (!CalcClipArea(&srcStart, &dstStart, &dstSize, imagePos, imageSize))
		{
			return false;
		}

		std::byte* srcPtr = bitmapBytes + (srcStart.x + srcStart.y * bitmapWidth) * 4;
		std::byte* dstPtr = m_lockedBackBuffer + (dstStart.x * 4) + dstStart.y * m_lockedBackBufferPitch;
		for (int y = 0; y < dstSize.y; y++)
		{
			for (int x = 0; x < dstSize.x; x++)
			{
				if (*(DWORD*)srcPtr != colorKey)
				{
					*(DWORD*)dstPtr = *(DWORD*)srcPtr;
				}
				srcPtr += 4;
				dstPtr += 4;
			}
			srcPtr -= (dstSize.x * 4);
			srcPtr += (bitmapWidth * 4);
			dstPtr -= (dstSize.x * 4);
			dstPtr += m_lockedBackBufferPitch;
		}

		return true;
	}
	void CDDraw::UpdateInfoText()
	{
		m_infoTextLen = swprintf_s(m_infoText, L"FPS : %u", m_FPS);
	}

	bool CDDraw::CheckFPS()
	{
		const ULONGLONG curTick = GetTickCount64();
		if (curTick - m_lastDrawTick > 1000)
		{
			const int oldPFS = m_FPS;

			m_FPS = m_frameCount;
			m_lastDrawTick = curTick;
			m_frameCount = 0;
			if (m_FPS != oldPFS)
			{
				UpdateInfoText();
				return true;
			}
		}
		m_frameCount++;
		return false;
	}

	void CDDraw::WriteText(const WCHAR* const wchTxt, const DWORD dwLen, const int x, const int y, const uint32_t color, const HDC hdc)
	{
		int fontWidth = 0;
		int fontHeight = 0;
		bool isSuccess = GetFontSize(&fontWidth, &fontHeight, wchTxt, dwLen, hdc);
		if (!isSuccess)
		{
			return;
		}
		
		RECT textRect;
		textRect.left = x;
		textRect.top = y;
		textRect.right = textRect.left + fontWidth;
		textRect.bottom = textRect.top + fontHeight;

		RECT texRectSide[4];
		texRectSide[0].left = textRect.left - 1;
		texRectSide[0].top = textRect.top - 1;
		texRectSide[0].right = textRect.right - 1;
		texRectSide[0].bottom = textRect.bottom - 1;

		texRectSide[1].left = textRect.left + 1;
		texRectSide[1].top = textRect.top - 1;
		texRectSide[1].right = textRect.right + 1;
		texRectSide[1].bottom = textRect.bottom - 1;

		texRectSide[2].left = textRect.left + 1;
		texRectSide[2].top = textRect.top + 1;
		texRectSide[2].right = textRect.right + 1;
		texRectSide[2].bottom = textRect.bottom + 1;

		texRectSide[3].left = textRect.left - 1;
		texRectSide[3].top = textRect.top + 1;
		texRectSide[3].right = textRect.right - 1;
		texRectSide[3].bottom = textRect.bottom + 1;

		::SetBkMode(hdc, TRANSPARENT);
		::SetTextColor(hdc, 0x00000000);

		// DT_LEFT      | Aligns text to the left.
		// DT_WORDBREAK | Breaks words.Lines are automatically broken between words
		for (int i = 0; i < 4; ++i)
		{
			::DrawText(hdc, wchTxt, -1, &texRectSide[i], DT_LEFT | DT_WORDBREAK);
		}

		const ::DWORD r = (color & 0x00ff0000) >> 16;
		const ::DWORD g = (color & 0x0000ff00) >> 8;
		const ::DWORD b = (color & 0x000000ff);
		const ::COLORREF rgb = RGB(r, g, b);
		::SetTextColor(hdc, rgb);
		::DrawText(hdc, wchTxt, -1, &textRect, DT_LEFT | DT_WORDBREAK);
	}

	bool CDDraw::GetFontSize(int* const outWidth, int* const outHeight, const ::WCHAR* const wchString, const uint32_t dwStrLen, const ::HDC hDC)
	{
		SIZE sz;
		const bool isSuccess = ::GetTextExtentPoint32W(hDC, wchString, dwStrLen, &sz);
		if (!isSuccess)
		{
			return false;
		}

		*outWidth = sz.cx;
		*outHeight = sz.cy;
		return true;
	}
} // DDrawLib