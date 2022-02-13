using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using COLORREF = TerraFX.Interop.Windows.COLORREF;
using DDBLT = TerraFX.Interop.DirectX.DDBLT;
using DDLOCK = TerraFX.Interop.DirectX.DDLOCK;
using DDSCAPS = TerraFX.Interop.DirectX.DDSCAPS;
using DDSCL = TerraFX.Interop.DirectX.DDSCL;
using DDSD = TerraFX.Interop.DirectX.DDSD;
using DDSURFACEDESC2 = TerraFX.Interop.DirectX.DDSURFACEDESC2;
using DirectX = TerraFX.Interop.DirectX.DirectX;
using DT = TerraFX.Interop.Windows.DT;
using HANDLE = TerraFX.Interop.Windows.HANDLE;
using HDC = TerraFX.Interop.Windows.HDC;
using HRESULT = TerraFX.Interop.Windows.HRESULT;
using HWND = TerraFX.Interop.Windows.HWND;
using IDirectDraw = TerraFX.Interop.DirectX.IDirectDraw;
using IDirectDraw7 = TerraFX.Interop.DirectX.IDirectDraw7;
using IDirectDrawClipper = TerraFX.Interop.DirectX.IDirectDrawClipper;
using IDirectDrawSurface7 = TerraFX.Interop.DirectX.IDirectDrawSurface7;
using IID = TerraFX.Interop.Windows.IID;
using POINT = TerraFX.Interop.Windows.POINT;
using RECT = TerraFX.Interop.Windows.RECT;
using SIZE = TerraFX.Interop.Windows.SIZE;
using Windows = TerraFX.Interop.Windows.Windows;

namespace DotnetDDrawSample.Core
{
    internal unsafe class DDraw : IDisposable
    {
        private TerraFX.Interop.Windows.ComPtr<IDirectDraw> _dd = null;
        private TerraFX.Interop.Windows.ComPtr<IDirectDraw7> _dd7 = null;
        private TerraFX.Interop.Windows.ComPtr<IDirectDrawSurface7> _ddPrimary = null;
        private TerraFX.Interop.Windows.ComPtr<IDirectDrawSurface7> _ddBack = null;
        private TerraFX.Interop.Windows.ComPtr<IDirectDrawClipper> _ddClipper = null;

        private byte* m_lockedBackBuffer;
        private int m_lockedBackBufferPitch;
        private RECT _rectWindow;
        private HWND _hwnd;
        private string _txt_FPS = "helloworld";

        public int Width { get; private set; }
        public int Height { get; private set; }

        public void Dispose()
        {
            if (_ddBack.Get() != null)
            {
                _ddBack.Get()->Release();
                _ddBack = null;
            }
            if (_ddClipper.Get() != null)
            {
                _ddClipper.Get()->Release();
                _ddClipper = null;
            }
            if (_ddPrimary.Get() != null)
            {
                _ddPrimary.Get()->Release();
                _ddPrimary = null;
            }
            if (_dd7.Get() != null)
            {
                _dd7.Get()->Release();
                _dd7 = null;
            }
            if (_dd.Get() != null)
            {
                _dd.Get()->Release();
                _dd = null;
            }
        }

        public bool InitializeDDraw(HWND hwnd)
        {
            _hwnd = hwnd;

            if (DirectX.DirectDrawCreate(null, _dd.GetAddressOf(), null) != DirectX.DD_OK)
            {
                return false;
            }

            fixed (Guid* iid = &IID.IID_IDirectDraw7)
            {
                if (_dd.Get()->QueryInterface(iid, (void**)_dd7.GetAddressOf()) != DirectX.DD_OK)
                {
                    return false;
                }
            }
            if (_dd7.Get()->SetCooperativeLevel(hwnd, DDSCL.DDSCL_NORMAL) != DirectX.DD_OK)
            {
                return false;
            }

            DDSURFACEDESC2 ddsd = new DDSURFACEDESC2();
            ddsd.dwSize = (uint)sizeof(DDSURFACEDESC2);
            ddsd.dwFlags = DDSD.DDSD_CAPS;
            ddsd.ddsCaps.dwCaps = 0x200; // 0x200 == DDSDCAPS.DDSDCAPS_PRIMARYSURFACE;
            if (_dd7.Get()->CreateSurface(&ddsd, _ddPrimary.GetAddressOf(), null) != DirectX.DD_OK)
            {
                return false;
            }
            if (_dd7.Get()->CreateClipper(0, _ddClipper.GetAddressOf(), null) != DirectX.DD_OK)
            {
                return false;
            }
            if (_ddClipper.Get()->SetHWnd(0, hwnd) != DirectX.DD_OK)
            {
                return false;
            }
            if (_ddPrimary.Get()->SetClipper(_ddClipper) != DirectX.DD_OK)
            {
                return false;
            }

            UpdateWindowPos(out _rectWindow);

            int width = _rectWindow.right - _rectWindow.left;
            int height = _rectWindow.bottom - _rectWindow.top;
            if (!CreateBackBuffer(width, height, _ddBack.GetAddressOf()))
            {
                return false;
            }
            Debug.Assert(_ddBack.Get() != null);
            return true;
        }

        private bool CreateBackBuffer(int width, int height, IDirectDrawSurface7** backBuffer)
        {
            DDSURFACEDESC2 ddsd = new DDSURFACEDESC2();
            ddsd.dwSize = (uint)sizeof(DDSURFACEDESC2);
            ddsd.dwFlags = DDSD.DDSD_CAPS | DDSD.DDSD_WIDTH | DDSD.DDSD_HEIGHT;
            ddsd.ddsCaps.dwCaps = DDSCAPS.DDSCAPS_OFFSCREENPLAIN | DDSCAPS.DDSCAPS_SYSTEMMEMORY;
            ddsd.dwWidth = (uint)width;
            ddsd.dwHeight = (uint)height;

            if (_dd7.Get()->CreateSurface(&ddsd, backBuffer, null) != DirectX.DD_OK)
            {
                return false;
            }

            (*backBuffer)->GetSurfaceDesc(&ddsd);
            Width = (int)ddsd.dwWidth;
            Height = (int)ddsd.dwHeight;
            return true;
        }

        internal void CaptureBackBuffer(FileStream f)
        {
            if (_ddBack.Get() == null)
            {
                return;
            }
            DDSURFACEDESC2 ddsd = new DDSURFACEDESC2();
            ddsd.dwSize = (uint)sizeof(DDSURFACEDESC2);
            HRESULT result = _ddBack.Get()->Lock(null, &ddsd, DDLOCK.DDLOCK_WAIT | DDLOCK.DDLOCK_WRITEONLY, HANDLE.NULL);
            if (result != DirectX.DD_OK)
            {
                return;
            }

            Util.WriteTGAImage(f, (byte*)ddsd.lpSurface, (int)ddsd.dwWidth, (int)ddsd.dwHeight, ddsd.lPitch, 32);

            _ddBack.Get()->Unlock(null);
        }

        private void UpdateWindowPos(out RECT rectWindow)
        {
            fixed (RECT* r = &rectWindow)
            {
                Windows.GetClientRect(_hwnd, r);
                Windows.ClientToScreen(_hwnd, (POINT*)&r->left);
                Windows.ClientToScreen(_hwnd, (POINT*)&r->right);
            }
        }

        public bool BeginDraw()
        {
            if (!LockBackBuffer(out byte* bits, out int width, out int height, out int pitch))
            {
                return false;
            }

            if (width != Width)
            {
                return false;
            }

            if (height != Height)
            {
                return false;
            }
            Debug.Assert(bits != null);
            m_lockedBackBuffer = bits;
            m_lockedBackBufferPitch = pitch;
            return true;
        }


        public void Clear()
        {
            if (m_lockedBackBuffer == null)
            {
                return;
            }

            uint bytesPerLine = (uint)(4 * Width);
            for (int y = 0; y < Height; ++y)
            {
                Unsafe.InitBlock((m_lockedBackBuffer + y * m_lockedBackBufferPitch), 0, bytesPerLine);
            }
        }

        bool LockBackBuffer(out byte* outBits, out int outWidth, out int outHeight, out int outPitch)
        {
            if (_ddBack.Get() == null)
            {
                outBits = null;
                outWidth = 0;
                outHeight = 0;
                outPitch = 0;
                return true;
            }

            DDSURFACEDESC2 ddsd = new DDSURFACEDESC2();
            ddsd.dwSize = (uint)sizeof(DDSURFACEDESC2);
            HRESULT result = _ddBack.Get()->Lock(null, &ddsd, DDLOCK.DDLOCK_WAIT | DDLOCK.DDLOCK_WRITEONLY, HANDLE.NULL);
            if ((int)result != DirectX.DD_OK)
            {
                outBits = null;
                outWidth = 0;
                outHeight = 0;
                outPitch = 0;
                return false;
            }

            Debug.Assert(ddsd.lpSurface != null);
            outBits = (byte*)ddsd.lpSurface;
            outWidth = (int)ddsd.dwWidth;
            outHeight = (int)ddsd.dwHeight;
            outPitch = ddsd.lPitch;
            return true;
        }

        public void EndDraw()
        {
            UnlockBackBuffer();
            m_lockedBackBuffer = null;
            m_lockedBackBufferPitch = 0;
        }

        private void UnlockBackBuffer()
        {
            if (_ddBack.Get() == null)
            {
                return;
            }
            _ddBack.Get()->Unlock(null);
        }

        // -----------------------
        #region Windows
        public void OnUpdateWindowSize()
        {
            if (_ddBack.Get() == null)
            {
                _ddBack.Get()->Release();
                _ddBack = null;
            }

            UpdateWindowPos(out _rectWindow);
            int width = _rectWindow.right - _rectWindow.left;
            int height = _rectWindow.bottom - _rectWindow.top;
            if (!CreateBackBuffer(width, height, _ddBack.GetAddressOf()))
            {
                return;
            }

            if (BeginDraw())
            {
                Clear();
                EndDraw();
            }
        }

        public void OnUpdateWindowPos()
        {
            UpdateWindowPos(out _rectWindow);
        }
        #endregion // Windows

        #region GDI

        public bool BeginGDI(out TerraFX.Interop.Windows.HDC outHdc)
        {
            HDC hdc;
            HRESULT hr = _ddBack.Get()->GetDC(&hdc);
            if (hr != DirectX.DD_OK)
            {
                outHdc = HDC.NULL;
                return false;
            }
            outHdc = hdc;
            return true;
        }

        internal void DrawInfo(TerraFX.Interop.Windows.HDC hdc)
        {
            WriteText(_txt_FPS, 0, 0, 0xffff00FF, hdc);
        }

        public void EndGDI(TerraFX.Interop.Windows.HDC hdc)
        {
            EndGDI_ProcessGDI(hdc);
            _ddBack.Get()->ReleaseDC(hdc);
        }

        private void EndGDI_ProcessGDI(TerraFX.Interop.Windows.HDC hdc)
        {
        }

        public void OnDraw()
        {
            fixed (RECT* rectPtr = &_rectWindow)
            {
                _ddPrimary.Get()->Blt(rectPtr, _ddBack.Get(), null, DDBLT.DDBLT_WAIT, null);
            }
        }

        public void WriteText(string wchTxt, int x, int y, uint dwColor, HDC hDC)
        {
            Windows.SetBkMode(hDC, Windows.TRANSPARENT);

            int iWidth = 0;
            int iHeight = 0;
            GetFontSize(out iWidth, out iHeight, wchTxt, hDC);

            RECT textRect;
            RECT[] texRectSide = new RECT[4];
            textRect.left = x;
            textRect.top = y;
            textRect.right = textRect.left + iWidth;
            textRect.bottom = textRect.top + iHeight;

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

            Windows.SetTextColor(hDC, 0x00000000);
            for (int i = 0; i < 4; ++i)
            {
                fixed (char* strPtr = wchTxt)
                fixed (RECT* rectPtr = &texRectSide[i])
                {
                    Windows.DrawText(hDC, (ushort*)strPtr, -1, rectPtr, DT.DT_LEFT | DT.DT_WORDBREAK);
                }
            }

            byte r = (byte)((dwColor & 0x00ff0000) >> 16);
            byte g = (byte)((dwColor & 0x0000ff00) >> 8);
            byte b = (byte)(dwColor & 0x000000ff);

            COLORREF color = Windows.RGB(r, g, b);
            Windows.SetTextColor(hDC, color);
            fixed (char* strPtr = wchTxt)
            {
                Windows.DrawText(hDC, (ushort*)strPtr, -1, &textRect, DT.DT_LEFT | DT.DT_WORDBREAK);
            }
        }

        unsafe bool GetFontSize(out int piOutWidth, out int piOutHeight, string str, HDC hDC)
        {
            SIZE strSize;
            fixed (char* strPtr = str)
            {
                bool bGetSize = Windows.GetTextExtentPoint32W(hDC, (ushort*)strPtr, str.Length, &strSize);
                if (!bGetSize)
                {
                    piOutWidth = 1;
                    piOutHeight = 1;
                    return false;
                }
            }
            piOutWidth = strSize.cx;
            piOutHeight = strSize.cy;
            return true;
        }
        #endregion // GDI

        // -------------------
        internal bool DrawBitmap(int startX, int startY, int imgWidth, int imgHeight, byte[] imgBytes)
        {
            int2 ivPos = new int2(startX, startY);
            int2 ivImageSize = new int2(imgWidth, imgHeight);

            if (!CalcClipArea(out int2 ivSrcStart, out int2 ivDestStart, out int2 ivDestSize, ivPos, ivImageSize))
            {
                return false;
            }

            fixed (byte* imgBytesPtr = imgBytes)
            {
                byte* pSrc = imgBytesPtr + (ivSrcStart.x + ivSrcStart.y * imgWidth) * 4;
                byte* pDest = m_lockedBackBuffer + (ivDestStart.x * 4) + ivDestStart.y * m_lockedBackBufferPitch;

                for (int y = 0; y < ivDestSize.y; y++)
                {
                    for (int x = 0; x < ivDestSize.x; x++)
                    {
                        *(UInt32*)pDest = *(UInt32*)pSrc;
                        pSrc += 4;
                        pDest += 4;
                    }
                    pSrc -= (ivDestSize.x * 4);
                    pSrc += (imgWidth * 4);
                    pDest -= (ivDestSize.x * 4);
                    pDest += m_lockedBackBufferPitch;
                }
            }
            return true;
        }

        private bool CalcClipArea(out int2 pivOutSrcStart, out int2 pivOutDestStart, out int2 pivOutDestSize, in int2 pivPos, in int2 pivImageSize)
        {
            int2 ivBufferSize = new int2(Width, Height);
            bool isSuccess = Util.CalcClipArea(out pivOutSrcStart, out pivOutDestStart, out pivOutDestSize, pivPos, pivImageSize, ivBufferSize);
            return isSuccess;
        }
        // ---------

        internal bool DrawImageData(in int startX, in int startY, in ImageData imageData)
        {
            int iScreenWidth = Width;

            int iBitmapWidth = imageData.Width;
            int iBitmapHeight = imageData.Height;

            int2 ivPos = new int2(startX, startY);
            int2 ivImageSize = new int2(iBitmapWidth, iBitmapHeight);
            if (!CalcClipArea(out int2 ivSrcStart, out int2 ivDestStart, out int2 ivDestSize, ivPos, ivImageSize))
            {
                return false;
            }

            COMPRESSED_LINE* pLineDesc = imageData.GetCompressedImage(ivSrcStart.y);
            byte* pDestPerLine = m_lockedBackBuffer + (ivDestStart.y) * m_lockedBackBufferPitch;

            for (int y = 0; y < ivDestSize.y; ++y)
            {
                for (int i = 0; i < pLineDesc->pixelStreamCount; ++i)
                {
                    PIXEL_STREAM* pStream = pLineDesc->data.pixelStreamPtr + i;
                    UInt32 dwPixelColor = pStream->color;
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
                    byte* pDest = pDestPerLine + (dest_x * 4);
                    for (int x = 0; x < iPixelNum; x++)
                    {
                        *(UInt32*)pDest = dwPixelColor;
                        pDest += 4;
                    }
                }
                pLineDesc++;
                pDestPerLine += m_lockedBackBufferPitch;
            }
            return true;
        }

    }
}