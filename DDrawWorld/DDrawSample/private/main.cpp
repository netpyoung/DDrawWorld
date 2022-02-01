#include "ddrawsample/precompiled.h"
#include <iostream>

#include "Util/QueryPerfCounter.h"
#include "ddrawsample/CGame.h"
#include "resource.h"

std::unique_ptr<DDrawSample::CGame> g_Game;

constexpr int MAX_LOADSTRING = 100;

::HINSTANCE g_hInst = nullptr;
::HWND      g_mainWindowHwnd = nullptr;

::WCHAR g_titleText[MAX_LOADSTRING];
::WCHAR g_windowClassName[MAX_LOADSTRING];

::ATOM                MyRegisterClass(::HINSTANCE hInstance);
bool                  InitInstance(const ::HINSTANCE hInstance, const int nCmdShow);
::LRESULT CALLBACK    WndProc(::HWND, ::UINT, ::WPARAM, ::LPARAM);
::INT_PTR CALLBACK    About(::HWND, ::UINT, ::WPARAM, ::LPARAM);


int APIENTRY wWinMain(
	_In_::HINSTANCE hInstance,
	_In_opt_::HINSTANCE hPrevInstance,
	_In_::WCHAR* lpCmdLine,
	_In_     int       nCmdShow)
{
	// |               |                                                        |
	// | ------------- | ------------------------------------------------------ |
	// | hInstance     | current instance's handle                              |
	// | hPrevInstance | It was used in 16-bit Windows, but is now always zero. |
	// | pCmdLine      | command-line arguments(Unicode).                       |
	// | nCmdShow      | flag( minimized, maximized, or shown normally )        |
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG

	::LoadStringW(hInstance, IDS_APP_TITLE, g_titleText, MAX_LOADSTRING);
	::LoadStringW(hInstance, IDC_DDrawSample, g_windowClassName, MAX_LOADSTRING);

	if (MyRegisterClass(hInstance) == 0)
	{
		return 100;
	}

	if (!InitInstance(hInstance, nCmdShow))
	{
		return 100;
	}

	::HACCEL hAccelTable = ::LoadAcceleratorsW(hInstance, MAKEINTRESOURCE(IDC_DDrawSample));
	if (hAccelTable == nullptr)
	{
		return 100;
	}

	Util::QueryPerfCounter::QCInit();
	g_Game = std::make_unique<DDrawSample::CGame>();
	if (!g_Game->Initialize(g_mainWindowHwnd))
	{
		return false;
	}

	::MSG msg;
	while (true)
	{
		bool isMessage = ::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE);
		if (isMessage)
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			if (::TranslateAcceleratorW(g_mainWindowHwnd, hAccelTable, &msg))
			{
				continue;
			}

			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else
		{
			g_Game->Process();
		}
	}

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif // _DEBUG

	return static_cast<int>(msg.wParam);
}

::ATOM MyRegisterClass(::HINSTANCE hInstance)
{
	::WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(::WNDCLASSEX);

	// CS_HREDRAW | Redraws the entire window if a movement or size adjustment changes the width of the client area.
	// CS_VREDRAW | Redraws the entire window if a movement or size adjustment changes the height of the client area.
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DDrawSample));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<::HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DDrawSample);
	wcex.lpszClassName = g_windowClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return ::RegisterClassExW(&wcex);
}

bool InitInstance(const ::HINSTANCE hInstance, const int nCmdShow)
{
	g_hInst = hInstance;

	//#define WS_OVERLAPPEDWINDOW (WS_OVERLAPPED     | \
	//                             WS_CAPTION        | \
	//                             WS_SYSMENU        | \
	//                             WS_THICKFRAME     | \
	//                             WS_MINIMIZEBOX    | \
	//                             WS_MAXIMIZEBOX)
	const HWND hWnd = ::CreateWindowExW(
		0,
		g_windowClassName, g_titleText,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		nullptr, nullptr, hInstance, nullptr
	);
	if (hWnd == nullptr)
	{
		return false;
	}

	g_mainWindowHwnd = hWnd;
	::ShowWindow(hWnd, nCmdShow);
	if (::UpdateWindow(hWnd) == 0)
	{
		return false;
	}

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		const WORD wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			::DialogBoxW(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;

	case WM_PAINT:
	{
		::PAINTSTRUCT ps;
		const ::HDC hdc = ::BeginPaint(hWnd, &ps);
		UNREFERENCED_PARAMETER(hdc);
		// TODO: Add any drawing code that uses hdc here...
		::EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	if (g_Game != nullptr)
	{
		switch (message)
		{
		case WM_ACTIVATE:
		{
			g_Game->OnActivate();
		}
		break;
		case WM_SIZE:
		{
			g_Game->OnUpdateWindowSize();
		}
		break;
		case WM_MOVE:
		{
			g_Game->OnUpdateWindowPos();
		}
		break;
		case WM_KEYDOWN:
		{
			// ref: https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
			// 
			// lParams' bit
			// 16-23 | The scan code. The value depends on the OEM.
			// 30	 | The previous key state.The value is 1 if the key is down before the message is sent, or it is zero if the key is up.

			// ref: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mapvirtualkeyw
			// MAPVK_VSC_TO_VK: The uCode parameter is a scan code and is translated into a virtual-key code that does not distinguish between left- and right-hand keys. If there is no translation, the function returns 0.

			const uint32_t uiScanCode = (0x00ff0000 & lParam) >> 16;
			const uint32_t vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
			if (!(lParam & 0x40000000))
			{
				g_Game->OnKeyDown(vkCode, uiScanCode);
			}
		}
		break;
		case WM_KEYUP:
		{
			const uint32_t uiScanCode = (0x00ff0000 & lParam) >> 16;
			const uint32_t vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);
			g_Game->OnKeyUp(vkCode, uiScanCode);
		}
		break;
		case WM_SYSKEYDOWN:
		{
			const uint32_t uiScanCode = (0x00ff0000 & lParam) >> 16;
			const uint32_t vkCode = MapVirtualKey(uiScanCode, MAPVK_VSC_TO_VK);

			/*
			 * WM_KEYUP/DOWN/CHAR HIWORD(lParam) flags
			*/
			//#define KF_EXTENDED       0x0100
			//#define KF_DLGMODE        0x0800
			//#define KF_MENUMODE       0x1000
			//#define KF_ALTDOWN        0x2000
			//#define KF_REPEAT         0x4000
			//#define KF_UP             0x8000
			const bool bAltKeyDown = ((HIWORD(lParam) & KF_ALTDOWN) != 0);
			if (!g_Game->OnSysKeyDown(vkCode, uiScanCode, bAltKeyDown))
			{
				DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
		default:
			break;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return static_cast<INT_PTR>(true);
	case WM_COMMAND:
		const WORD loword = LOWORD(wParam);
		if (loword == IDOK || loword == IDCANCEL)
		{
			::EndDialog(hDlg, loword);
			return static_cast<INT_PTR>(true);
		}
		break;
	}
	return static_cast<INT_PTR>(false);
}