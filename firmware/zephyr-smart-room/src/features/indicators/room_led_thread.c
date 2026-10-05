#include "room_led_thread.h"

#include <stdbool.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/zbus/zbus.h>

#include "board_leds.h"
#include "room_bus.h"
#include "room_indicators.h"

#define ROOM_LED_THREAD_STACK_SIZE 1280
#define ROOM_LED_THREAD_PRIORITY 3
#define ROOM_LED_UPDATE_MS 50U

ZBUS_SUBSCRIBER_DEFINE(room_led_sub, 8);

static K_THREAD_STACK_DEFINE(room_led_thread_stack, ROOM_LED_THREAD_STACK_SIZE);
static K_SEM_DEFINE(room_led_thread_ready, 0, 1);
static struct k_thread room_led_thread_data;
static bool room_led_thread_started;

static void room_led_thread_fn(void *a, void *b, void *c)
{
	struct room_state state = {
		.occupied = false,
		.light_on = false,
		.contact_open = false,
		.alarm = ALARM_NONE,
	};
	bool have_state = false;

	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);
	/* Signal that the thread is ready to start processing. */
	k_sem_give(&room_led_thread_ready);

	while (1) {
		const struct zbus_channel *chan = NULL;
		int rc = zbus_sub_wait(&room_led_sub, &chan, K_MSEC(ROOM_LED_UPDATE_MS));

		if (rc == 0 && chan == &room_state_chan) {
			(void)zbus_chan_read(&room_state_chan, &state, K_NO_WAIT);
			have_state = true;
		}

		if (!have_state) {
			continue;
		}

		struct board_leds leds = room_indicators_compute_leds(&state, k_uptime_get_32());

		board_leds_apply(&leds);
	}
}

int room_led_thread_start(void)
{
	if (room_led_thread_started) {
		return 0;
	}

	k_thread_create(&room_led_thread_data, room_led_thread_stack,
			K_THREAD_STACK_SIZEOF(room_led_thread_stack), room_led_thread_fn,
			NULL, NULL, NULL, ROOM_LED_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&room_led_thread_data, "room_led");
	k_sem_take(&room_led_thread_ready, K_FOREVER);
	room_led_thread_started = true;
	return 0;
}
