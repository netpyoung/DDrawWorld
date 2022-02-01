## PixelHorizon

- [megayuchi: DirectDraw 프로그래밍 소개](https://www.youtube.com/watch?v=J0MNKUYw1zY)
  - [github: megayuchi/PixelHorizon](https://github.com/megayuchi/PixelHorizon)
- <https://blog.naver.com/stweed/30013157926>

### DirectDraw

- 개념공부
  - bitmap 
  - 화면 출력
  - 게임루프

ddraw
키입력
interpolation


0번압축 - 비교할 키 칼라(투명효과를 나타내기 위한 색, 보통 0,0 좌표)를 압축
CTGAImage에서 `w * h * 4byte`로 4byte(rgba - 32bit)컬러로 저장하고
CImageData 에서 
  비교할 키 칼라와 같은 색은 건너 뛰면서, 다른 색을 마주치면 PIXEL_STREAM을 만듬

|          |                                                      |
| -------- | ---------------------------------------------------: |
| 개발시   |                     로드(tga)     -> 0번압축 -> 출력 |
| 릴리즈시 | 사전 0번압축빌드 -> 로드(0번압축) ->            출력 |

``` txt
colorkey | 0x00000000
rgba     | 0x00000000 0x00000000 0xff000000 0xff000000 0x00ff0000 0x00000000
index    | 0          1          2          3          4


#pragma pack(push,4)
      // 8 = 2/2/4
      struct PIXEL_STREAM
      {
        uint16_t posX;
        uint16_t sameColorCount;
        uint32_t color;
      };
      static_assert(sizeof(PIXEL_STREAM) == 8);
#pragma pack(pop)


pixelStreams[0].posX           = 2
pixelStreams[0].color          = 0xff000000
pixelStreams[0].sameColorCount = 2

pixelStreams[1].posX           = 4
pixelStreams[1].color          = 0x00ff0000
pixelStreams[1].sameColorCount = 1


4byte(color)        * 6개 = 24 byte가
8byte(piexelStream) * 2개 = 16 byte로 압축됨
```

- [wiki: TGA](https://en.wikipedia.org/wiki/Truevision_TGA)
- [wiki: Run-Length Encoding](https://en.wikipedia.org/wiki/Run-length_encoding)

``` txt
// RLE example
AAABAAACC => 3A1B3A2C

| AAA | B  | AAA | CC |
| 3A  | 1B | 3A  | 2C |
```

https://www.gamers.org/dEngine/quake3/TGA.txt
https://github.com/image-rs/image/tree/master/tests/images/tga/testsuite
https://github.com/megayuchi/PixelHorizon/blob/master/Util/TGAImage.h

## Premake

``` cmd
premake5  vs2019
```

초기 윈도우 메시지
장치초기화
입력
플레이어 출력
총알관리
적 관리

``` cpp
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
::GetClientRect(m_hWnd, outRect);
::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->left));
::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->right));

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
```



``` cpp
void CGame::DrawScene()
{
    if (m_ddraw->BeginDraw())
    {
        // ----- DDSURFACEDESC2 ddsc;
        // ----- memset(&ddsc, 0, sizeof(DDSURFACEDESC2));
        // ----- ddsc.dwSize = sizeof(DDSURFACEDESC2);
        // ----- m_ddBack->Lock(nullptr, &ddsc, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr);

        m_ddraw->Clear();
        // ----- const int bytesPerLine = 4 * m_width;
        // ----- for (int y = 0; y < m_height; ++y)
        // ----- {
        // -----   memset(m_lockedBackBuffer + y * m_lockedBackBufferPitch, 0, bytesPerLine);
        // ----- }

        /// [=== draw someting here ===]

        m_ddraw->EndDraw();         // m_ddBack->Unlock(nullptr);
    }

    ::HDC hdc = nullptr;
    if (m_ddraw->BeginGDI(&hdc)) 		// (m_ddBack->GetDC(&hdc) != DD_OK)
    {
        m_ddraw->DrawInfo(hdc);
        m_ddraw->EndGDI(hdc);       // m_ddBack->ReleaseDC(hdc);
    }

    m_ddraw->OnDraw();              // m_ddPrimary->Blt(&m_rectWindow, m_ddBack, nullptr, DDBLT_WAIT, nullptr);
    m_ddraw->CheckFPS();
}
```


## debug

``` cpp
#include <assert.h>
assert(expression);

__debugbreak(); // == DebugBreak();
_CrtDbgBreak();
```

|                |                     | etc       |
| -------------- | ------------------- | --------- |
| assert()       | #include <assert.h> | _DEBUG    |
| _CrtDbgBreak() | #include <crtdbg.h> | _DEBUG    |
| __debugbreak() | symbol              | asm int 3 |