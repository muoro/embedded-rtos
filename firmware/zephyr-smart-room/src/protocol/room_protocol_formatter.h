#pragma once

#include <stddef.h>

#include "room_bus.h"

void room_protocol_format_state(char *buf, size_t buf_size, const struct room_state *state);
void room_protocol_format_event(char *buf, size_t buf_size, const struct room_event_message *event);
void room_protocol_format_result(char *buf, size_t buf_size,
				 const struct room_command_result_message *result);
void room_protocol_format_error(char *buf, size_t buf_size, const char *code);
