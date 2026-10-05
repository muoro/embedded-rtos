#pragma once

#include <stdbool.h>
#include <stdint.h>

enum room_event_type {
	ROOM_EVENT_OCCUPANCY_TOGGLE = 0,
	ROOM_EVENT_LIGHT_TOGGLE = 1,
	ROOM_EVENT_CONTACT_TOGGLE = 2,
	ROOM_EVENT_ALARM_CYCLE = 3,
};

enum alarm_level {
	ALARM_NONE = 0,
	ALARM_WARNING = 1,
	ALARM_ACTIVE = 2,
};

enum room_property_id {
	ROOM_PROP_OCCUPIED = 0,
	ROOM_PROP_LIGHT_ON = 1,
	ROOM_PROP_CONTACT_OPEN = 2,
	ROOM_PROP_ALARM = 3,
};

enum room_change_flags {
	ROOM_CHANGE_NONE = 0U,
	ROOM_CHANGE_OCCUPANCY = (1U << 0),
	ROOM_CHANGE_LIGHT = (1U << 1),
	ROOM_CHANGE_CONTACT = (1U << 2),
	ROOM_CHANGE_ALARM = (1U << 3),

	ROOM_CHANGE_ALL = (ROOM_CHANGE_OCCUPANCY | ROOM_CHANGE_LIGHT | ROOM_CHANGE_CONTACT |
			   ROOM_CHANGE_ALARM),
};

struct room_state {
	bool occupied;
	bool light_on;
	bool contact_open;
	enum alarm_level alarm;
};

struct room_model {
	struct room_state state;
};

void room_model_init(struct room_model *model, uint32_t now_ms);
uint32_t room_model_apply_event(struct room_model *model, enum room_event_type event);

bool room_model_set_occupied(struct room_model *model, bool occupied);
bool room_model_set_light_on(struct room_model *model, bool light_on);
bool room_model_set_contact_open(struct room_model *model, bool contact_open);
bool room_model_set_alarm(struct room_model *model, enum alarm_level alarm);

const struct room_state *room_model_state(const struct room_model *model);
uint32_t room_model_diff_flags(const struct room_state *before, const struct room_state *after);

const char *room_alarm_level_to_str(enum alarm_level level);
