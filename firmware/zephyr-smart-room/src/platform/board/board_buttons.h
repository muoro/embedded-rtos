#pragma once

#include <stdint.h>

#include <zephyr/kernel.h>

#define BOARD_BUTTON_COUNT 4

struct board_button_event {
	uint8_t button_index; /* 0..BOARD_BUTTON_COUNT-1 */
	uint32_t timestamp_ms;
};

int board_buttons_init(void);
int board_buttons_get_event(struct board_button_event *event, k_timeout_t timeout);
