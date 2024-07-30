#include "event.h"

#include "core/logger.h"

bool IEvent::RegisterListener(IEvent* pEvent, EventType type) {
	// -1 means there's no entry in this event type list.
	bool isAlreadyRegistered = s_EventListeners[type].find_index(pEvent) != -1;

	if (isAlreadyRegistered) {
		Logger::warning("Trying to register listener (%p) more than once in event code %i", pEvent, type);
		return false;
	}

	s_EventListeners[type].push_back(pEvent);

	return true;
}

bool IEvent::UnregisterListener(IEvent* pEvent, EventType type) {
	// -1 means there's no entry in this event type list.
	size_t eventIndex = s_EventListeners[type].find_index(pEvent);
	bool isAlreadyRegistered = eventIndex != -1;

	if (!isAlreadyRegistered) {
		Logger::warning("Listener (%p) is not registered for event code: %i", pEvent, type);
		return false;
	}

	s_EventListeners[type].remove_at(eventIndex);

	return true;
}

void IEvent::FireEvent(EventType type, const EventData* eventData) {
	for (size_t i = 0; i < s_EventListeners[type].size(); i++) {
		s_EventListeners[type][i]->OnEvent(type, eventData);
	}
}
