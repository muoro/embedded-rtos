#include "room_console.h"

#include <zephyr/sys/printk.h>

const char *room_console_event_name(enum room_event_type event)
{
	switch (event) {
	case ROOM_EVENT_OCCUPANCY_TOGGLE:
		return "occupancy_toggle";
	case ROOM_EVENT_LIGHT_TOGGLE:
		return "light_toggle";
	case ROOM_EVENT_CONTACT_TOGGLE:
		return "contact_toggle";
	case ROOM_EVENT_ALARM_CYCLE:
		return "alarm_cycle";
	default:
		return "unknown";
	}
}

const char *room_console_banner_title(void)
{
	return "Smart Room (single-DK) starting...";
}

const char *room_console_banner_mapping(void)
{
	return "Buttons: B1/sw0=occ B2/sw1=light B3/sw2=contact B4/sw3=alarm | LEDs: hb, light, occ, alarm";
}

void room_console_format_event(char *buf, size_t buf_size, enum room_event_type event,
			       const struct room_state *state)
{
	(void)state;

	snprintk(buf, buf_size, "event: %s", room_console_event_name(event));
}

void room_console_format_state(char *buf, size_t buf_size, const char *tag,
			       const struct room_state *state)
{
	snprintk(buf, buf_size, "%s: occ=%s light=%s contact=%s alarm=%s", tag,
		 state->occupied ? "occupied" : "empty", state->light_on ? "on" : "off",
		 state->contact_open ? "open" : "closed", room_alarm_level_to_str(state->alarm));
}
