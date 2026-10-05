#include "room_domain_thread.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/zbus/zbus.h>

#include "board_buttons.h"
#include "room_bus.h"
#include "room_model.h"

#define ROOM_DOMAIN_THREAD_STACK_SIZE 1536
#define ROOM_DOMAIN_THREAD_PRIORITY 2
#define ROOM_HEARTBEAT_INTERVAL_MS 5000U
#define ROOM_COMMAND_QUEUE_LEN 8

static K_THREAD_STACK_DEFINE(room_domain_thread_stack, ROOM_DOMAIN_THREAD_STACK_SIZE);
K_MSGQ_DEFINE(room_command_q, sizeof(struct room_domain_command), ROOM_COMMAND_QUEUE_LEN, 4);
static struct k_thread room_domain_thread_data;
static bool room_domain_thread_started;

static bool map_button_to_event(uint8_t index, enum room_event_type *out)
{
	switch (index) {
	case 0:
		*out = ROOM_EVENT_OCCUPANCY_TOGGLE;
		return true;
	case 1:
		*out = ROOM_EVENT_LIGHT_TOGGLE;
		return true;
	case 2:
		*out = ROOM_EVENT_CONTACT_TOGGLE;
		return true;
	case 3:
		*out = ROOM_EVENT_ALARM_CYCLE;
		return true;
	default:
		return false;
	}
}

static void publish_state_snapshot(const struct room_state *state)
{
	struct room_state snapshot = *state;

	(void)zbus_chan_pub(&room_state_chan, &snapshot, K_NO_WAIT);
}

static void publish_event_message(enum room_event_type event, uint32_t change_flags,
				  const struct room_state *state, uint32_t timestamp_ms)
{
	struct room_event_message message = {
		.type = event,
		.change_flags = change_flags,
		.state = *state,
		.timestamp_ms = timestamp_ms,
	};

	(void)zbus_chan_pub(&room_event_chan, &message, K_NO_WAIT);
}

static void publish_command_result(enum room_command_result_type type,
				   const struct room_domain_command *command,
				   bool changed, const struct room_state *state)
{
	struct room_command_result_message message = {
		.type = type,
		.property = command->property,
		.changed = changed,
		.state = *state,
		.timestamp_ms = command->timestamp_ms,
	};

	(void)zbus_chan_pub(&room_command_result_chan, &message, K_NO_WAIT);
}

static bool apply_command(struct room_model *model, const struct room_domain_command *command)
{
	switch (command->type) {
	case ROOM_DOMAIN_COMMAND_SET_BOOL:
		switch (command->property) {
		case ROOM_PROP_OCCUPIED:
			return room_model_set_occupied(model, command->bool_value);
		case ROOM_PROP_LIGHT_ON:
			return room_model_set_light_on(model, command->bool_value);
		case ROOM_PROP_CONTACT_OPEN:
			return room_model_set_contact_open(model, command->bool_value);
		default:
			return false;
		}
	case ROOM_DOMAIN_COMMAND_SET_ALARM:
		return room_model_set_alarm(model, command->alarm_value);
	default:
		return false;
	}
}

static void handle_command(struct room_model *model, const struct room_domain_command *command)
{
	const struct room_state *state;
	bool changed = false;

	if (command->type == ROOM_DOMAIN_COMMAND_GET_STATE) {
		publish_command_result(ROOM_COMMAND_RESULT_STATE, command, false,
				       room_model_state(model));
		return;
	}

	changed = apply_command(model, command);
	state = room_model_state(model);

	if (changed) {
		publish_state_snapshot(state);
	}

	publish_command_result(ROOM_COMMAND_RESULT_ACK, command, changed, state);
}

static void room_domain_thread_fn(void *a, void *b, void *c)
{
	struct room_model model;
	uint32_t last_hb_ms;

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	last_hb_ms = k_uptime_get_32();
	room_model_init(&model, last_hb_ms);
	publish_state_snapshot(room_model_state(&model));

	while (1) {
		struct board_button_event button_event;
		struct room_domain_command command;
		uint32_t now_ms = k_uptime_get_32();
		uint32_t elapsed_ms = now_ms - last_hb_ms;
		uint32_t wait_ms = ROOM_HEARTBEAT_INTERVAL_MS - MIN(elapsed_ms, ROOM_HEARTBEAT_INTERVAL_MS);
		int rc;

		while (k_msgq_get(&room_command_q, &command, K_NO_WAIT) == 0) {
			handle_command(&model, &command);
		}

		rc = board_buttons_get_event(&button_event, K_MSEC(MIN(wait_ms, 50U)));

		if (rc == 0) {
			enum room_event_type event;
			uint32_t change_flags;
			const struct room_state *state;

			if (!map_button_to_event(button_event.button_index, &event)) {
				continue;
			}

			change_flags = room_model_apply_event(&model, event);
			state = room_model_state(&model);

			if (change_flags != ROOM_CHANGE_NONE) {
				publish_state_snapshot(state);
			}

			publish_event_message(event, change_flags, state, button_event.timestamp_ms);
			continue;
		}

		now_ms = k_uptime_get_32();
		if ((now_ms - last_hb_ms) >= ROOM_HEARTBEAT_INTERVAL_MS) {
			last_hb_ms = now_ms;
			publish_state_snapshot(room_model_state(&model));
		}
	}
}

int room_domain_thread_start(void)
{
	if (room_domain_thread_started) {
		return 0;
	}

	k_thread_create(&room_domain_thread_data, room_domain_thread_stack,
			K_THREAD_STACK_SIZEOF(room_domain_thread_stack), room_domain_thread_fn,
			NULL, NULL, NULL, ROOM_DOMAIN_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&room_domain_thread_data, "room_domain");
	room_domain_thread_started = true;
	return 0;
}

int room_domain_submit_command(const struct room_domain_command *command, k_timeout_t timeout)
{
	if (command == NULL) {
		return -EINVAL;
	}

	return k_msgq_put(&room_command_q, command, timeout);
}
