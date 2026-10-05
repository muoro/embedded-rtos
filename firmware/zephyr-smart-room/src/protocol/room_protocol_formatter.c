#include "room_protocol_formatter.h"

#include <stdio.h>

#include "room_console.h"

static const char *property_name(enum room_property_id property)
{
	switch (property) {
	case ROOM_PROP_OCCUPIED:
		return "occupied";
	case ROOM_PROP_LIGHT_ON:
		return "light_on";
	case ROOM_PROP_CONTACT_OPEN:
		return "contact_open";
	case ROOM_PROP_ALARM:
		return "alarm";
	default:
		return "unknown";
	}
}

void room_protocol_format_state(char *buf, size_t buf_size, const struct room_state *state)
{
	snprintf(buf, buf_size, "ROOM STATE occupied=%d light_on=%d contact_open=%d alarm=%s",
		 state->occupied ? 1 : 0, state->light_on ? 1 : 0,
		 state->contact_open ? 1 : 0, room_alarm_level_to_str(state->alarm));
}

void room_protocol_format_event(char *buf, size_t buf_size, const struct room_event_message *event)
{
	snprintf(buf, buf_size,
		 "ROOM EVENT type=%s changed=0x%08x occupied=%d light_on=%d contact_open=%d alarm=%s",
		 room_console_event_name(event->type), event->change_flags,
		 event->state.occupied ? 1 : 0, event->state.light_on ? 1 : 0,
		 event->state.contact_open ? 1 : 0, room_alarm_level_to_str(event->state.alarm));
}

void room_protocol_format_result(char *buf, size_t buf_size,
				 const struct room_command_result_message *result)
{
	if (result->type == ROOM_COMMAND_RESULT_STATE) {
		room_protocol_format_state(buf, buf_size, &result->state);
		return;
	}

	snprintf(buf, buf_size, "ROOM ACK property=%s changed=%d",
		 property_name(result->property), result->changed ? 1 : 0);
}

void room_protocol_format_error(char *buf, size_t buf_size, const char *code)
{
	snprintf(buf, buf_size, "ROOM ERR code=%s", code);
}
