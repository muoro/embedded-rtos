#include "room_indicators.h"

struct board_leds room_indicators_compute_leds(const struct room_state *state, uint32_t now_ms)
{
	bool heartbeat_on = ((now_ms / 500U) % 2U) == 0U;
	bool alarm_led_on = false;

	switch (state->alarm) {
	case ALARM_NONE:
		alarm_led_on = false;
		break;
	case ALARM_WARNING:
		alarm_led_on = ((now_ms / 500U) % 2U) == 0U;
		break;
	case ALARM_ACTIVE:
		alarm_led_on = ((now_ms / 200U) % 2U) == 0U;
		break;
	}

	return (struct board_leds){
		.led0 = heartbeat_on,
		.led1 = state->light_on,
		.led2 = state->occupied,
		.led3 = alarm_led_on,
	};
}
