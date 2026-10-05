#pragma once

#include <stdbool.h>

#include "room_domain_thread.h"

#define ROOM_PROTOCOL_MAX_LINE_LEN 128

enum room_protocol_parse_result {
	ROOM_PROTOCOL_PARSE_OK = 0,
	ROOM_PROTOCOL_PARSE_PING,
	ROOM_PROTOCOL_PARSE_INVALID,
};

enum room_protocol_parse_result
room_protocol_parse_line(const char *line, struct room_domain_command *command);
