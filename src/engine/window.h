#pragma once
#include <Windows.h>
#include "types.h"

namespace engine
{
	namespace window
	{
		ATOM                RegisterWindowClass(HINSTANCE hInstance);
		BOOL                InitInstance(HINSTANCE, int);
		LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

		Rect GetWindowRect();
		HWND GetHwnd();
	}
}