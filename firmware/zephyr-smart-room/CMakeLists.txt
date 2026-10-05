cmake_minimum_required(VERSION 3.20.0)

# Select the J-Link runner before Zephyr loads the board configuration.
set(BOARD_FLASH_RUNNER jlink)
set(BOARD_DEBUG_RUNNER jlink)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(firmware)

target_sources(app PRIVATE
	src/app/main.c
	src/app/app_runtime.c
	src/app/runtime/room_bus.c
	src/app/runtime/room_domain_thread.c
	src/domain/room/room_model.c
	src/diagnostics/room_diagnostics_thread.c
	src/features/indicators/room_indicators.c
	src/features/indicators/room_led_thread.c
	src/diagnostics/room_console.c
	src/platform/board/board_buttons.c
	src/platform/board/board_leds.c
	src/protocol/room_protocol_parser.c
	src/protocol/room_protocol_formatter.c
	src/protocol/room_protocol_thread.c
)

target_include_directories(app PRIVATE
	src/app
	src/app/runtime
	src/domain/room
	src/diagnostics
	src/features/indicators
	src/platform/board
	src/protocol
)
