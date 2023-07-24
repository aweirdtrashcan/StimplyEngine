#pragma once

#include <cstdint>

typedef uint64_t WINDLESS_CAPABILITIES;
typedef uint64_t WINDLESS_BACKGROUND_BEHAVIOR;

enum WINDLESS_PLATFORM
{
	WINDLESS_WIN32
};

enum
{
	WINDLESS_CAPABILITY_REDRAW_ON_RESIZE = 1,
	WINDLESS_CAPABILITY_NO_CLOSE = 2,
	WINDLESS_CAPABILITY_PROCESS_DOUBLE_MOUSE_CLICKS = 4,
	
	WINDLESS_CAPABILITY_ACCEPT_FILES = 8,
	WINDLESS_CAPABILITY_SHOW_ON_TASKBAR = 16,
	WINDLESS_CAPABILITY_NOT_SHOW_ON_TASKBAR = 32,
	WINDLESS_CAPABILITY_OVERLAPPED = 64,
	WINDLESS_CAPABILITY_SMALL_BAR_TOOL_WINDOW = 128,
	WINDLESS_CAPABILITY_TOP_MOST = 256,
	WINDLESS_CAPABILITY_TRANSPARENT = 512,

	WINDLESS_CAPABILITY_SMALL_BAR = 1024,
	WINDLESS_CAPABILITY_VISIBLE_NAME = 2048,
	WINDLESS_CAPABILITY_INITIALLY_MINIMIZED = 4096,
	WINDLESS_CAPABILITY_INITIALLY_MAXIMIZED = 8192,
	WINDLESS_CAPABILITY_HAS_MAXIMIZE_BOX = 8192 * 2,
	WINDLESS_CAPABILITY_HAS_MINIMIZE_BOX = 8192 * 4,
	WINDLESS_CAPABILITY_HAS_SIZING_BORDER = 8192 * 8,
	WINDLESS_CAPABILITY_INITIALLY_VISIBLE = 65536 * 2,
} _WINDLESS_CAPABILITIES;

enum
{
	WINDLESS_BACKGROUND_BEHAVIOR_TRANSPARENT = 1
} _WINDLESS_BACKGROUND_BEHAVIOR;

typedef struct vec2d
{
	uint32_t x;
	uint32_t y;
} vec2d;

struct windless_initialize_params
{
	const char* window_name;
	const char* class_name;
	vec2d initial_window_pos;
	vec2d initial_window_size;
	WINDLESS_CAPABILITIES capabilities_bitfield;
	WINDLESS_BACKGROUND_BEHAVIOR background_behavior_bitfield;
	const char* icon_path;
	const char* small_icon_path;
	const char* cursor_name;
};

typedef struct _wwindow32
{
	void* _hwnd;
	void* _hinstance;
	const char* _name;
	const char* _class_name;
	vec2d _initial_window_pos;
	vec2d _initial_window_size;
	WINDLESS_CAPABILITIES capabilities;
	WINDLESS_BACKGROUND_BEHAVIOR background;
	uint8_t _visible;
	uint8_t _running;

} _wwindow32;

typedef _wwindow32 *wwindow32;
typedef void (*PFN_windless_on_resize)(vec2d new_window_position);

uint8_t windless_win32_initialize(wwindow32* out_window, struct windless_initialize_params init_params, const char* out_error_msg);
uint8_t windless_win32_show_window(wwindow32 window);
uint8_t windless_win32_shutdown();
uint8_t windless_win32_begin_process_messages(wwindow32 window);
uint8_t windless_win32_window_resize(wwindow32 window, PFN_windless_on_resize resize_func_ptr);
