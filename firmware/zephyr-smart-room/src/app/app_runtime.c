#include "app_runtime.h"

#include <zephyr/kernel.h>

#include "board_buttons.h"
#include "board_leds.h"
#include "room_diagnostics_thread.h"
#include "room_domain_thread.h"
#include "room_led_thread.h"
#include "room_protocol_thread.h"

#define APP_IDLE_SLEEP_MS 1000U

int app_runtime_run(void)
{
	int rc;

	rc = board_leds_init();
	if (rc != 0) {
		return rc;
	}

	rc = board_buttons_init();
	if (rc != 0) {
		return rc;
	}

	rc = room_diagnostics_thread_start();
	if (rc != 0) {
		return rc;
	}

	rc = room_led_thread_start();
	if (rc != 0) {
		return rc;
	}

	rc = room_protocol_thread_start();
	if (rc != 0) {
		return rc;
	}

	rc = room_domain_thread_start();
	if (rc != 0) {
		return rc;
	}

	while (1) {
		//k_msleep(APP_IDLE_SLEEP_MS);
		k_thread_suspend(k_current_get());
	}

	return 0;
}
