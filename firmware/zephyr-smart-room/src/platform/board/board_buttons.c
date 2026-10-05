#include "board_buttons.h"

#include <errno.h>
#include <stdbool.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(board_buttons, LOG_LEVEL_INF);

#define BUTTON_EVENT_QUEUE_LEN 16
#define DEBOUNCE_MS 50U

#if !DT_NODE_HAS_STATUS(DT_ALIAS(sw0), okay) || !DT_NODE_HAS_STATUS(DT_ALIAS(sw1), okay) || \
	!DT_NODE_HAS_STATUS(DT_ALIAS(sw2), okay) || !DT_NODE_HAS_STATUS(DT_ALIAS(sw3), okay)
#error "Board missing sw0..sw3 Devicetree aliases"
#endif

static const struct gpio_dt_spec buttons[] = {
	GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw1), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw2), gpios),
	GPIO_DT_SPEC_GET(DT_ALIAS(sw3), gpios),
};

struct button_ctx {
	struct gpio_callback callback;
	uint8_t index;
};

K_MSGQ_DEFINE(button_event_q, sizeof(struct board_button_event), BUTTON_EVENT_QUEUE_LEN, 4);

static struct button_ctx button_ctxs[BOARD_BUTTON_COUNT];
static uint32_t last_button_event_ms[BOARD_BUTTON_COUNT];
static bool last_button_active[BOARD_BUTTON_COUNT];

static void button_isr(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	struct button_ctx *ctx = CONTAINER_OF(cb, struct button_ctx, callback);
	uint32_t now_ms = k_uptime_get_32();
	struct board_button_event event;

	ARG_UNUSED(port);
	ARG_UNUSED(pins);

	if (last_button_event_ms[ctx->index] != 0U &&
	    (now_ms - last_button_event_ms[ctx->index]) < DEBOUNCE_MS) {
		return;
	}

	last_button_event_ms[ctx->index] = now_ms;
	last_button_active[ctx->index] = true;

	event.button_index = ctx->index;
	event.timestamp_ms = now_ms;

	if (k_msgq_put(&button_event_q, &event, K_NO_WAIT) == 0) {
		LOG_INF("button %u irq event", ctx->index);
	} else {
		LOG_WRN("button %u irq queue full", ctx->index);
	}
}

static bool button_is_active(uint8_t index)
{
	int value = gpio_pin_get_dt(&buttons[index]);

	if (value < 0) {
		LOG_WRN("button %u read failed: %d", index, value);
		return false;
	}

	return value != 0;
}

static int poll_button_event(struct board_button_event *event)
{
	uint32_t now_ms = k_uptime_get_32();

	for (uint8_t i = 0U; i < ARRAY_SIZE(buttons); i++) {
		bool active = button_is_active(i);

		if (active && !last_button_active[i] &&
		    (last_button_event_ms[i] == 0U ||
		     (now_ms - last_button_event_ms[i]) >= DEBOUNCE_MS)) {
			last_button_event_ms[i] = now_ms;
			last_button_active[i] = true;
			event->button_index = i;
			event->timestamp_ms = now_ms;
			LOG_INF("button %u poll event", i);
			return 0;
		}

		last_button_active[i] = active;
	}

	return -EAGAIN;
}

int board_buttons_init(void)
{
	int rc;

	for (int i = 0; i < ARRAY_SIZE(buttons); i++) {
		if (!gpio_is_ready_dt(&buttons[i])) {
			LOG_ERR("button %d not ready", i);
			return -ENODEV;
		}

		rc = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT);
		if (rc != 0) {
			LOG_ERR("button %d configure failed: %d", i, rc);
			return rc;
		}

		button_ctxs[i].index = i;
		gpio_init_callback(&button_ctxs[i].callback, button_isr, BIT(buttons[i].pin));

		rc = gpio_add_callback(buttons[i].port, &button_ctxs[i].callback);
		if (rc != 0) {
			LOG_ERR("button %d add_callback failed: %d", i, rc);
			return rc;
		}

		rc = gpio_pin_interrupt_configure_dt(&buttons[i], GPIO_INT_EDGE_TO_ACTIVE);
		if (rc != 0) {
			LOG_ERR("button %d irq configure failed: %d", i, rc);
			return rc;
		}

		last_button_active[i] = button_is_active(i);
		LOG_INF("button %d port=%s pin=%u flags=0x%x active=%d",
			i, buttons[i].port->name, buttons[i].pin, buttons[i].dt_flags,
			last_button_active[i]);
	}

	LOG_INF("board buttons init ok");
	return 0;
}

int board_buttons_get_event(struct board_button_event *event, k_timeout_t timeout)
{
	int rc;

	if (event == NULL) {
		return -EINVAL;
	}

	rc = k_msgq_get(&button_event_q, event, timeout);
	if (rc == 0) {
		return 0;
	}

	return poll_button_event(event);
}
