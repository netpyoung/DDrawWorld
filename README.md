# DDrawWorld

- megayuchi님의 PixelHorizon베이스로 코드 가지고 놀기
  - [megayuchi: DirectDraw 프로그래밍 소개](https://www.youtube.com/watch?v=J0MNKUYw1zY)
  - [github: megayuchi/PixelHorizon](https://github.com/megayuchi/PixelHorizon)

## overview

- `DDrawWorld/`
  - [premake5](https://premake.github.io/) 적용
  - dll([Implicit linking](https://docs.microsoft.com/en-us/cpp/build/linking-an-executable-to-a-dll)) 적용
  - [precompiled header](https://docs.microsoft.com/en-us/cpp/build/creating-precompiled-header-files) 적용
- `DotnetDDrawSample/`
  - interop를 위한 [terrafx.interop.windows](https://github.com/terrafx/terrafx.interop.windows) 적용

## 참고

### 0번압축

- 비교할 키 칼라(투명효과를 나타내기 위한 색, 보통 첫번째 픽셀 값 x:0, y:0)를 압축
- [wiki: TGA](https://en.wikipedia.org/wiki/Truevision_TGA)
  - https://www.gamers.org/dEngine/quake3/TGA.txt
  - tga도 RLE인코딩 옵션이 있다. [wiki: Run-Length Encoding](https://en.wikipedia.org/wiki/Run-length_encoding)
  ``` txt
  // RLE example
  AAABAAACC => 3A1B3A2C

  | AAA | B  | AAA | CC |
  | 3A  | 1B | 3A  | 2C |
  ```
- RLE와 유사하나, 여기서 말하는 0번압축은 투명 색 하나(0번째)를 건너뛰고 저장하기에 투명이 많은 이미치를 처리하기에 더 효율적이다.

#### 압축과정

- `CTGAImage`에서 tga파일을 로드하여 `w * h * 4byte`로 4byte(rgba - 32bit)컬러로 저장하고
- `CImageData`에서 비교할 키 칼라와 같은 색은 건너 뛰면서, 다른 색을 마주치면 PIXEL_STREAM을 만듬

``` cpp
// ex) 압축 예
// 
// colorkey | 0x00000000
// rgba     | 0x00000000 0x00000000 | 0xff000000 0xff000000 | 0x00ff0000 | 0x00000000
// index    | 0          1          | 2          3          | 4          | 5


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


// colorkey:0x00000000 은 무시

pixelStreams[0].posX           = 2
pixelStreams[0].color          = 0xff000000
pixelStreams[0].sameColorCount = 2

pixelStreams[1].posX           = 4
pixelStreams[1].color          = 0x00ff0000
pixelStreams[1].sameColorCount = 1

// 4byte(color)        * 6개 = 24 byte가
// 8byte(piexelStream) * 2개 = 16 byte로 압축됨


struct COMPRESSED_LINE
{
    DWORD dwStreamNum;
    DWORD dwLineDataSize;
    union
    {
        struct { DWORD         dwOffset;     }; // offset은 pPixelStream을 얻기위한 중간데이터
        struct { PIXEL_STREAM* pPixelStream; }; // compressedImageBytes는 최종적으로 pPixelStream 즉 주소를 들고 있게 됨.
    };
};

COMPRESSED_LINE compressedLine;
compressedLine.dwStreamNum     = 2;
compressedLine.dwLineDataSize  = sizeof(PIXEL_STREAM) * 2;
compressedLine.dwOffset        = 6;                        // rgba값 6개 읽었으니

// compressedImageBytes 미리 메모리 잡아 놓고
compressedLine.pPixelStream = pixelStreams + 0;            // 라인이 시작하는 첫번째

// 그런 다음 그대로 저장.
// compressedImageBytes: [COMPRESSED_LINE * imgHeight | PIXEL_STREAM.... ]
```

#### 참고

- [Voxel의 컬러테이블 압축](https://megayuchi.com/2017/06/15/gamedev-voxel%EC%9D%98-%EC%BB%AC%EB%9F%AC%ED%85%8C%EC%9D%B4%EB%B8%94-%EC%95%95%EC%B6%95/)
- [Voxel Horizon – 백그라운드 컬러 테이블 RLE압축 in Server](https://megayuchi.com/2017/06/16/gamedev-voxel-horizon-%EB%B0%B1%EA%B7%B8%EB%9D%BC%EC%9A%B4%EB%93%9C-%EC%BB%A4%EB%9F%AC-%ED%85%8C%EC%9D%B4%EB%B8%94-rle%EC%95%95%EC%B6%95-in-server/)
- [slideshare: Voxel기반 네트워크 최적화기법](https://www.slideshare.net/dgtman/voxel-based-gameoptimazationrelelase)

### ddraw.h 초기화

``` cpp
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

DDSURFACEDESC2 ddsd = { 0, };
ddsd.dwSize         = sizeof(DDSURFACEDESC2);
ddsd.dwFlags        = DDSD_CAPS;
ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
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

// ::GetClientRect(m_hWnd, outRect);
// ::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->left));
// ::ClientToScreen(m_hWnd, reinterpret_cast<POINT*>(&outRect->right));

DDSURFACEDESC2 ddsd = { 0, };
ddsd.dwSize         = sizeof(DDSURFACEDESC2);
ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
ddsd.dwWidth        = width;
ddsd.dwHeight       = height;

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
        // -----     memset(m_lockedBackBuffer + y * m_lockedBackBufferPitch, 0, bytesPerLine);
        // ----- }

        /// [=== draw someting here ===]

        m_ddraw->EndDraw();         // m_ddBack->Unlock(nullptr);
    }

    ::HDC hdc = nullptr;
    if (m_ddraw->BeginGDI(&hdc))    // (m_ddBack->GetDC(&hdc) != DD_OK)
    {
        m_ddraw->DrawInfo(hdc);
        m_ddraw->EndGDI(hdc);       // m_ddBack->ReleaseDC(hdc);
    }

    m_ddraw->OnDraw();              // m_ddPrimary->Blt(&m_rectWindow, m_ddBack, nullptr, DDBLT_WAIT, nullptr);
    m_ddraw->CheckFPS();
}
```

- ddraw 추가 참고 : <https://blog.naver.com/stweed/30013157926>

### debug

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