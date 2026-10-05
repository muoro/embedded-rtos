#include "room_bus.h"

ZBUS_CHAN_DEFINE(room_state_chan,
		 struct room_state,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS(room_led_sub, room_diagnostics_sub, room_protocol_sub),
		 ZBUS_MSG_INIT(.occupied = false,
			       .light_on = false,
			       .contact_open = false,
			       .alarm = ALARM_NONE));

ZBUS_CHAN_DEFINE(room_event_chan,
		 struct room_event_message,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS(room_diagnostics_sub, room_protocol_sub),
		 ZBUS_MSG_INIT(.type = ROOM_EVENT_OCCUPANCY_TOGGLE,
			       .change_flags = ROOM_CHANGE_NONE,
			       .state = {.occupied = false,
					 .light_on = false,
					 .contact_open = false,
					 .alarm = ALARM_NONE},
			       .timestamp_ms = 0U));

ZBUS_CHAN_DEFINE(room_command_result_chan,
		 struct room_command_result_message,
		 NULL,
		 NULL,
		 ZBUS_OBSERVERS(room_protocol_sub),
		 ZBUS_MSG_INIT(.type = ROOM_COMMAND_RESULT_STATE,
			       .property = ROOM_PROP_OCCUPIED,
			       .changed = false,
			       .state = {.occupied = false,
					 .light_on = false,
					 .contact_open = false,
					 .alarm = ALARM_NONE},
			       .timestamp_ms = 0U));
