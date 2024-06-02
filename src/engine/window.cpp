#include "window.h"


namespace engine
{
    namespace window
    {
        HINSTANCE g_hInstance;
        HWND g_hwnd;
        LPCWCHAR g_szTitle = L"Title";
        LPCWCHAR g_szWindowClass = L"windowklass";

        Rect g_windowRect = {100, 100, 800, 600};

	    Rect GetWindowRect()
	    {
            return g_windowRect;
	    }

	    HWND GetHwnd()
	    {
            return g_hwnd;
	    }

        ATOM RegisterMainWindowClass(HINSTANCE hInstance)
        {
            WNDCLASSEXW wcex;

            wcex.cbSize = sizeof(WNDCLASSEX);

            wcex.style = CS_HREDRAW | CS_VREDRAW;
            wcex.lpfnWndProc = WndProc;
            wcex.cbClsExtra = 0;
            wcex.cbWndExtra = 0;
            wcex.hInstance = hInstance;
            wcex.hIcon = NULL;
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            wcex.lpszMenuName = NULL;
            wcex.lpszClassName = g_szWindowClass;
            wcex.hIconSm = NULL;

            return RegisterClassExW(&wcex);
        }


        BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
        {
            g_hInstance = hInstance; // Store instance handle in our global variable

            HWND hwnd = CreateWindowExW(
                NULL,
                g_szWindowClass,
                g_szTitle,
                WS_OVERLAPPEDWINDOW,    // window style
                g_windowRect.x,    // x-position of the window
                g_windowRect.y,    // y-position of the window
                g_windowRect.w,    // width of the window
                g_windowRect.h,    // height of the window
                NULL,    // we have no parent window, NULL
                NULL,    // we aren't using menus, NULL
                hInstance,    // application handle
                NULL);    // used with multiple windows, NULL

            if (!hwnd)
            {
                return FALSE;
            }

            g_hwnd = hwnd;

            ShowWindow(hwnd, nCmdShow);
            UpdateWindow(hwnd);

            return TRUE;
        }

        LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            switch (message)
            {
            case WM_DESTROY:
                PostQuitMessage(0);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
            return 0;
        }

    }
}