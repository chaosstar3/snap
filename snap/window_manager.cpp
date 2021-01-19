#include "window_manager.h"
#include "debug.h"

#define RECT_EQ(r1, r2) !memcmp(r1, r2, sizeof(RECT))

void WindowManager::snap_window(SNAP_TYPE type, SNAP_BASE base) {
	HWND window = GetForegroundWindow();

	if(!window) {
		TRACE("GetForegroundWindow() failed");
		return;
	}

	// SNAP_FULL use maximize
	if(type == SNAP_TYPE::SNAP_FULL) {
		PostMessage(window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		return;
	}

	if(SetThreadDpiAwarenessContext(GetWindowDpiAwarenessContext(window)) == NULL) {
		TRACE("DpiAwareness failed");
		return;
	}

	HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);

	if(!monitor) {
		TRACE("MonitorFromWindow() failed");
		return;
	}

	if(!GetMonitorInfo(monitor, &minfo)) {
		TRACE("GetMonitorInfo() failed");
		return;
	}

	WINDOWINFO winfo;
	winfo.cbSize = sizeof(WINDOWINFO);

	if(!GetWindowInfo(window, &winfo)) {
		TRACE("GetWindowInfo() failed");
		return;
	}

	RECT border;
	border.left = min(winfo.rcClient.left - winfo.rcWindow.left, (LONG)winfo.cxWindowBorders);
	border.right = min(winfo.rcWindow.right - winfo.rcClient.right, (LONG)winfo.cxWindowBorders);
	border.top = 0;
	border.bottom = min(winfo.rcWindow.bottom - winfo.rcClient.bottom, (LONG)winfo.cyWindowBorders);


	int width = RECT_WIDTH(&minfo.rcWork);
	int height = RECT_HEIGHT(&minfo.rcWork);
	int cw[3] = { width / 2, width / 3, width / 3 * 2 };
	int ch[3] = { height / 2, height / 3, height / 3 * 2 };
	int snap_repeat = 0;

	// repeated sequence
	if(last_snap.window == window &&
				last_snap.type == type &&
				last_snap.base == base &&
				RECT_EQ(&last_snap.rect, &winfo.rcWindow)) {
		snap_repeat = (last_snap.repeat + 1) % 3;
	}

	last_snap.window = window;
	last_snap.type = type;
	last_snap.base = base;
	last_snap.repeat = snap_repeat;


	Window new_window;
	LPRECT mon_lprect = &minfo.rcWork;
	LPRECT win_lprect = &winfo.rcWindow;

	switch(base) {
	case SNAP_BASE::BY_DIRECTION_ONLY:
		new_window.init(win_lprect, &border, false);
		break;
	case SNAP_BASE::BY_ENTIRE_MONITOR:
		new_window.init(mon_lprect, &border, true);
		break;
	}

	switch(type) {
	case SNAP_TYPE::SNAP_LEFT:
		new_window.set_width(mon_lprect->left, cw[snap_repeat]);
		break;
	case SNAP_TYPE::SNAP_RIGHT:
		new_window.set_width(mon_lprect->right - cw[snap_repeat], cw[snap_repeat]);
		break;
	case SNAP_TYPE::SNAP_TOP:
		new_window.set_height(mon_lprect->top, ch[snap_repeat]);
		break;
	case SNAP_TYPE::SNAP_BOTTOM:
		new_window.set_height(mon_lprect->bottom - ch[snap_repeat], ch[snap_repeat]);
		break;
	case SNAP_TYPE::SNAP_CENTER:
		if (RECT_WIDTH(mon_lprect) > RECT_HEIGHT(mon_lprect)) {
			// landscape monitor: horizontal center
			int hcenter = (mon_lprect->left + mon_lprect->right) / 2;
			new_window.set_width(hcenter - cw[snap_repeat] / 2, cw[snap_repeat]);
		}
		else {
			// portrait monitor: vertical center
			int vcenter = (mon_lprect->top + mon_lprect->bottom) / 2;
			new_window.set_height(vcenter - ch[snap_repeat] / 2, ch[snap_repeat]);
		}

		break;
	default:
		return;
		break;
	}

	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	placement.ptMaxPosition = {-1,-1};
	placement.ptMinPosition = {0,0};
	placement.flags = 0;
	placement.showCmd = SW_SHOWNORMAL; // for not maximize
	new_window.get_rect(&placement.rcNormalPosition);

	if(!SetWindowPlacement(window, &placement)) {
		TRACE("SetWindowPlacement() failed");
		return;
	}

	dprintf("\n[SNAP]");
	dprintf("border %d %d %d %d, %d %d",
		winfo.rcClient.left - winfo.rcWindow.left,
		winfo.rcClient.right - winfo.rcWindow.right,
		winfo.rcClient.top - winfo.rcWindow.top,
		winfo.rcClient.bottom - winfo.rcWindow.bottom,
		winfo.cxWindowBorders, winfo.cyWindowBorders
	);
	dprintrect("mon", mon_lprect);
	dprintrect("win", &winfo.rcWindow);
	dprintrect("new", &new_window.rect);
	dprintf("%d %d", new_window.isborder.width, new_window.isborder.height);

	GetWindowRect(window, &last_snap.rect);
	dprintrect("win", &last_snap.rect);
};
