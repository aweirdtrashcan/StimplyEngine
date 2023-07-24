#pragma once

#include "includes.h"
#include "Renderer.h"

enum class EventType : uint8_t
{
	WINDOW_RESIZE
};

struct EventListener
{
	void (*EventCallback)(void* EventData);
	EventType type;
};

class Event
{
public:
	Event() = delete;
	~Event() = delete;
	
	static bool Subscribe(EventListener listener)
	{
		for (EventListener& list : _listeners)
		{
			if (list.EventCallback == listener.EventCallback)
			{
				return false;
			}
		}
		_listeners.push_back(listener);
		return true;
	}

	static void Trigger(EventType type, void* EventData)
	{
		for (EventListener& list : _listeners)
		{
			if (list.type == type)
			{
				list.EventCallback(EventData);
			}
		}
	}

	static void DisableEvents() { _listeners.clear(); }

private:
	static inline std::vector<EventListener> _listeners;
};

