#include "room_protocol_parser.h"

#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE_LOCAL(array) (sizeof(array) / sizeof((array)[0]))

static bool parse_bool(const char *text, bool *value)
{
	if (strcmp(text, "0") == 0 || strcmp(text, "false") == 0 || strcmp(text, "off") == 0) {
		*value = false;
		return true;
	}

	if (strcmp(text, "1") == 0 || strcmp(text, "true") == 0 || strcmp(text, "on") == 0) {
		*value = true;
		return true;
	}

	return false;
}

static bool parse_alarm(const char *text, enum alarm_level *alarm)
{
	if (strcmp(text, "0") == 0 || strcmp(text, "none") == 0) {
		*alarm = ALARM_NONE;
		return true;
	}

	if (strcmp(text, "1") == 0 || strcmp(text, "warning") == 0) {
		*alarm = ALARM_WARNING;
		return true;
	}

	if (strcmp(text, "2") == 0 || strcmp(text, "alarm") == 0 || strcmp(text, "active") == 0) {
		*alarm = ALARM_ACTIVE;
		return true;
	}

	return false;
}

static bool parse_property(const char *text, enum room_property_id *property)
{
	if (strcmp(text, "occupied") == 0 || strcmp(text, "occupancy") == 0) {
		*property = ROOM_PROP_OCCUPIED;
		return true;
	}

	if (strcmp(text, "light_on") == 0 || strcmp(text, "light") == 0) {
		*property = ROOM_PROP_LIGHT_ON;
		return true;
	}

	if (strcmp(text, "contact_open") == 0 || strcmp(text, "contact") == 0) {
		*property = ROOM_PROP_CONTACT_OPEN;
		return true;
	}

	if (strcmp(text, "alarm") == 0) {
		*property = ROOM_PROP_ALARM;
		return true;
	}

	return false;
}

enum room_protocol_parse_result
room_protocol_parse_line(const char *line, struct room_domain_command *command)
{
	char buffer[ROOM_PROTOCOL_MAX_LINE_LEN];
	char *tokens[5];
	size_t count = 0U;
	char *ctx = NULL;
	char *token;
	size_t start = 0U;

	if (line == NULL || command == NULL) {
		return ROOM_PROTOCOL_PARSE_INVALID;
	}

	while (line[start] == ' ' || line[start] == '\t') {
		start++;
	}

	strncpy(buffer, &line[start], sizeof(buffer) - 1U);
	buffer[sizeof(buffer) - 1U] = '\0';

	token = strtok_r(buffer, " \t\r\n", &ctx);
	while (token != NULL && count < ARRAY_SIZE_LOCAL(tokens)) {
		tokens[count++] = token;
		token = strtok_r(NULL, " \t\r\n", &ctx);
	}

	if (count > 0U && strcmp(tokens[0], "ROOM") == 0) {
		if (count == 1U) {
			return ROOM_PROTOCOL_PARSE_INVALID;
		}

		for (size_t i = 1U; i < count; i++) {
			tokens[i - 1U] = tokens[i];
		}
		count--;
	}

	if (count == 1U && strcmp(tokens[0], "PING") == 0) {
		return ROOM_PROTOCOL_PARSE_PING;
	}

	if ((count == 2U && strcmp(tokens[0], "GET") == 0 && strcmp(tokens[1], "STATE") == 0) ||
	    (count == 1U && strcmp(tokens[0], "STATE") == 0)) {
		command->type = ROOM_DOMAIN_COMMAND_GET_STATE;
		command->property = ROOM_PROP_OCCUPIED;
		return ROOM_PROTOCOL_PARSE_OK;
	}

	if (count == 3U && strcmp(tokens[0], "SET") == 0) {
		enum room_property_id property;

		if (!parse_property(tokens[1], &property)) {
			return ROOM_PROTOCOL_PARSE_INVALID;
		}

		command->property = property;
		if (property == ROOM_PROP_ALARM) {
			command->type = ROOM_DOMAIN_COMMAND_SET_ALARM;
			return parse_alarm(tokens[2], &command->alarm_value) ?
				       ROOM_PROTOCOL_PARSE_OK : ROOM_PROTOCOL_PARSE_INVALID;
		}

		command->type = ROOM_DOMAIN_COMMAND_SET_BOOL;
		return parse_bool(tokens[2], &command->bool_value) ?
			       ROOM_PROTOCOL_PARSE_OK : ROOM_PROTOCOL_PARSE_INVALID;
	}

	return ROOM_PROTOCOL_PARSE_INVALID;
}
