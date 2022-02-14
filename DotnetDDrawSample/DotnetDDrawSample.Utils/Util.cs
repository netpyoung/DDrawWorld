using System;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace DotnetDDrawSample.Utils
{
    public static partial class Util
    {
        [StructLayout(LayoutKind.Sequential, Pack = 1)]
        public struct TGA_HEADER
        {
            public byte idLength;
            public byte ColorMapType;
            public byte ImageType;
            public UInt16 ColorMapFirst;
            public UInt16 ColorMapLast;
            public byte ColorMapBits;
            public UInt16 FirstX;
            public UInt16 FirstY;
            public UInt16 width;
            public UInt16 height;
            public byte Bits;
            public byte Descriptor;
        };

        public static unsafe void WriteTGAImage(FileStream f, byte* srcBytes, int width, int height, int pitch, int bpp)
        {
            int imgByteCount = width * height * 3;
            byte[] byteBuffer = new byte[imgByteCount];
            if (bpp == 16)
            {
                byte r;
                byte g;
                byte b;
                UInt16 pixel;
                int baseIdx = 0;
                for (int y = height - 1; y >= 0; --y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        pixel = *(UInt16*)(srcBytes + (x << 1) + pitch * y);
                        r = (byte)((pixel & 0x001f) << 3);
                        g = (byte)((pixel & 0x07e0) >> 5 << 2);
                        b = (byte)((pixel & 0xf800) >> 11 << 3);

                        byteBuffer[baseIdx + 0] = r;
                        byteBuffer[baseIdx + 1] = g;
                        byteBuffer[baseIdx + 2] = b;
                        baseIdx += 3;
                    }
                }
            }
            else if (bpp == 32)
            {
                byte r;
                byte g;
                byte b;
                int baseIdx = 0;

                for (int y = height - 1; y >= 0; --y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        r = (byte)*((srcBytes + (x << 2) + pitch * y) + 0);
                        g = (byte)*((srcBytes + (x << 2) + pitch * y) + 1);
                        b = (byte)*((srcBytes + (x << 2) + pitch * y) + 2);
                        byteBuffer[baseIdx + 0] = r;
                        byteBuffer[baseIdx + 1] = g;
                        byteBuffer[baseIdx + 2] = b;
                        baseIdx += 3;

                    }
                }
            }
            else if (bpp == 24)
            {
                byte r;
                byte g;
                byte b;
                int baseIdx = 0;

                for (int y = height - 1; y >= 0; --y)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        r = (byte)*((srcBytes + (x * 3) + pitch * y) + 0);
                        g = (byte)*((srcBytes + (x * 3) + pitch * y) + 1);
                        b = (byte)*((srcBytes + (x * 3) + pitch * y) + 2);
                        byteBuffer[baseIdx + 0] = r;
                        byteBuffer[baseIdx + 1] = g;
                        byteBuffer[baseIdx + 2] = b;
                        baseIdx += 3;
                    }
                }
            }

            TGA_HEADER header = new TGA_HEADER();
            header.width = (UInt16)width;
            header.height = (UInt16)height;
            header.Bits = (byte)24;
            header.ImageType = (byte)2;

            ReadOnlySpan<byte> headerSpan = new ReadOnlySpan<byte>(Unsafe.AsPointer(ref header), sizeof(TGA_HEADER));
            ReadOnlySpan<byte> bytebufferSpan = new Span<byte>(byteBuffer);
            f.Write(headerSpan);
            f.Write(bytebufferSpan);
        }

        public static bool CalcClipArea(out int2 pivOutSrcStart, out int2 pivOutDestStart, out int2 pivOutDestSize, int2 pivPos, int2 pivImageSize, int2 pivBufferSize)
        {
            int dest_start_x = Math.Clamp(pivPos.x, 0, pivBufferSize.x);
            int dest_end_x = Math.Clamp(pivPos.x + pivImageSize.x, 0, pivBufferSize.x);
            int width = dest_end_x - dest_start_x;
            if (width <= 0)
            {
                pivOutSrcStart = new int2(0, 0);
                pivOutDestStart = new int2(0, 0);
                pivOutDestSize = new int2(0, 0);
                return false;
            }

            int dest_start_y = Math.Clamp(pivPos.y, 0, pivBufferSize.y);
            int dest_end_y = Math.Clamp(pivPos.y + pivImageSize.y, 0, pivBufferSize.y);
            int height = dest_end_y - dest_start_y;
            if (height <= 0)
            {
                pivOutSrcStart = new int2(0, 0);
                pivOutDestStart = new int2(0, 0);
                pivOutDestSize = new int2(0, 0);
                return false;
            }

            int src_start_x = dest_start_x - pivPos.x;
            int src_start_y = dest_start_y - pivPos.y;
            pivOutSrcStart.x = src_start_x;
            pivOutSrcStart.y = src_start_y;
            pivOutDestStart.x = dest_start_x;
            pivOutDestStart.y = dest_start_y;
            pivOutDestSize.x = width;
            pivOutDestSize.y = height;
            return true;
        }

        public static unsafe bool IsCollisionRectVsRect(in int2 pv3MinA, in int2 pv3MaxA, in int2 pv3MinB, in int2 pv3MaxB)
        {
            fixed (int* a_min = &pv3MinA.x)
            fixed (int* a_max = &pv3MaxA.x)
            fixed (int* b_min = &pv3MinB.x)
            fixed (int* b_max = &pv3MaxB.x)
            {
                for (int i = 0; i < 2; ++i)
                {
                    if (a_min[i] > b_max[i] || a_max[i] < b_min[i])
                    {
                        return false;
                    }
                }
            }
            return true;
        }
    }
}