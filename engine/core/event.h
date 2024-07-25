#pragma once

#include "event_types.h"
#include "containers/list.h"

class RAPI IEvent {
public:
	IEvent() = default;
	virtual ~IEvent() = default;

	virtual void OnEvent(EventType type, const EventData* eventData) = 0;

	static bool RegisterListener(IEvent* pEvent, EventType type);
	static bool UnregisterListener(IEvent* pEvent, EventType type);
	static void FireEvent(EventType type, const EventData* eventData);

private:
	static inline list<IEvent*> s_EventListeners[EventType::MAX];
};