#pragma once

enum EventType {
	MouseMoved,
	KeyboardEvent,
	ApplicationQuit,

	MAX
};

struct EventData {};

struct MouseEventData : EventData {
	// Relative mouse motion in the X direction. i.e: How much it has moved.
	int MouseXMotion;
	// Relative mouse motion in the Y direction. i.e: How much it has moved.
	int MouseYMotion;
	// Mouse X coordinate relative to the window.
	int MouseXScreen;
	// Mouse Y coordinate relative to the window.
	int MouseYScreen;
};

struct KeyboardEventData : EventData {
	int Key;
	bool Pressed;
};