#include "room_diagnostics_thread.h"

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "room_bus.h"
#include "room_console.h"
#include "room_model.h"

#define ROOM_DIAGNOSTICS_THREAD_STACK_SIZE 1536
#define ROOM_DIAGNOSTICS_THREAD_PRIORITY 4

LOG_MODULE_REGISTER(diag, LOG_LEVEL_INF);

ZBUS_SUBSCRIBER_DEFINE(room_diagnostics_sub, 16);

static K_THREAD_STACK_DEFINE(room_diagnostics_thread_stack, ROOM_DIAGNOSTICS_THREAD_STACK_SIZE);
static K_SEM_DEFINE(room_diagnostics_thread_ready, 0, 1);
static struct k_thread room_diagnostics_thread_data;
static bool room_diagnostics_thread_started;

static void room_diagnostics_thread_fn(void *a, void *b, void *c)
{
	struct room_state last_state = {
		.occupied = false,
		.light_on = false,
		.contact_open = false,
		.alarm = ALARM_NONE,
	};
	char log_line[128];
	bool have_state = false;

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	LOG_INF("%s", room_console_banner_title());
	LOG_INF("%s", room_console_banner_mapping());
	k_sem_give(&room_diagnostics_thread_ready);

	while (1) {
		const struct zbus_channel *chan = NULL;

		if (zbus_sub_wait(&room_diagnostics_sub, &chan, K_FOREVER) != 0) {
			continue;
		}

		if (chan == &room_state_chan) {
			struct room_state state;

			(void)zbus_chan_read(&room_state_chan, &state, K_NO_WAIT);

			if (!have_state) {
				room_console_format_state(log_line, sizeof(log_line), "boot", &state);
				LOG_INF("%s", log_line);
				last_state = state;
				have_state = true;
				continue;
			}

			if (room_model_diff_flags(&last_state, &state) == ROOM_CHANGE_NONE) {
				room_console_format_state(log_line, sizeof(log_line), "hb", &state);
				if (state.alarm == ALARM_ACTIVE) {
					LOG_ERR("%s", log_line);
				} else if (state.alarm == ALARM_WARNING) {
					LOG_WRN("%s", log_line);
				} else {
					LOG_INF("%s", log_line);
				}
			}

			last_state = state;
			continue;
		}

		if (chan == &room_event_chan) {
			struct room_event_message message;

			(void)zbus_chan_read(&room_event_chan, &message, K_NO_WAIT);
			room_console_format_event(log_line, sizeof(log_line), message.type,
						      &message.state);
			LOG_WRN("%s", log_line);

			room_console_format_state(log_line, sizeof(log_line), "state",
						  &message.state);
			if (message.state.alarm == ALARM_ACTIVE) {
				LOG_ERR("%s", log_line);
			} else if (message.state.alarm == ALARM_WARNING) {
				LOG_WRN("%s", log_line);
			} else {
				LOG_INF("%s", log_line);
			}

			last_state = message.state;
			have_state = true;
		}
	}
}

int room_diagnostics_thread_start(void)
{
	if (room_diagnostics_thread_started) {
		return 0;
	}

	k_thread_create(&room_diagnostics_thread_data, room_diagnostics_thread_stack,
			K_THREAD_STACK_SIZEOF(room_diagnostics_thread_stack),
			room_diagnostics_thread_fn, NULL, NULL, NULL,
			ROOM_DIAGNOSTICS_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&room_diagnostics_thread_data, "room_diag");
	k_sem_take(&room_diagnostics_thread_ready, K_FOREVER);
	room_diagnostics_thread_started = true;
	return 0;
}
