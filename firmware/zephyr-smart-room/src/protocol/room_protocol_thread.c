#include "room_protocol_thread.h"

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/atomic.h>

#include "room_bus.h"
#include "room_domain_thread.h"
#include "room_protocol_formatter.h"
#include "room_protocol_parser.h"

#define ROOM_PROTOCOL_THREAD_STACK_SIZE 1536
#define ROOM_PROTOCOL_THREAD_PRIORITY 5
#define ROOM_PROTOCOL_POLL_MS 20U
#define ROOM_PROTOCOL_TX_LINE_LEN 192

LOG_MODULE_REGISTER(protocol, LOG_LEVEL_INF);

ZBUS_SUBSCRIBER_DEFINE(room_protocol_sub, 16);

static K_THREAD_STACK_DEFINE(room_protocol_thread_stack, ROOM_PROTOCOL_THREAD_STACK_SIZE);
static K_SEM_DEFINE(room_protocol_thread_ready, 0, 1);
static struct k_thread room_protocol_thread_data;
static bool room_protocol_thread_started;
static const struct device *uart_dev;

static void uart_write_line(const char *line)
{
	for (size_t i = 0U; line[i] != '\0'; i++) {
		uart_poll_out(uart_dev, line[i]);
	}
	uart_poll_out(uart_dev, '\r');
	uart_poll_out(uart_dev, '\n');
}

static void send_error(const char *code)
{
	char line[ROOM_PROTOCOL_TX_LINE_LEN];

	room_protocol_format_error(line, sizeof(line), code);
	uart_write_line(line);
}

static void send_current_state(void)
{
	struct room_state state;
	char line[ROOM_PROTOCOL_TX_LINE_LEN];

	(void)zbus_chan_read(&room_state_chan, &state, K_NO_WAIT);
	room_protocol_format_state(line, sizeof(line), &state);
	uart_write_line(line);
}

static bool is_protocol_output_line(const char *line)
{
	return strncmp(line, "ROOM STATE", 10) == 0 ||
	       strncmp(line, "ROOM EVENT", 10) == 0 ||
	       strncmp(line, "ROOM ACK", 8) == 0 ||
	       strncmp(line, "ROOM ERR", 8) == 0 ||
	       strncmp(line, "ROOM BOOT", 9) == 0 ||
	       strncmp(line, "ROOM PONG", 9) == 0;
}

static void handle_rx_line(const char *line)
{
	struct room_domain_command command = {
		.timestamp_ms = k_uptime_get_32(),
	};
	enum room_protocol_parse_result result = room_protocol_parse_line(line, &command);

	if (is_protocol_output_line(line)) {
		return;
	}

	switch (result) {
	case ROOM_PROTOCOL_PARSE_PING:
		uart_write_line("ROOM PONG");
		break;
	case ROOM_PROTOCOL_PARSE_OK:
		if (command.type == ROOM_DOMAIN_COMMAND_GET_STATE) {
			send_current_state();
			break;
		}

		if (room_domain_submit_command(&command, K_NO_WAIT) != 0) {
			send_error("command_queue_full");
		}
		break;
	case ROOM_PROTOCOL_PARSE_INVALID:
	default:
		send_error("invalid_command");
		break;
	}
}

/* Assemble complete lines in the ISR so polling delays cannot lose UART bytes.
 * Only the protocol thread parses commands or writes responses.
 */
struct rx_line {
	char text[ROOM_PROTOCOL_MAX_LINE_LEN];
};

K_MSGQ_DEFINE(rx_lines, sizeof(struct rx_line), 8, 4);
static atomic_t rx_overflow;

static void uart_rx_irq(const struct device *dev, void *context)
{
	static struct rx_line line;
	static size_t len;
	static bool discarding;
	unsigned char bytes[16];
	int count;

	ARG_UNUSED(context);
	if (!uart_irq_update(dev)) {
		return;
	}
	while (uart_irq_rx_ready(dev)) {
		count = uart_fifo_read(dev, bytes, sizeof(bytes));
		if (count <= 0) {
			break;
		}
		for (int i = 0; i < count; i++) {
			unsigned char ch = bytes[i];

			if (ch == '\r' || ch == '\n') {
				if (!discarding && len > 0U) {
					line.text[len] = '\0';
					if (k_msgq_put(&rx_lines, &line, K_NO_WAIT) != 0) {
						atomic_set(&rx_overflow, 1);
					}
				}
				len = 0U;
				discarding = false;
			} else if (!discarding) {
				if (len + 1U >= sizeof(line.text)) {
					len = 0U;
					discarding = true;
					atomic_set(&rx_overflow, 1);
				} else {
					line.text[len++] = (char)ch;
				}
			}
		}
	}
}

static void poll_uart_rx(void)
{
	struct rx_line line;

	if (atomic_set(&rx_overflow, 0)) {
		send_error("rx_overflow");
	}
	while (k_msgq_get(&rx_lines, &line, K_NO_WAIT) == 0) {
		handle_rx_line(line.text);
	}
}
static void drain_protocol_events(void)
{
	const struct zbus_channel *chan = NULL;
	char line[ROOM_PROTOCOL_TX_LINE_LEN];

	while (zbus_sub_wait(&room_protocol_sub, &chan, K_NO_WAIT) == 0) {
		if (chan == &room_state_chan) {
			struct room_state state;

			(void)zbus_chan_read(&room_state_chan, &state, K_NO_WAIT);
			room_protocol_format_state(line, sizeof(line), &state);
			uart_write_line(line);
		} else if (chan == &room_event_chan) {
			struct room_event_message event;

			(void)zbus_chan_read(&room_event_chan, &event, K_NO_WAIT);
			room_protocol_format_event(line, sizeof(line), &event);
			uart_write_line(line);
		} else if (chan == &room_command_result_chan) {
			struct room_command_result_message result;

			(void)zbus_chan_read(&room_command_result_chan, &result, K_NO_WAIT);
			room_protocol_format_result(line, sizeof(line), &result);
			uart_write_line(line);
		}
	}
}

static void room_protocol_thread_fn(void *a, void *b, void *c)
{
	ARG_UNUSED(a);
	ARG_UNUSED(b);
	ARG_UNUSED(c);

	k_sem_give(&room_protocol_thread_ready);
	uart_write_line("ROOM BOOT proto=1 node=smart_room");

	while (1) {
		poll_uart_rx();
		drain_protocol_events();
		k_sleep(K_MSEC(ROOM_PROTOCOL_POLL_MS));
	}
}

int room_protocol_thread_start(void)
{
	if (room_protocol_thread_started) {
		return 0;
	}

	uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	if (!device_is_ready(uart_dev)) {
		return -ENODEV;
	}

	int err = uart_irq_callback_user_data_set(uart_dev, uart_rx_irq, NULL);
	if (err != 0) {
		return err;
	}
	uart_irq_rx_enable(uart_dev);

	k_thread_create(&room_protocol_thread_data, room_protocol_thread_stack,
			K_THREAD_STACK_SIZEOF(room_protocol_thread_stack),
			room_protocol_thread_fn, NULL, NULL, NULL,
			ROOM_PROTOCOL_THREAD_PRIORITY, 0, K_NO_WAIT);
	k_thread_name_set(&room_protocol_thread_data, "room_protocol");
	k_sem_take(&room_protocol_thread_ready, K_FOREVER);
	room_protocol_thread_started = true;
	return 0;
}
