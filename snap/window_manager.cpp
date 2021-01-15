#include "window_manager.h"
#include "debug.h"

#define RECT_EQ(r1, r2) !memcmp(r1, r2, sizeof(RECT))
#define RECT_WIDTH(r) ((r)->right - (r)->left)
#define RECT_HEIGHT(r) ((r)->bottom - (r)->top)

void WindowManager::snap_window(SNAP_TYPE type) {
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

	RECT mon_rect, win_rect, cli_rect, new_rect;

	if (!GetWindowRect(window, &win_rect)) {
		TRACE("GetWindowRect() failed");
		return;
	}

	if (!GetClientRect(window, &cli_rect)) {
		TRACE("GetClientRect() failed");
		return;
	}

	mon_rect = minfo.rcWork;
	new_rect = mon_rect;

	int width = RECT_WIDTH(&mon_rect);
	int height = RECT_HEIGHT(&mon_rect);

	int cw[3] = { width / 2, width / 3, width / 3 * 2 };
	int ch[3] = { height / 2, height / 3, height / 3 * 2 };

	int snap_size_index = 0;

	// repeated sequence
	if (last_snap.window == window &&
				last_snap.type == type &&
				RECT_EQ(&last_snap.rect, &win_rect)) {
		snap_size_index = (last_snap.index + 1) % 3;
	}

	switch (type) {
	case SNAP_TYPE::SNAP_LEFT:
		new_rect.right = mon_rect.left + cw[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_RIGHT:
		new_rect.left = mon_rect.right - cw[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_TOP:
		new_rect.bottom = mon_rect.top + ch[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_BOTTOM:
		new_rect.top = mon_rect.bottom - ch[snap_size_index];
		break;
	case SNAP_TYPE::SNAP_CENTER:
		if (RECT_WIDTH(&mon_rect) > RECT_HEIGHT(&mon_rect)) {
			// landscape monitor: horizontal center
			int hcenter = (mon_rect.left + mon_rect.right) / 2;
			new_rect.left = hcenter - cw[snap_size_index] / 2;
			new_rect.right = hcenter + cw[snap_size_index] / 2;
		}
		else {
			// portrait monitor: vertical center
			int vcenter = (mon_rect.top + mon_rect.bottom) / 2;
			new_rect.top = vcenter - ch[snap_size_index] / 2;
			new_rect.bottom = vcenter + ch[snap_size_index] / 2;
		}

		break;
	default: // SNAP_FULL same as monitor size
		break;
	}

	// adjust client margin
	int margin = RECT_WIDTH(&win_rect) - RECT_WIDTH(&cli_rect);
	new_rect.left -= margin / 2;
	new_rect.right += margin - margin / 2;
	new_rect.bottom += margin / 2;
	dprintf("margin %d", margin);
	dprintrect("mon", &mon_rect);
	dprintrect("win", &win_rect);
	dprintrect("new", &new_rect);

	if (!SetWindowPos(window, NULL, new_rect.left, new_rect.top, RECT_WIDTH(&new_rect), RECT_HEIGHT(&new_rect), 0)) {
		TRACE("SetWindowPos() failed");
		return;
	}

	// between two different DPI monitor, margin causes error in size and position
	// if the result window is not the desired one, retry SetWindowPos with margin
	GetWindowRect(window, &win_rect);
	dprintrect("win", &win_rect);

	if (!RECT_EQ(&win_rect, &new_rect)) {
		dprintf("Retry with margin");
		new_rect.left += margin / 2;
		new_rect.right -= margin - margin / 2;
		SetWindowPos(window, NULL, new_rect.left, new_rect.top, RECT_WIDTH(&new_rect), RECT_HEIGHT(&new_rect), 0);
		GetWindowRect(window, &win_rect);
	}

	last_snap.window = window;
	last_snap.type = type;
	last_snap.index = snap_size_index;
	last_snap.rect = win_rect;
};
