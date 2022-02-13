using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace DotnetDDrawSample.Core
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct PIXEL_STREAM
    {
        public UInt16 posX;
        public UInt16 sameColorCount;
        public UInt32 color;
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct COMPRESSED_LINE
    {
        public UInt32 pixelStreamCount;
        public UInt32 usedByteCount;

        [StructLayout(LayoutKind.Explicit, Pack = 1)]
        public unsafe struct Data
        {
            [FieldOffset(0)]
            public PIXEL_STREAM* pixelStreamPtr;
            [FieldOffset(0)]
            public UInt32 dwOffset;
        };
        public Data data;
    };

    public class ImageData
    {
        public int Width { get; private set; }
        public int Height { get; private set; }

        byte[] m_compressedImageBytes;

        public unsafe COMPRESSED_LINE* GetCompressedImage(in int y)
        {
            Debug.Assert(0 <= y && y < Height);
            fixed (byte* compressedImageBytesPtr = m_compressedImageBytes)
            {
                return ((COMPRESSED_LINE*)compressedImageBytesPtr) + y;
            }
        }

        public unsafe bool Create(byte[] imgBytes, int imgWidth, int imgHeight, uint colorKey)
        {
            int reservedByteCount = sizeof(PIXEL_STREAM) * imgWidth * imgHeight;

            PIXEL_STREAM[] pixelStreams = new PIXEL_STREAM[imgWidth * imgHeight];
            COMPRESSED_LINE[] compressedLines = new COMPRESSED_LINE[imgHeight];

            int dwUsedMemSize = 0;

            fixed (PIXEL_STREAM* pixelStreamsPtr = pixelStreams)
            fixed (byte* imgBytesPtr = imgBytes)
            {
                PIXEL_STREAM* outCurrPixelStream = pixelStreamsPtr;
                UInt32* imgColors = (UInt32*)imgBytesPtr;
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

                    int memoryByteOffset = (int)(outCurrPixelStream - pixelStreamsPtr);

                    int usedByteCount = sizeof(PIXEL_STREAM) * pixelStreamCount;
                    Debug.Assert((reservedByteCount - usedByteCount) >= 0);

                    reservedByteCount -= usedByteCount;

                    dwUsedMemSize += usedByteCount;
                    outCurrPixelStream += pixelStreamCount;

                    compressedLines[y].usedByteCount = (uint)usedByteCount;
                    compressedLines[y].pixelStreamCount = (uint)pixelStreamCount;
                    compressedLines[y].data.dwOffset = (uint)memoryByteOffset;
                }
            }

            // compressedImageBytes: [COMPRESSED_LINE * imgHeight | PIXEL_STREAM ... ]
            // 수집한 픽셀 스트림과 각 라인의 디스크립터가 되는 COMPRESSED_LINE메모리를 한번에 할당한다.
            dwUsedMemSize += sizeof(COMPRESSED_LINE) * imgHeight;
            byte[] compressedImageBytes = new byte[dwUsedMemSize];

            fixed (PIXEL_STREAM* pixelStreamsPtr = pixelStreams)
            fixed (COMPRESSED_LINE* compressedLinesPtr = compressedLines)
            fixed (byte* compressedImageBytesPtr = compressedImageBytes)
            {
                COMPRESSED_LINE* sectionCompressedLinePtr = (COMPRESSED_LINE*)compressedImageBytesPtr;
                PIXEL_STREAM* sectionPixelStreamPtr = (PIXEL_STREAM*)(compressedImageBytesPtr + sizeof(COMPRESSED_LINE) * imgHeight);
                uint accPixelStreamCount = 0;
                for (int y = 0; y < imgHeight; ++y)
                {
                    accPixelStreamCount += compressedLines[y].pixelStreamCount;

                    PIXEL_STREAM* storedPixelStream = pixelStreamsPtr + accPixelStreamCount;                       // 앞서 수집한 픽셀 스트림이 저장된 메모리
                    Unsafe.CopyBlock(sectionPixelStreamPtr, storedPixelStream, compressedLines[y].usedByteCount);  // 수집한 픽셀 스트림을 확정된 메모리로 카피.
                    compressedLines[y].data.pixelStreamPtr = sectionPixelStreamPtr;                                // 써넣을 메모리
                    sectionPixelStreamPtr += compressedLines[y].pixelStreamCount;
                }
                Unsafe.CopyBlock(sectionCompressedLinePtr, compressedLinesPtr, (uint)(sizeof(COMPRESSED_LINE) * imgHeight));
            }
            Width = imgWidth;
            Height = imgHeight;
            m_compressedImageBytes = compressedImageBytes;
            return true;
        }

        private unsafe int ExtractPixelStreamPerLine(
            PIXEL_STREAM* outPixelStreams,
            in int maxMemSize,
            in UInt32* srcColors,
            in int width,
            in UInt32 compareColorKey)
        {

            int pixelStreamCount = 0;
            int remainMemSize = maxMemSize;

            for (int x = 0; x < width;)
            {
                while (srcColors[x] == compareColorKey)
                {
                    x++;
                    if (x >= width)
                    {
                        return pixelStreamCount;
                    }
                }

                Debug.Assert((remainMemSize - sizeof(PIXEL_STREAM)) >= 0);
                remainMemSize -= sizeof(PIXEL_STREAM);

                UInt32 currColor = srcColors[x];

                outPixelStreams[pixelStreamCount].posX = (UInt16)x;
                outPixelStreams[pixelStreamCount].color = currColor;
                while (x < width && srcColors[x] == currColor)
                {
                    outPixelStreams[pixelStreamCount].sameColorCount++;
                    x++;
                }
                pixelStreamCount++;
            }
            return pixelStreamCount;
        }
    }
}