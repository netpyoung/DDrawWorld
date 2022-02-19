#include "ImageData/precompiled_imagedata.h"
#include "ImageData/CImageData.h"

namespace ImageData
{
	bool CImageData::Create(const std::byte* const imgBytes, const int imgWidth, const int imgHeight, const uint32_t colorKey)
	{
		int reservedByteCount = sizeof(PIXEL_STREAM) * imgWidth * imgHeight;

		std::unique_ptr<PIXEL_STREAM[]> pixelStreams = std::make_unique<PIXEL_STREAM[]>(static_cast<int64_t>(imgWidth) * imgHeight);
		std::unique_ptr<COMPRESSED_LINE[]> compressedLines = std::make_unique<COMPRESSED_LINE[]>(imgHeight);

		PIXEL_STREAM* outCurrPixelStream = pixelStreams.get();
		::DWORD dwUsedMemSize = 0;
		const uint32_t* imgColors = reinterpret_cast<const uint32_t*>(imgBytes);
		for (int y = 0; y < imgHeight; ++y)
		{
			int pixelStreamCount = ExtractPixelStreamPerLine(
				outCurrPixelStream,
				reservedByteCount,
				imgColors,
				imgWidth,
				colorKey
			);
			imgColors += imgWidth;

			const uint32_t memoryByteOffset = (uint32_t)(outCurrPixelStream - pixelStreams.get());

			int usedByteCount = sizeof(PIXEL_STREAM) * pixelStreamCount;
			if ((reservedByteCount - usedByteCount) < 0)
			{
				__debugbreak();
			}
			reservedByteCount -= usedByteCount;

			dwUsedMemSize += usedByteCount;
			outCurrPixelStream += pixelStreamCount;

			COMPRESSED_LINE& currCompressedLine = compressedLines[y];
			currCompressedLine.usedByteCount = usedByteCount;
			currCompressedLine.pixelStreamCount = pixelStreamCount;
			currCompressedLine.data.dwOffset = memoryByteOffset;
		}

		// compressedImageBytes: [COMPRESSED_LINE * imgHeight | PIXEL_STREAM ... ]
		// 수집한 픽셀 스트림과 각 라인의 디스크립터가 되는 COMPRESSED_LINE메모리를 한번에 할당한다.
		dwUsedMemSize += sizeof(COMPRESSED_LINE) * imgHeight;
		std::unique_ptr<std::byte[]> compressedImageBytes = std::make_unique<std::byte[]>(dwUsedMemSize);

		COMPRESSED_LINE* const sectionCompressedLinePtr = reinterpret_cast<COMPRESSED_LINE*>(compressedImageBytes.get());
		PIXEL_STREAM* sectionPixelStreamPtr = reinterpret_cast<PIXEL_STREAM*>(compressedImageBytes.get() + sizeof(COMPRESSED_LINE) * imgHeight);
		int accPixelStreamCount = 0;
		for (int y = 0; y < imgHeight; ++y)
		{
			COMPRESSED_LINE& compressedLine = compressedLines[y];
			accPixelStreamCount += compressedLine.pixelStreamCount;

			const PIXEL_STREAM* const storedPixelStream = pixelStreams.get() + accPixelStreamCount;	// 앞서 수집한 픽셀 스트림이 저장된 메모리
			memcpy(sectionPixelStreamPtr, storedPixelStream, compressedLine.usedByteCount);			// 수집한 픽셀 스트림을 확정된 메모리로 카피.

			compressedLine.data.pixelStreamPtr = sectionPixelStreamPtr;								// 써넣을 메모리
			sectionPixelStreamPtr += compressedLine.pixelStreamCount;
		}
		memcpy(sectionCompressedLinePtr, compressedLines.get(), sizeof(COMPRESSED_LINE) * imgHeight);

		m_width = imgWidth;
		m_height = imgHeight;
		m_compressedImageBytes = std::move(compressedImageBytes);

#ifdef _DEBUG
		_ASSERT(_CrtCheckMemory());
#endif // _DEBUG

		return true;
	}

	int CImageData::ExtractPixelStreamPerLine(
		PIXEL_STREAM* const	outPixelStreams,
		const int maxMemSize,
		const uint32_t* const srcColors,
		const int32_t width,
		const uint32_t compareColorKey)
	{
		int pixelStreamCount = 0;
		int remainMemSize = maxMemSize;

		for (int x = 0; x < width; )
		{
			while (srcColors[x] == compareColorKey)
			{
				x++;
				if (x >= width)
				{
					return pixelStreamCount;
				}
			}

			if (remainMemSize - sizeof(PIXEL_STREAM) < 0)
			{
				__debugbreak();
			}
			remainMemSize -= sizeof(PIXEL_STREAM);

			const uint32_t currColor = srcColors[x];

			PIXEL_STREAM& currOutPixelStream = outPixelStreams[pixelStreamCount];

			currOutPixelStream.posX = static_cast<uint16_t>(x);
			currOutPixelStream.color = currColor;
			while (x < width && srcColors[x] == currColor)
			{
				currOutPixelStream.sameColorCount++;
				x++;
			}
			pixelStreamCount++;
		}
		return pixelStreamCount;
	}
} // ImageData