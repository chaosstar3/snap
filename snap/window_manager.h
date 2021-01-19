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

	void set(LPRECT rect) {
		this->rect = *rect;
	}

	void set(LPRECT rect, LPRECT border) {
		this->rect.left = rect->left - border->left;
		this->border.left = border->left;
		this->rect.right = rect->right - border->right;
		this->border.right = border->right;
		this->rect.top = rect->top - border->top;
		this->border.top = border->top;
		this->rect.bottom = rect->bottom - border->bottom;
		this->border.bottom = border->bottom;
	}

	void set_width(LONG left, LONG width, LPRECT border) {
		rect.left = left - border->left;
		this->border.left = border->left;
		rect.right = left + width - border->right;
		this->border.right = border->right;
	}

	void set_height(LONG top, LONG height, LPRECT border) {
		rect.top = top - border->top;
		this->border.top = border->top;
		rect.bottom = top + height - border->bottom;
		this->border.bottom = border->bottom;
	}

	LONG x() {
		return rect.left;
	}

	LONG y() {
		return rect.top;
	}

	LONG width() {
		return RECT_WIDTH(&rect);
	}

	LONG height() {
		return RECT_HEIGHT(&rect);
	}
};

class WindowManager {
public:
	struct SnapInfo last_snap;

	void snap_window(SNAP_TYPE, SNAP_BASE);
};
