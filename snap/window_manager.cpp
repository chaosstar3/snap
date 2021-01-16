#include "window_manager.h"
#include "debug.h"

#define RECT_EQ(r1, r2) !memcmp(r1, r2, sizeof(RECT))

void WindowManager::snap_window(SNAP_TYPE type, SNAP_BASE base) {
	HWND window = GetForegroundWindow();

	if (!window) {
		TRACE("GetForegroundWindow() failed");
		return;
	}

	HMONITOR monitor = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);

	if (!monitor) {
		TRACE("MonitorFromWindow() failed");
		return;
	}

	MONITORINFO minfo;
	minfo.cbSize = sizeof(MONITORINFO);

	if (!GetMonitorInfo(monitor, &minfo)) {
		TRACE("GetMonitorInfo() failed");
		return;
	}

	RECT mon_rect, win_rect, cli_rect, margin_rect;
	MGWindow new_window;

	if (!GetWindowRect(window, &win_rect)) {
		TRACE("GetWindowRect() failed");
		return;
	}

	if (!GetClientRect(window, &cli_rect)) {
		TRACE("GetClientRect() failed");
		return;
	}

	mon_rect = minfo.rcWork;

	int width = RECT_WIDTH(&mon_rect);
	int height = RECT_HEIGHT(&mon_rect);
	int margin_width = RECT_WIDTH(&win_rect) - RECT_WIDTH(&cli_rect);

	margin_rect.left = margin_width / 2;
	margin_rect.right = margin_width / 2 - margin_width;
	margin_rect.top = 0;
	margin_rect.bottom = - margin_width / 2;

	int cw[3] = { width / 2, width / 3, width / 3 * 2 };
	int ch[3] = { height / 2, height / 3, height / 3 * 2 };
	int snap_repeat = 0;

	// repeated sequence
	if (last_snap.window == window &&
				last_snap.type == type &&
				last_snap.base == base &&
				RECT_EQ(&last_snap.rect, &win_rect)) {
		snap_repeat = (last_snap.repeat + 1) % 3;
	}

	last_snap.window = window;
	last_snap.type = type;
	last_snap.base = base;
	last_snap.repeat = snap_repeat;

	switch (base) {
	case SNAP_BASE::BY_DIRECTION_ONLY:
		new_window.set(&win_rect);
		break;
	case SNAP_BASE::BY_ENTIRE_MONITOR:
		new_window.set(&mon_rect, &margin_rect);
		break;
	}

	switch (type) {
	case SNAP_TYPE::SNAP_LEFT:
		new_window.set_width(mon_rect.left, cw[snap_repeat], &margin_rect);
		break;
	case SNAP_TYPE::SNAP_RIGHT:
		new_window.set_width(mon_rect.right - cw[snap_repeat], cw[snap_repeat], &margin_rect);
		break;
	case SNAP_TYPE::SNAP_TOP:
		new_window.set_height(mon_rect.top, ch[snap_repeat], &margin_rect);
		break;
	case SNAP_TYPE::SNAP_BOTTOM:
		new_window.set_height(mon_rect.bottom - ch[snap_repeat], ch[snap_repeat], &margin_rect);
		break;
	case SNAP_TYPE::SNAP_CENTER:
		if (RECT_WIDTH(&mon_rect) > RECT_HEIGHT(&mon_rect)) {
			// landscape monitor: horizontal center
			int hcenter = (mon_rect.left + mon_rect.right) / 2;
			new_window.set_width(hcenter - cw[snap_repeat] / 2, cw[snap_repeat], &margin_rect);
		}
		else {
			// portrait monitor: vertical center
			int vcenter = (mon_rect.top + mon_rect.bottom) / 2;
			new_window.set_height(vcenter - ch[snap_repeat] / 2, ch[snap_repeat], &margin_rect);
		}

		break;
	case SNAP_TYPE::SNAP_FULL:
		PostMessage(window, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		return;
		break;
	default:
		return;
		break;
	}

	dprintf("\n");
	dprintrect("mon", &mon_rect);
	dprintrect("win", &win_rect);
	dprintrect("cli", &cli_rect);
	dprintrect("margin", &new_window.margin);
	dprintrect("new", &new_window.rect);

	// ShowWindow(window, SW_SHOWNORMAL) and SetWindowPos()
	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	placement.ptMaxPosition = {-1,-1};
	placement.ptMinPosition = {0,0};
	placement.flags = 0;
	placement.showCmd = SW_SHOWNORMAL; // for not maximize
	placement.rcNormalPosition = new_window.rect;

	if(!SetWindowPlacement(window, &placement)) {
		TRACE("SetWindowPlacement() failed");
		return;
	}

	// between two different DPI monitor, margin causes error in size and position
	// if the result window is not the desired one, retry with margin
	GetWindowRect(window, &win_rect);
	dprintrect("win", &win_rect);

	bool equal = true;

	if (win_rect.left != new_window.rect.left) {
		new_window.rect.left += new_window.margin.left;
		new_window.margin.left = 0;
		equal = false;
	}

	if (win_rect.right != new_window.rect.right) {
		new_window.rect.right += new_window.margin.right;
		new_window.margin.right = 0;
		equal = false;
	}

	if (win_rect.top != new_window.rect.top) {
		new_window.rect.top += new_window.margin.top;
		new_window.margin.top = 0;
		equal = false;
	}

	if (win_rect.bottom != new_window.rect.bottom) {
		new_window.rect.bottom += new_window.margin.bottom;
		new_window.margin.bottom = 0;
		equal = false;
	}

	if (!equal) {
		dprintrect("retry", &new_window.rect);
		SetWindowPos(window, NULL, new_window.x(), new_window.y(), new_window.width(), new_window.height(), 0);
		GetWindowRect(window, &win_rect);
	}

	last_snap.rect = win_rect;
};
