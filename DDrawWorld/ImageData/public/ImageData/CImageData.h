#pragma once
#include <cstddef>
#include <cstdint>
#include <windows.h>
#include "Common/sharedapi.h"

namespace ImageData
{
	static_assert(sizeof(::WORD) == 2);
	static_assert(sizeof(::DWORD) == 4);
	static_assert(sizeof(uint16_t) == 2);
	static_assert(sizeof(uint32_t) == 4);
	static_assert(sizeof(uint64_t) == 8);

#pragma pack(push,4)
	// 8 = 2/2/4
	struct SHARED_API PIXEL_STREAM
	{
		uint16_t posX;
		uint16_t sameColorCount;
		uint32_t color;
	};
	static_assert(sizeof(PIXEL_STREAM) == 8);
	static_assert(sizeof(PIXEL_STREAM*) == 8);
#pragma pack(pop)

	// 16 = 4/4/8
	struct SHARED_API COMPRESSED_LINE
	{
		uint32_t pixelStreamCount;
		uint32_t usedByteCount;
		union Data
		{
			PIXEL_STREAM* pixelStreamPtr;
			uint32_t dwOffset;
		};
		Data data;
	};
	static_assert(sizeof(COMPRESSED_LINE) == 16);

	class SHARED_API CImageData final
	{
	private:
		int m_width = 0;
		int m_height = 0;

#pragma warning(push)
#pragma warning(disable: 4251)
		// m_compressedImageBytes: [COMPRESSED_LINE * imgHeight | PIXEL_STREAM ... ]
		std::unique_ptr<std::byte[]> m_compressedImageBytes = nullptr;
#pragma warning(pop)

	public:
		CImageData() = default;
		~CImageData() = default;
	public:
		inline int GetWidth() const
		{
			return m_width;
		}
		inline int GetHeight() const
		{
			return m_height;
		}

		const COMPRESSED_LINE* GetCompressedImage(const int y) const
		{
#ifdef _DEBUG
			if (y < 0 || m_height <= y)
			{
				__debugbreak();
			}
#endif // _DEBUG
			return reinterpret_cast<COMPRESSED_LINE*>(m_compressedImageBytes.get()) + y;
		}
	public:
		bool Create(const std::byte* const imgBytes, const int imgWidth, const int imgHeight, const uint32_t compareColorKey);
	private:
		int ExtractPixelStreamPerLine(PIXEL_STREAM* const pDest, const int iMaxMemSize, const uint32_t* const pSrcBits, const int32_t imgWidth, const uint32_t compareColorKey);
	};
} // ImageData