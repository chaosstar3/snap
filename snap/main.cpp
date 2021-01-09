#include <Windows.h>
#include <tchar.h>
#include <stdio.h>

#ifdef _DEBUG
#define debug(fmt, ...) dprintf(fmt, __VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

void dprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buf[1024];
	vsprintf_s(buf, fmt, args);
	va_end(args);
	OutputDebugStringA(buf);
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
		bool const ctrl = GetAsyncKeyState(VK_CONTROL) & 0x8000;
		bool const shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
		bool const alt = GetAsyncKeyState(VK_MENU) & 0x8000;
		bool const win = GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000;

		KBDLLHOOKSTRUCT* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
		debug("key C%d S%d A%d W%d %x %x", ctrl, shift, alt, win, kbd->vkCode, kbd->scanCode);

		// Temporary quit
		if (ctrl && kbd->vkCode == 'C') {
			PostMessage(NULL, WM_QUIT, 0, 0);
		}
	}
		
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow) {
	HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);

	if (!hook) {
		debug("SetWindowsHookEx() failed");
		return -1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0));
	return 0;
}
