#include <Windows.h>
#include <tchar.h>
#include "debug.h"
#include "window_manager.h"

static class WindowManager wm;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
		bool const ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
		bool const shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		bool const alt = GetAsyncKeyState(VK_MENU) & 0x8000;
		bool const win = GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000;

		KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		//dprintf("key C%d S%d A%d W%d %x %x", ctrl, shift, alt, win, kbd->vkCode, kbd->scanCode);

		// FIXME: Temporary quit
		if (ctrl && alt && win && kbd->vkCode == 0x26) {
			PostMessage(NULL, WM_QUIT, 0, 0);
		}

		enum SNAP_TYPE snap_type = SNAP_TYPE::SNAP_NONE;

		if (win && !ctrl && !alt) {
			if (shift) {
				switch (kbd->vkCode) {
				case 0x26: // up
					snap_type = SNAP_TYPE::SNAP_FULL;
					break;
				case 0x28: // down
					snap_type = SNAP_TYPE::SNAP_CENTER;
					break;
				}
			}	else {
				switch (kbd->vkCode) {
				case 0x25: // left
					snap_type = SNAP_TYPE::SNAP_LEFT;
					break;
				case 0x26: // up
					snap_type = SNAP_TYPE::SNAP_TOP;
					break;
				case 0x27: // right
					snap_type = SNAP_TYPE::SNAP_RIGHT;
					break;
				case 0x28: // down
					snap_type = SNAP_TYPE::SNAP_BOTTOM;
					break;
				}
			}

			if (snap_type != SNAP_TYPE::SNAP_NONE) {
				wm.snap_window(snap_type);
				return 1;
			}
		}
	}
		
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
	{
		return -2;
	}

	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);

	if (!hook) {
		dprintf("SetWindowsHookEx() failed");
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0));
	return 0;
}
