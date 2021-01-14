#pragma once
#include <Windows.h>

enum class SNAP_TYPE { SNAP_LEFT, SNAP_RIGHT, SNAP_TOP, SNAP_BOTTOM, SNAP_CENTER, SNAP_FULL, SNAP_NONE };

struct SnapInfo {
	HWND window;
	SNAP_TYPE type;
	int index;
	RECT rect;
};

class WindowManager {
public:
	struct SnapInfo last_snap;

	void snap_window(SNAP_TYPE);
};
