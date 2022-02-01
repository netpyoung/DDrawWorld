#include "Util/precompiled.h"
#include "Util/CTGAImage.h"

namespace Util
{
	CTGAImage::~CTGAImage()
	{
		Destroy();
	}

	bool CTGAImage::LoadTGAImage(::FILE* const fp, const uint32_t dwBytesPerPixel)
	{
		Destroy();
		TGA_HEADER header = { 0, };

		constexpr size_t HEADER_SIZE = sizeof(header);

		const size_t headerCount = ::fread(&header, HEADER_SIZE, 1, fp);
		if (headerCount != 1)
		{
#ifdef _DEBUG
			__debugbreak();
#endif // _DEBUG
			return false;
		}

		const size_t byteLength = (header.width * header.height * 4);
		std::unique_ptr<std::byte[]> temp = std::make_unique<std::byte[]>(byteLength);
		const size_t size = (header.width * header.height * 3);
		const size_t readSize = fread(temp.get(), sizeof(std::byte), size, fp);
		if (readSize != size)
		{
#ifdef _DEBUG
			__debugbreak();
#endif // _DEBUG
			return false;
		}


		m_rawImage = std::make_unique<std::byte[]>(byteLength);

		if (dwBytesPerPixel == 2)
		{
			m_bytePerPixel = 2;
			const uint32_t dwWidthBytes = header.width * 2;
			uint32_t line = header.height;

			for (int y = 0; y < header.height; ++y)
			{
				line--;
				for (int x = 0; x < header.width; ++x)
				{
					*(WORD*)(m_rawImage.get() + line * header.width * 2 + x * 2) =
						static_cast<WORD>(
							temp[y * header.width * 3 + x * 3 + 0] >> 3 |
							temp[y * header.width * 3 + x * 3 + 1] >> 2 << 5 |
							temp[y * header.width * 3 + x * 3 + 2] >> 3 << 11);
				}
			}
		}
		else if (dwBytesPerPixel == 4)
		{
			m_bytePerPixel = 4;
			const uint32_t dwWidthBytes = header.width * 4;
			uint32_t line = header.height;

			for (int y = 0; y < header.height; ++y)
			{
				line--;
				for (int x = 0; x < header.width; ++x)
				{
					m_rawImage[line * dwWidthBytes + x * 4 + 0] = temp[y * header.width * 3 + x * 3 + 0];
					m_rawImage[line * dwWidthBytes + x * 4 + 1] = temp[y * header.width * 3 + x * 3 + 1];
					m_rawImage[line * dwWidthBytes + x * 4 + 2] = temp[y * header.width * 3 + x * 3 + 2];
					m_rawImage[line * dwWidthBytes + x * 4 + 3] = std::byte(255);
				}
			}
		}
		m_width = header.width;
		m_height = header.height;
		return true;
	}

	void CTGAImage::Destroy()
	{
		m_rawImage = nullptr;
		m_width = 0;
		m_height = 0;
		m_bytePerPixel = 0;
	}

	bool CTGAImage::Load24BitsTGA(const char* const szFileName, const uint32_t dwBytesPerPixel)
	{
		::FILE* fp = nullptr;
		const errno_t err = fopen_s(&fp, szFileName, "rb");
		if (err != 0)
		{
			return false;
		}
		const bool isLoaded = LoadTGAImage(fp, dwBytesPerPixel);
		fclose(fp);

		return isLoaded;
	}

	uint32_t CTGAImage::GetPixel(const uint32_t x, const uint32_t y)
	{
		const uint32_t dwColor = *(uint32_t*)(m_rawImage.get() + ((x + y * m_width) * 4));
		return dwColor;
	}

	// ---------------------
	bool WriteTGA(const char* const filePath, const std::byte* const srcBytes, const int width, const int height, const int pitch, const int bpp)
	{
		FILE* fp = nullptr;
		const errno_t err = fopen_s(&fp, filePath, "wb");
		if (err != 0)
		{
			return false;
		}
		WriteTGAImage(fp, srcBytes, width, height, pitch, bpp);
		fclose(fp);
		return true;
	}

	uint32_t WriteTGAImage(FILE* const fp, const std::byte* const srcBytes, const int width, const int height, const int pitch, const int bpp)
	{
		TGA_HEADER header = { 0, };
		header.width = static_cast<uint16_t>(width);
		header.height = static_cast<uint16_t>(height);
		header.Bits = 24;
		header.ImageType = 2;

		const uint32_t dwOldPos = ftell(fp);
		fwrite(&header, sizeof(header), 1, fp);

		const uint32_t imgByteCount = static_cast<uint32_t>(width * height * 3);
		std::unique_ptr<std::byte[]> byteBuffer = std::make_unique<std::byte[]>(imgByteCount);
		std::byte* dstPtr = byteBuffer.get();
		if (bpp == 16)
		{
			std::byte r;
			std::byte g;
			std::byte b;
			uint16_t pixel;

			for (int y = height - 1; y >= 0; --y)
			{
				for (int x = 0; x < width; ++x)
				{
					pixel = *(uint16_t*)(srcBytes + (x << 1) + pitch * y);
					r = static_cast<std::byte>((pixel & 0x001f) << 3);
					g = static_cast<std::byte>((pixel & 0x07e0) >> 5 << 2);
					b = static_cast<std::byte>((pixel & 0xf800) >> 11 << 3);

					*(dstPtr + 0) = r;
					*(dstPtr + 1) = g;
					*(dstPtr + 2) = b;
					dstPtr += 3;

				}
			}
		}
		else if (bpp == 32)
		{
			for (int y = height - 1; y >= 0; --y)
			{
				for (int x = 0; x < width; ++x)
				{
					*(dstPtr + 0) = *((srcBytes + (x << 2) + pitch * y) + 0);
					*(dstPtr + 1) = *((srcBytes + (x << 2) + pitch * y) + 1);
					*(dstPtr + 2) = *((srcBytes + (x << 2) + pitch * y) + 2);
					dstPtr += 3;
				}
			}
		}
		else if (bpp == 24)
		{
			for (int y = height - 1; y >= 0; --y)
			{
				for (int x = 0; x < width; ++x)
				{
					*(dstPtr + 0) = *((srcBytes + (x * 3) + pitch * y) + 0);
					*(dstPtr + 1) = *((srcBytes + (x * 3) + pitch * y) + 1);
					*(dstPtr + 2) = *((srcBytes + (x * 3) + pitch * y) + 2);
					dstPtr += 3;
				}
			}
		}

		fwrite(byteBuffer.get(), sizeof(std::byte), imgByteCount, fp);

		return ftell(fp) - dwOldPos;
	}

	uint32_t CTGAImage::Blt(
		std::byte* pDestBits, const int dstX, const int dstY,
		const int screenWidth, const int screenHeight, const int dstPitch, const int bytePerPixel,
		const int srcStartX, const int srcStartY, const int srcWidth, const int srcHeight)
	{
		if (m_rawImage == nullptr)
		{
			return 0;
		}

		if (bytePerPixel != (int)m_bytePerPixel)
		{
			__debugbreak();
			return 0;
		}

		const int screen_end_x = screenWidth - 1;
		const int screen_end_y = screenHeight - 1;

		int src_start_x = srcStartX;
		int src_start_y = srcStartY;
		int width = srcWidth;
		int height = srcHeight;
		
		// dest
		// 수평 클리핑
		int dest_start_x = dstX;
		int dest_end_x = dstX + width - 1;
		if (dest_start_x > screen_end_x)
		{
			return 0;
		}
		if (dest_end_x < 0)
		{
			return 0;
		}
		if (dest_start_x < 0)
		{
			width += dest_start_x;
			src_start_x -= dest_start_x;
			dest_start_x = 0;
		}
		const int margineX = screen_end_x - dest_end_x;
		if (margineX < 0)
		{
			width += margineX;
		}

		// 수직 클리핑
		int dest_start_y = dstY;
		int dest_end_y = dstY + height - 1;
		if (dest_start_y > screen_end_y)
		{
			return 0;
		}
		if (dest_end_y < 0)
		{
			return 0;
		}
		if (dest_start_y < 0)
		{
			height += dest_start_y;
			src_start_y -= dest_start_y;
			dest_start_y = 0;
		}
		const int margineY = screen_end_y - dest_end_y;
		if (margineY < 0)
		{
			height += margineY;
		}

		const uint32_t srcPitch = m_width * m_bytePerPixel;
		std::byte* srcBytes = m_rawImage.get() + (src_start_x * m_bytePerPixel) + (src_start_y * srcPitch);
		std::byte* dstBytes = pDestBits + (dest_start_x * bytePerPixel) + (dest_start_y * dstPitch);

		uint32_t dwPixelNum = 0;
		for (int y = 0; y < height; ++y)
		{
			memcpy(dstBytes, srcBytes, width * bytePerPixel);
			srcBytes += srcPitch;
			dstBytes += dstPitch;
			dwPixelNum += width;
		}
		return dwPixelNum;
	}
} // Util