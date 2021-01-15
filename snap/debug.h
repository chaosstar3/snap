#pragma once
#include <Windows.h>
#include <stdio.h>

#ifdef _DEBUG
static void dprintf(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);

	char buf[256];
	vsprintf_s(buf, fmt, args);
	va_end(args);
	OutputDebugStringA(buf);
}

static void dprintrect(const char* name, LPRECT lpRect) {
	int width = lpRect->right - lpRect->left;
	int height = lpRect->bottom - lpRect->top;

	dprintf("%s %d - %d (%d) %d - %d (%d)", name,
		lpRect->left, lpRect->right, width,
		lpRect->top, lpRect->bottom, height);
}

#else
#define dprintf(fmt, ...)
#define dprintrect(name, rect)
#endif

#define TRACE(msg) dprintf("[%s:%d] %s %d", __func__, __LINE__, msg, GetLastError())
