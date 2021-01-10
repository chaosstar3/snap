#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#ifdef _DEBUG
void dprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buf[256];
	vsprintf_s(buf, fmt, args);
	va_end(args);
	OutputDebugStringA(buf);
}
#else
#define dprintf(fmt, ...)
#endif

void dprintrect(const char* name, LPRECT lpRect) {
	int width = lpRect->right - lpRect->left;
	int height = lpRect->bottom - lpRect->top;

	dprintf("%s %d - %d (%d) %d - %d (%d)", name,
		lpRect->left, lpRect->right, width,
		lpRect->top, lpRect->bottom, height);
}

#define TRACE(err) dprintf("[%s:%d] %d %d", __func__, __LINE__, err, GetLastError())

#define RECT_EQ(r1, r2) memcmp(r1, r2, sizeof(RECT))
#define RECT_EQ2(r1, r2, e1, e2) ((r1)->e1 == (r2)->e1 && (r1)->e2 == (r2)->e2)
#define RECT_EQ3(r1, r2, e1, e2, e3) ((r1)->e1 == (r2)->e1 && (r1)->e2 == (r2)->e2 && (r1)->e3 == (r2)->e3)
#define RECT_WIDTH(r) ((r)->right - (r)->left)
#define RECT_HEIGHT(r) ((r)->bottom - (r)->top)

enum class SNAP_TYPE { SNAP_LEFT, SNAP_RIGHT, SNAP_TOP, SNAP_BOTTOM, SNAP_CENTER, SNAP_FULL, SNAP_NONE };

#define TRACE_SNAP_WINDOW(err) TRACE(err)
void snap_window(SNAP_TYPE type) {
	HWND window = GetForegroundWindow();

	if (!window) {
		TRACE_SNAP_WINDOW(-1);
		return;
	}

	HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

	if (!monitor) {
		TRACE_SNAP_WINDOW(-2);
		return;
	}

	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);

	if (!GetMonitorInfo(monitor, &minfo)) {
		TRACE_SNAP_WINDOW(-3);
		return;
	}

	LPRECT mon_lprect = &minfo.rcWork;
	RECT win_rect, new_rect;

	if (!GetWindowRect(window, &win_rect)) {
		TRACE_SNAP_WINDOW(-4);
	}

	new_rect = *mon_lprect;
	int width = RECT_WIDTH(mon_lprect);
	int height = RECT_HEIGHT(mon_lprect);

	int cw[3] = { width / 2, width / 3, width / 3 * 2 };
	int ch[3] = { height / 2, height / 3, height / 3 * 2 };

	int snap_size_index = -1;

	switch (type) {
	case SNAP_TYPE::SNAP_LEFT:
		if (RECT_EQ3(&win_rect, mon_lprect, left, top, bottom)) {
			for (int i = 0; i < 3; i++) {
				if (RECT_WIDTH(&win_rect) == cw[i]) {
					snap_size_index = i;
					break;
				}
			}
		}

		snap_size_index = (snap_size_index + 1) % 3;
		new_rect.right = mon_lprect->left + cw[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_RIGHT:
		if (RECT_EQ3(&win_rect, mon_lprect, right, top, bottom)) {
			for (int i = 0; i < 3; i++) {
				if (RECT_WIDTH(&win_rect) == cw[i]) {
					snap_size_index = i;
					break;
				}
			}
		}

		snap_size_index = (snap_size_index + 1) % 3;
		new_rect.left = mon_lprect->right - cw[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_TOP:
		if (RECT_EQ3(&win_rect, mon_lprect, left, right, top)) {
			for (int i = 0; i < 3; i++) {
				if (RECT_HEIGHT(&win_rect) == ch[i]) {
					snap_size_index = i;
					break;
				}
			}
		}

		snap_size_index = (snap_size_index + 1) % 3;
		new_rect.bottom = mon_lprect->top + ch[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_BOTTOM:
		if (RECT_EQ3(&win_rect, mon_lprect, left, right, bottom)) {
			for (int i = 0; i < 3; i++) {
				if (RECT_HEIGHT(&win_rect) == ch[i]) {
					snap_size_index = i;
					break;
				}
			}
		}

		snap_size_index = (snap_size_index + 1) % 3;
		new_rect.top = mon_lprect->bottom - ch[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_CENTER:
		if (RECT_WIDTH(mon_lprect) > RECT_HEIGHT(mon_lprect)) {
			// landscape monitor: horizontal center
			int hcenter = (mon_lprect->left + mon_lprect->right) / 2;

			if (RECT_EQ2(&win_rect, mon_lprect, top, bottom)) {
				for (int i = 0; i < 3; i++) {
					int left = hcenter - cw[i] / 2;
					int right = hcenter + cw[i] / 2;

					if (win_rect.left == left && win_rect.right == right) {
						snap_size_index = i;
						break;
					}
				}
			}

			snap_size_index = (snap_size_index + 1) % 3;
			new_rect.left = hcenter - cw[snap_size_index] / 2;
			new_rect.right = hcenter + cw[snap_size_index] / 2;
		} else {
			// portrait monitor: vertical center
			int vcenter = (mon_lprect->top + mon_lprect->bottom) / 2;

			if (RECT_EQ2(&win_rect, mon_lprect, left, right)) {
				for (int i = 0; i < 3; i++) {
					int top = vcenter - ch[i] / 2;
					int bottom = vcenter + ch[i] / 2;

					if (win_rect.top == top && win_rect.bottom == bottom) {
						snap_size_index = i;
						break;
					}
				}

			}

			snap_size_index = (snap_size_index + 1) % 3;
			new_rect.top = vcenter - ch[snap_size_index] / 2;
			new_rect.bottom = vcenter + ch[snap_size_index] / 2;
		}

		break;
	default: // SNAP_FULL same as monitor size
		break;
	}

	dprintrect("monitor", mon_lprect);
	dprintrect("window", &win_rect);
	dprintrect("resize", &new_rect);

	if (!SetWindowPos(window, NULL, new_rect.left, new_rect.top, RECT_WIDTH(&new_rect), RECT_HEIGHT(&new_rect), 0)) {
		TRACE_SNAP_WINDOW(-10);
		return;
	}
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
		bool const ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
		bool const shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		bool const alt = GetAsyncKeyState(VK_MENU) & 0x8000;
		bool const win = GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000;

		KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		//debug("key C%d S%d A%d W%d %x %x", ctrl, shift, alt, win, kbd->vkCode, kbd->scanCode);

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
				snap_window(snap_type);
				return 1;
			}
		}
	}
		
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);

	if (!hook) {
		dprintf("SetWindowsHookEx() failed");
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0));
	return 0;
}
