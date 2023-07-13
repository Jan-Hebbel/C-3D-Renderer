#include "input.h"

#include "misc.h"

Event event_queue[EVENT_QUEUE_SIZE];
Key_State keyboard_state[KEY_CODE_AMOUNT];

void process_key_event(Key_Code key_code, Key_State key_state, Event_Reader *event_reader) {
	keyboard_state[key_code] = key_state; // set keyboard_state (this is for moving and the similar, since this works every frame)

	Event e = {0};
	e.key_code = key_code;
	e.key_state = key_state;
	event_queue[event_reader->index++] = e; // adding events to the event queue; only called every few frames
}

Event event_queue_next(Event_Reader *event_reader) {
	Key_State key_state_all_false = {0};
	Event dummy = { UNKNOWN, key_state_all_false };

	if (event_reader->index < 0 || event_reader->index >= EVENT_QUEUE_SIZE) {
		return dummy;
	}

	int old_index = event_reader->index++;

	Event old_event = event_queue[old_index];
	Event new_event = { UNKNOWN, key_state_all_false };
	event_queue[old_index] = new_event;
	return old_event;
}

Key_State get_key_state(Key_Code key_code) {
	return keyboard_state[key_code];
}

void reset_keyboard_state() {
	for (int i = 0; i < KEY_CODE_AMOUNT; ++i) {
		Key_State zero = {0};
		keyboard_state[i] = zero;
	}
}
