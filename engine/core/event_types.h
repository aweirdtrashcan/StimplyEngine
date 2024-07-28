#pragma once

#include "defines.h"
#include "window/key_defines.h"

enum EventType {
	MouseMoved,
	KeyboardEvent,
	ApplicationQuit,
	WindowResized,

	MAX
};

struct EventData {};

struct MouseEventData : EventData {
	// Relative mouse motion in the X direction. i.e: How much it has moved.
	int32_t MouseXMotion;
	// Relative mouse motion in the Y direction. i.e: How much it has moved.
	int32_t MouseYMotion;
	// Mouse X coordinate relative to the window.
	int32_t MouseXScreen;
	// Mouse Y coordinate relative to the window.
	int32_t MouseYScreen;
};

struct KeyboardEventData : EventData {
	Key Key;
	bool Pressed;
};

struct WindowResizedEventData : EventData {
	int32_t width;
	int32_t height;
};