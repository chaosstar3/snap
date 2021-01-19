#pragma once
#include <Windows.h>

#define RECT_WIDTH(r) ((r)->right - (r)->left)
#define RECT_HEIGHT(r) ((r)->bottom - (r)->top)

enum class SNAP_TYPE {
	SNAP_LEFT, SNAP_RIGHT, SNAP_TOP, SNAP_BOTTOM,
	SNAP_CENTER, SNAP_FULL, SNAP_NONE
};

enum class SNAP_BASE {
	BY_ENTIRE_MONITOR, BY_DIRECTION_ONLY
};

struct SnapInfo {
	HWND window;
	SNAP_TYPE type;
	SNAP_BASE base;
	int repeat;
	RECT rect;
};

// window with border
struct Window {
public:
	RECT rect;
	RECT border;
	// border is applies or not
	struct {
		bool width;
		bool height;
	} isborder;

	void init(LPRECT rect, LPRECT border, bool isborder) {
		this->rect = *rect;
		this->border = *border;
		this->isborder.width = isborder;
		this->isborder.height = isborder;
	}

	void set_width(LONG left, LONG width) {
		rect.left = left;
		rect.right = left + width;
		isborder.width = true;
	}

	void set_height(LONG top, LONG height) {
		rect.top = top;
		rect.bottom = top + height;
		isborder.height = true;
	}

	LONG x() {
		LONG adjust = isborder.width ? border.left : 0;
		return rect.left - adjust;
	}

	LONG y() {
		LONG adjust = isborder.height ? border.top : 0;
		return rect.top - adjust;
	}

	LONG width() {
		LONG adjust = isborder.width ? border.left + border.right : 0;
		return RECT_WIDTH(&rect) + adjust;
	}

	LONG height() {
		LONG adjust = isborder.height ? border.top + border.bottom : 0;
		return RECT_HEIGHT(&rect) + adjust;
	}

	void get_rect(LPRECT out) {
		if(isborder.width) {
			out->left = rect.left - border.left;
			out->right = rect.right + border.right;
		} else {
			out->left = rect.left;
			out->right = rect.right;
		}

		if(isborder.height) {
			out->top = rect.top - border.top;
			out->bottom = rect.bottom + border.bottom;
		} else {
			out->top = rect.top;
			out->bottom = rect.bottom;
		}
	}
};

class WindowManager {
public:
	struct SnapInfo last_snap;

	void snap_window(SNAP_TYPE, SNAP_BASE);
};
