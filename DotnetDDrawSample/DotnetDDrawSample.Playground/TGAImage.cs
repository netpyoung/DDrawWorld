using System;
using System.IO;
using System.Runtime.CompilerServices;

namespace DotnetDDrawSample.Core
{
    static partial class Util
    {
        internal class TGAImage : IDisposable
        {
            private int m_bytePerPixel = 0;
            public int Width { get; private set; }
            public int Height { get; private set; }
            public byte[] RawImage { get; private set; }

            public void Dispose()
            {
                m_bytePerPixel = 0;
                Width = 0;
                Height = 0;
                RawImage = null;
            }

            public bool Load24BitsTGA(in string fpath, in int bytesPerPixel)
            {
                Dispose();

                using (FileStream f = File.Open(fpath, FileMode.Open))
                {
                    return LoadTGAImage(f, bytesPerPixel);
                }
            }

            private unsafe bool LoadTGAImage(in FileStream f, in int bytesPerPixel)
            {
                TGA_HEADER header = new TGA_HEADER();

                Span<byte> span = new Span<byte>(Unsafe.AsPointer(ref header), sizeof(TGA_HEADER));
                int headReadSize = f.Read(span);
                if (headReadSize != sizeof(TGA_HEADER))
                {
                    return false;
                }

                int size = (header.width * header.height * 3);
                byte[] temp = new byte[size];
                Span<byte> tempSpan = new Span<byte>(temp);
                int bodyReadSize = f.Read(tempSpan);
                if (bodyReadSize != size)
                {
                    return false;
                }

                int byteLength = (header.width * header.height * 4);
                RawImage = new byte[byteLength];

                if (bytesPerPixel == 2)
                {
                    m_bytePerPixel = 2;
                    int dwWidthBytes = header.width * 2;
                    int line = header.height;

                    fixed (byte* rawImagePtr = RawImage)
                    {
                        for (int y = 0; y < header.height; ++y)
                        {
                            line--;
                            for (int x = 0; x < header.width; ++x)
                            {
                                *(UInt16*)(rawImagePtr + line * header.width * 2 + x * 2) =
                                    (UInt16)(
                                        temp[y * header.width * 3 + x * 3 + 0] >> 3 |
                                        temp[y * header.width * 3 + x * 3 + 1] >> 2 << 5 |
                                        temp[y * header.width * 3 + x * 3 + 2] >> 3 << 11);
                            }
                        }
                    }
                }
                else if (bytesPerPixel == 4)
                {
                    m_bytePerPixel = 4;
                    int dwWidthBytes = header.width * 4;
                    int line = header.height;

                    for (int y = 0; y < header.height; ++y)
                    {
                        line--;
                        for (int x = 0; x < header.width; ++x)
                        {
                            RawImage[line * dwWidthBytes + x * 4 + 0] = temp[y * header.width * 3 + x * 3 + 0];
                            RawImage[line * dwWidthBytes + x * 4 + 1] = temp[y * header.width * 3 + x * 3 + 1];
                            RawImage[line * dwWidthBytes + x * 4 + 2] = temp[y * header.width * 3 + x * 3 + 2];
                            RawImage[line * dwWidthBytes + x * 4 + 3] = 255;
                        }
                    }
                }
                Width = header.width;
                Height = header.height;
                return true;
            }

            public unsafe uint GetPixel(in int x, in int y)
            {
                fixed (byte* rawImagePtr = RawImage)
                {
                    uint dwColor = *(uint*)(rawImagePtr + ((x + y * Width) * 4));
                    return dwColor;
                }
            }

            public unsafe int Blt(
                byte* pDestBits, in int dstX, in int dstY,
                in int screenWidth, in int screenHeight, in int dstPitch, in int bytePerPixel,
                in int srcStartX, in int srcStartY, in int srcWidth, in int srcHeight)
            {
                if (RawImage == null)
                {
                    return 0;
                }

                if (bytePerPixel != m_bytePerPixel)
                {
                    return 0;
                }

                int screen_end_x = screenWidth - 1;
                int screen_end_y = screenHeight - 1;

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

                int margineX = screen_end_x - dest_end_x;
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

                int margineY = screen_end_y - dest_end_y;
                if (margineY < 0)
                {
                    height += margineY;
                }

                int srcPitch = Width * m_bytePerPixel;
                fixed (byte* rawImagePtr = RawImage)
                {
                    byte* srcBytes = rawImagePtr + (src_start_x * m_bytePerPixel) + (src_start_y * srcPitch);
                    byte* dstBytes = pDestBits + (dest_start_x * bytePerPixel) + (dest_start_y * dstPitch);
                    int dwPixelNum = 0;

                    for (int y = 0; y < height; ++y)
                    {
                        Unsafe.CopyBlock((void*)dstBytes, (void*)srcBytes, (uint)(width * bytePerPixel));
                        srcBytes += srcPitch;
                        dstBytes += dstPitch;
                        dwPixelNum += width;
                    }
                    return dwPixelNum;
                }
            }
        }
    }
}