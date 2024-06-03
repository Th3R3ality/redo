// redo.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "redo.h"

// window managing headers
#include "engine/window.h"
#include "engine/directx.h"

// Direct3D 12 headers
#include "include/directx/d3dx12.h"

#include <d3d12.h>
#pragma comment (lib, "d3d12.lib")

#include <d3d12sdklayers.h>

#define MAX_LOADSTRING 100


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    engine::window::RegisterWindowClass(hInstance);

    if (!engine::window::InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    engine::directx::Init();

    // Main application loop:
    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                break;
        }

        engine::directx::Render();
    }
    engine::directx::Destroy();

    return (int) msg.wParam;
}