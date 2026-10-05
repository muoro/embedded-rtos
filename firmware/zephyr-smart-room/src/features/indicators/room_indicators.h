#pragma once

#include <stdint.h>

#include "board_leds.h"
#include "room_model.h"

struct board_leds room_indicators_compute_leds(const struct room_state *state, uint32_t now_ms);
