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

// window with margin
class MGWindow {
public:
	RECT rect;
	RECT margin;

	MGWindow() {
		rect = {0,0,0,0};
		margin = {0,0,0,0};
	}

	MGWindow(LPRECT rect) {
		this->rect = *rect;
		margin = {0,0,0,0};
	}

	void set(LPRECT rect) {
		this->rect = *rect;
	}

	void set(LPRECT rect, LPRECT margin) {
		this->rect.left = rect->left - margin->left;
		this->margin.left = margin->left;
		this->rect.right = rect->right - margin->right;
		this->margin.right = margin->right;
		this->rect.top = rect->top - margin->top;
		this->margin.top = margin->top;
		this->rect.bottom = rect->bottom - margin->bottom;
		this->margin.bottom = margin->bottom;
	}

	void set_margin(LPRECT margin) {
		rect.left -= margin->left;
		this->margin.left = margin->left;
		rect.right -= margin->right;
		this->margin.right = margin->right;
		rect.top -= margin->top;
		this->margin.top = margin->top;
		rect.bottom -= margin->bottom;
		this->margin.bottom = margin->bottom;
	}

	void set_width(LONG left, LONG width, LPRECT margin) {
		rect.left = left - margin->left;
		this->margin.left = margin->left;
		rect.right = left + width - margin->right;
		this->margin.right = margin->right;
	}

	void set_height(LONG top, LONG height, LPRECT margin) {
		rect.top = top - margin->top;
		this->margin.top = margin->top;
		rect.bottom = top + height - margin->bottom;
		this->margin.bottom = margin->bottom;
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
