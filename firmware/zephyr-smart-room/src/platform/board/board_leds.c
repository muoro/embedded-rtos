#include "board_leds.h"

#include <errno.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include <hal/nrf_gpio.h>

LOG_MODULE_REGISTER(board_leds, LOG_LEVEL_INF);

#define LED_SELF_TEST_STEP_MS 500U

#if !DT_NODE_HAS_STATUS(DT_ALIAS(led0), okay) || !DT_NODE_HAS_STATUS(DT_ALIAS(led1), okay) || \
	!DT_NODE_HAS_STATUS(DT_ALIAS(led2), okay) || !DT_NODE_HAS_STATUS(DT_ALIAS(led3), okay)
#error "Board missing led0..led3 Devicetree aliases"
#endif

static const struct gpio_dt_spec leds[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(led3), gpios),
};

static int set_led(int index, bool on)
{
	const struct gpio_dt_spec *led = &leds[index];
	int raw_value = on ? 1 : 0;

	if ((led->dt_flags & GPIO_ACTIVE_LOW) != 0U) {
		raw_value = !raw_value;
	}

	return gpio_pin_set_raw(led->port, led->pin, raw_value);
}

static void log_led_state(int index, bool on)
{
	const struct gpio_dt_spec *led = &leds[index];
	uint32_t out_value = nrf_gpio_pin_out_read(led->pin);
	nrf_gpio_pin_dir_t dir = nrf_gpio_pin_dir_get(led->pin);

	LOG_INF("led %d %s port=%s pin=%u flags=0x%x out=%u dir=%u",
		index, on ? "on" : "off", led->port->name, led->pin, led->dt_flags,
		out_value, dir);
}

static void led_self_test(void)
{
	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		for (int j = 0; j < ARRAY_SIZE(leds); j++) {
			(void)set_led(j, false);
		}

		(void)set_led(i, true);
		log_led_state(i, true);
		k_msleep(LED_SELF_TEST_STEP_MS);
	}

	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		(void)set_led(i, false);
	}
}

int board_leds_init(void)
{
	int rc;
	// validate GPIOs
	for (int i = 0; i < ARRAY_SIZE(leds); i++) {
		if (!gpio_is_ready_dt(&leds[i])) {
			LOG_ERR("LED %d not ready", i);
			return -ENODEV;
		}

		rc = gpio_pin_configure(leds[i].port, leds[i].pin, GPIO_OUTPUT);
		if (rc != 0) {
			LOG_ERR("LED %d configure failed: %d", i, rc);
			return rc;
		}

		rc = set_led(i, false);
		if (rc != 0) {
			LOG_ERR("LED %d initial set failed: %d", i, rc);
			return rc;
		}

		log_led_state(i, false);
	}

	led_self_test();
	LOG_INF("board leds init ok");
	return 0;
}

void board_leds_apply(const struct board_leds *led_state)
{
	if (led_state == NULL) {
		return;
	}

	(void)set_led(0, led_state->led0);
	(void)set_led(1, led_state->led1);
	(void)set_led(2, led_state->led2);
	(void)set_led(3, led_state->led3);
}
