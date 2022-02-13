using DotnetDDrawSample.Core;
using System.Runtime.InteropServices;
using CS = TerraFX.Interop.Windows.CS;
using HDC = TerraFX.Interop.Windows.HDC;
using HINSTANCE = TerraFX.Interop.Windows.HINSTANCE;
using HMENU = TerraFX.Interop.Windows.HMENU;
using HMODULE = TerraFX.Interop.Windows.HMODULE;
using HWND = TerraFX.Interop.Windows.HWND;
using IDC = TerraFX.Interop.Windows.IDC;
using LPARAM = TerraFX.Interop.Windows.LPARAM;
using LRESULT = TerraFX.Interop.Windows.LRESULT;
using MSG = TerraFX.Interop.Windows.MSG;
using PAINTSTRUCT = TerraFX.Interop.Windows.PAINTSTRUCT;
using PM = TerraFX.Interop.Windows.PM;
using SW = TerraFX.Interop.Windows.SW;
using Windows = TerraFX.Interop.Windows.Windows;
using WM = TerraFX.Interop.Windows.WM;
using WNDCLASSEXW = TerraFX.Interop.Windows.WNDCLASSEXW;
using WPARAM = TerraFX.Interop.Windows.WPARAM;
using WS = TerraFX.Interop.Windows.WS;

namespace DotnetDDrawSample.Playground
{
    public unsafe class Program
    {
        public static int Main()
        {
            return new Program().Run();
        }

        static private HWND s_hwnd = HWND.NULL;
        static private Game s_game = new Game();

        public int Run()
        {
            HMODULE hInstance = Windows.GetModuleHandleW(null);
            fixed (char* lpszClassName = "DXSampleClass")
            fixed (char* lpWindowName = "name")
            {
                // Initialize the window class.
                WNDCLASSEXW windowClass = new WNDCLASSEXW
                {
                    cbSize = (uint)sizeof(WNDCLASSEXW),
                    style = CS.CS_HREDRAW | CS.CS_VREDRAW,
                    lpfnWndProc = &WindowProc,
                    hInstance = hInstance,
                    hCursor = Windows.LoadCursorW(HINSTANCE.NULL, (ushort*)IDC.IDC_ARROW),
                    lpszClassName = (ushort*)lpszClassName
                };
                _ = Windows.RegisterClassExW(&windowClass);

                // Create the window and store a handle to it.
                s_hwnd = Windows.CreateWindowExW(
                      0,
                      windowClass.lpszClassName,
                      (ushort*)lpWindowName,
                      WS.WS_OVERLAPPEDWINDOW,
                      Windows.CW_USEDEFAULT,
                      Windows.CW_USEDEFAULT,
                      Windows.CW_USEDEFAULT,
                      Windows.CW_USEDEFAULT,
                      HWND.NULL,                       // We have no parent window.
                      HMENU.NULL,                      // We aren't using menus.
                      hInstance,
                      null
                  );
            }

            int nCmdShow = SW.SW_SHOWDEFAULT;
            _ = Windows.ShowWindow(s_hwnd, nCmdShow);
            _ = Windows.UpdateWindow(s_hwnd);

            if (!s_game.Initialize(s_hwnd))
            {
                return 1;
            }

            MSG msg;
            while (true)
            {
                if (Windows.PeekMessageW(&msg, hWnd: HWND.NULL, wMsgFilterMin: WM.WM_NULL, wMsgFilterMax: WM.WM_NULL, PM.PM_REMOVE) != Windows.FALSE)
                {
                    if (msg.message == WM.WM_QUIT)
                    {
                        break;
                    }
                    _ = Windows.TranslateMessage(&msg);
                    _ = Windows.DispatchMessageW(&msg);
                    continue;
                }

                s_game.Process();
            }
            return (int)msg.wParam;
        }

        [UnmanagedCallersOnly]
        private static LRESULT WindowProc(HWND hWnd, uint message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
                case WM.WM_DESTROY:
                    Windows.PostQuitMessage(0);
                    return 0;
                case WM.WM_KILLFOCUS:
                    if (s_game != null)
                    {
                        s_game.OnLostFocus();
                    }
                    break;
                case WM.WM_SIZE:
                    if (s_game != null)
                    {
                        s_game.OnUpdateWindowSize();
                    }
                    break;
                case WM.WM_MOVE:
                    if (s_game != null)
                    {
                        s_game.OnUpdateWindowPos();
                    }
                    break;
                case WM.WM_KEYDOWN:
                    {
                        uint scanCode = (uint)((lParam & 0x00ff0000) >> 16);
                        uint vkCode = Windows.MapVirtualKey(scanCode, Windows.MAPVK_VSC_TO_VK);
                        if ((lParam & 0x40000000) == 0)
                        {
                            if (s_game != null)
                            {
                                s_game.OnKeyDown(vkCode, scanCode);
                            }
                        }
                    }
                    break;
                case WM.WM_KEYUP:
                    {
                        uint scanCode = (uint)((lParam & 0x00ff0000) >> 16);
                        uint vkCode = Windows.MapVirtualKey(scanCode, Windows.MAPVK_VSC_TO_VK);
                        if (s_game != null)
                        {
                            s_game.OnKeyUp(vkCode, scanCode);
                        }
                    }
                    break;
                case WM.WM_SYSKEYDOWN:
                    {
                        uint scanCode = (uint)((0x00ff0000 & lParam) >> 16);
                        uint vkCode = Windows.MapVirtualKey(scanCode, Windows.MAPVK_VSC_TO_VK);
                        bool isAltKeyDown = ((Windows.HIWORD(lParam) & Windows.KF_ALTDOWN) != 0);

                        if (!s_game.OnSysKeyDown(vkCode, scanCode, isAltKeyDown))
                        {
                            return Windows.DefWindowProc(hWnd, message, wParam, lParam);
                        }
                    }
                    break;
                case WM.WM_PAINT:
                    {
                        PAINTSTRUCT ps;
                        HDC hdc = Windows.BeginPaint(hWnd, &ps);
                        Windows.EndPaint(hWnd, &ps);
                    }
                    break;
                default:
                    return Windows.DefWindowProcW(hWnd, message, wParam, lParam);
            }
            return 0;
        }
    }

}