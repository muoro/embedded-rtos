#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/zbus/zbus.h>

#include "room_model.h"

struct room_event_message {
	enum room_event_type type;
	uint32_t change_flags;
	struct room_state state;
	uint32_t timestamp_ms;
};

enum room_command_result_type {
	ROOM_COMMAND_RESULT_ACK = 0,
	ROOM_COMMAND_RESULT_STATE = 1,
};

struct room_command_result_message {
	enum room_command_result_type type;
	enum room_property_id property;
	bool changed;
	struct room_state state;
	uint32_t timestamp_ms;
};

extern const struct zbus_observer room_led_sub;
extern const struct zbus_observer room_diagnostics_sub;
extern const struct zbus_observer room_protocol_sub;

ZBUS_CHAN_DECLARE(room_state_chan);
ZBUS_CHAN_DECLARE(room_event_chan);
ZBUS_CHAN_DECLARE(room_command_result_chan);
