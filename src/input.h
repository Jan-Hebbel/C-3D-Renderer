/*
* Here is how input works for this game:
*
* Once per frame platform_process_events() is called. This function
* polls Windows for events. If the event was a Keydown or
* Syskeydown event we set the local keyboard state for the key to
* true. It stays that way until the key is released and Windows
* sends a new event, that's when the keyboard state for that key is
* set to false.
*
* Poll the internal keyboard state to get the position of key for
* every frame.
*
* The other system in place is the one that just sends events as
* Windows does and sends stores them in an Event queue. These can
* be polled with event_queue_next().
*
* Use this when it's not important or unwanted to check for a key
* state every frame. Like for example when pressing Esc to enter
* the menu.
*/

#ifndef INPUT_H
#define INPUT_H

#define EVENT_QUEUE_SIZE 10

typedef enum Tag_Key_Code {
	UNKNOWN = 0,
	ESCAPE = 1,
	W = 2,
	A = 3,
	S = 4,
	D = 5,
	SPACE = 6,
	KEY_CODE_AMOUNT = 7
} Key_Code;

typedef struct Tag_Key_State { // @Memory: put these into one byte, we only need a single bit for any of them
	b8 is_down;
	b8 released;
	b8 repeated;
	b8 alt_down;
} Key_State;

typedef struct Tag_Event {
	Key_Code key_code;
	Key_State key_state;
} Event;

typedef struct Tag_Event_Reader { // typedef better? @Cleanup
	int index;
} Event_Reader;

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

void reset_keyboard_state(void) {
	for (int i = 0; i < KEY_CODE_AMOUNT; ++i) {
		Key_State zero = {0};
		keyboard_state[i] = zero;
	}
}

#endif

// see:  https://youtu.be/AAFkdrP1CHQ?list=PLmV5I2fxaiCI9IAdFmGChKbIbenqRMi6Z&t=3549
// also: https://youtu.be/AAFkdrP1CHQ?list=PLmV5I2fxaiCI9IAdFmGChKbIbenqRMi6Z&t=5379
