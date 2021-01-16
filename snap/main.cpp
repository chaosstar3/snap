#include <Windows.h>
#include <shellapi.h>
#include <tchar.h>
#include "debug.h"
#include "window_manager.h"

static class WindowManager wm;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
		bool const ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
		bool const shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		bool const alt = GetAsyncKeyState(VK_MENU) & 0x8000;
		bool const win = GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000;

		KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		//dprintf("key C%d S%d A%d W%d %x %x %x", ctrl, shift, alt, win, kbd->vkCode, kbd->scanCode, kbd->flags);

		SNAP_TYPE snap_type = SNAP_TYPE::SNAP_NONE;

		if (win && !ctrl) {
			if (shift) {
				switch (kbd->vkCode) {
				case VK_UP: // up
					snap_type = SNAP_TYPE::SNAP_FULL;
					break;
				case VK_DOWN: // down
					snap_type = SNAP_TYPE::SNAP_CENTER;
					break;
				}

				if (snap_type != SNAP_TYPE::SNAP_NONE) {
					wm.snap_window(snap_type, SNAP_BASE::BY_ENTIRE_MONITOR);
					return 1;
				}
			} else {
				switch (kbd->vkCode) {
				case VK_LEFT: // left
					snap_type = SNAP_TYPE::SNAP_LEFT;
					break;
				case VK_UP: // up
					snap_type = SNAP_TYPE::SNAP_TOP;
					break;
				case VK_RIGHT: // right
					snap_type = SNAP_TYPE::SNAP_RIGHT;
					break;
				case VK_DOWN: // down
					snap_type = SNAP_TYPE::SNAP_BOTTOM;
					break;
				}

				if (snap_type != SNAP_TYPE::SNAP_NONE) {
					wm.snap_window(snap_type,
						alt ? SNAP_BASE::BY_DIRECTION_ONLY : SNAP_BASE::BY_ENTIRE_MONITOR);
					return 1;
				}
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#define WM_TRAYICON (WM_USER+3)
#define ID_TRAY_POPUP_QUIT (1000)

static HMENU menu;
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	UINT select;

	switch (msg) {
	case WM_CREATE:
		menu = CreatePopupMenu();
		AppendMenu(menu, MF_ENABLED | MF_STRING, ID_TRAY_POPUP_QUIT, _T("Quit"));
		break;
	case WM_TRAYICON:
		if (lParam == WM_RBUTTONDOWN) {
			POINT cursor;
			GetCursorPos(&cursor);
			SetForegroundWindow(hwnd); // Remark: TrackPopupMenu

			select = TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, cursor.x, cursor.y, 0, hwnd, NULL);

			switch (select) {
			case ID_TRAY_POPUP_QUIT:
				PostMessage(NULL, WM_QUIT, 0, 0);
				break;
			}
		}

		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

const TCHAR* APPNAME = _T("SNAP");

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	// duplicate execution prevention
	HANDLE mutex = CreateMutex(NULL, TRUE, APPNAME);

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		TRACE("already running");
		return 1;
	}

	if (!SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
	{
		TRACE("SetProcessDpiAwarenessContext() failed");
		return -1;
	}


	// create main Window
	WNDCLASS wc = { 0, };
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = APPNAME;

	if (!RegisterClass(&wc)) {
		TRACE("RegisterClass() failed");
		return -1;
	}

	HWND hwnd = CreateWindow(APPNAME, _T(""), 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

	if (!hwnd) {
		TRACE("CreateWindow() failed");
		return -1;
	}


	// create trayicon
	HICON icon = LoadIcon(LoadLibrary(_T("imageres.dll")), MAKEINTRESOURCE(5308));

	if (!icon) {
		TRACE("icon not found");
		icon = LoadIcon(NULL, IDI_APPLICATION);
	}

	NOTIFYICONDATA tray;
	tray.cbSize = sizeof(NOTIFYICONDATA);
	tray.hWnd = hwnd;
	tray.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	_tcscpy_s(tray.szTip, _T("Snap"));
	tray.hIcon = icon;
	tray.uCallbackMessage = WM_TRAYICON;

	if (!Shell_NotifyIcon(NIM_ADD, &tray)) {
		TRACE("Shell_NotifyIcon() failed");
		return -1;
	}


	// set keyboard hook
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);

	if (!hook) {
		TRACE("SetWindowsHookEx() failed");
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0));

	// Quit
	Shell_NotifyIcon(NIM_DELETE, &tray);
	ReleaseMutex(&mutex);
	CloseHandle(&mutex);
	return 0;
}
