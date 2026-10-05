#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>

#include "room_model.h"

enum room_domain_command_type {
	ROOM_DOMAIN_COMMAND_GET_STATE = 0,
	ROOM_DOMAIN_COMMAND_SET_BOOL = 1,
	ROOM_DOMAIN_COMMAND_SET_ALARM = 2,
};

struct room_domain_command {
	enum room_domain_command_type type;
	enum room_property_id property;
	bool bool_value;
	enum alarm_level alarm_value;
	uint32_t timestamp_ms;
};

int room_domain_thread_start(void);
int room_domain_submit_command(const struct room_domain_command *command, k_timeout_t timeout);
