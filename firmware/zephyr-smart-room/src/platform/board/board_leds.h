#pragma once

#include <stdbool.h>

struct board_leds {
	bool led0;
	bool led1;
	bool led2;
	bool led3;
};

int board_leds_init(void);
void board_leds_apply(const struct board_leds *leds);
