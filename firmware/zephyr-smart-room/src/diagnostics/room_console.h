#pragma once

#include <stddef.h>

#include "room_model.h"

const char *room_console_banner_title(void);
const char *room_console_banner_mapping(void);
const char *room_console_event_name(enum room_event_type event);
void room_console_format_event(char *buf, size_t buf_size, enum room_event_type event,
			       const struct room_state *state);
void room_console_format_state(char *buf, size_t buf_size, const char *tag,
			       const struct room_state *state);
