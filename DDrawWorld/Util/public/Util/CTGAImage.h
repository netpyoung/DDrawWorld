#pragma once
#include "Common/sharedapi.h"

namespace Util
{
#pragma pack(push, 1)
	struct SHARED_API TGA_HEADER
	{
		char idLength;
		char ColorMapType;
		char ImageType;
		uint16_t ColorMapFirst;
		uint16_t ColorMapLast;
		char ColorMapBits;
		uint16_t FirstX;
		uint16_t FirstY;
		uint16_t width;
		uint16_t height;
		char Bits;
		char Descriptor;
	};
	static_assert(sizeof(TGA_HEADER) == 18);
#pragma pack(pop)

	class SHARED_API CTGAImage final
	{
	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::unique_ptr<std::byte[]> m_rawImage = nullptr;
#pragma warning(pop)
		uint32_t	m_width = 0;
		uint32_t	m_height = 0;
		uint32_t	m_bytePerPixel = 0;
	private:
		void	Destroy();
		bool	LoadTGAImage(::FILE* const fp, const uint32_t dwBytesPerPixel);
	public:
		CTGAImage() = default;
		~CTGAImage();
		CTGAImage& operator=(CTGAImage&&) = default;
		CTGAImage(const CTGAImage&) = delete;
		CTGAImage& operator=(const CTGAImage&) = delete;
	public:
		inline uint32_t	GetWidth() const { return m_width; }
		inline uint32_t	GetHeight() const { return m_height; }
		inline std::byte* GetRawImage() const { return m_rawImage.get(); }
	public:
		bool Load24BitsTGA(const char* const szFileName, const uint32_t dwBytesPerPixel);
		uint32_t GetPixel(const uint32_t x, const uint32_t y);
		uint32_t Blt(
			std::byte* pDestBits, const int iDestX, const int iDestY,
			const int dwScreenWidth, const int dwScreenHeight, const int dwPitch, const int dwBytePerPixel,
			const int srcStartX, const int srcStartY, const int srcWidth, const int srcHeight);
	};

	SHARED_API bool WriteTGA(const char* const filePath, const std::byte* const srcBytes, const int width, const int height, const int pitch, const int bpp);
	SHARED_API uint32_t WriteTGAImage(FILE* const fp, const std::byte* const srcBytes, const int width, const int height, const int pitch, const int bpp);

} // Util