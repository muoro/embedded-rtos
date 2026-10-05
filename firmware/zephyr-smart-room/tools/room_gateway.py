#!/usr/bin/env python3
"""
Command-line gateway for the Smart Room firmware UART protocol.

Examples:
    python tools/room_gateway.py --port COM5 ping
    python tools/room_gateway.py --port COM5 state
    python tools/room_gateway.py --port COM5 set light_on 1
    python tools/room_gateway.py --port COM5 watch
"""

from __future__ import annotations

import argparse
import sys
import time
from dataclasses import dataclass

try:
	import serial
except ImportError:  # pragma: no cover - exercised by users without pyserial
	serial = None


DEFAULT_BAUDRATE = 115200
DEFAULT_TIMEOUT = 3.0
DEFAULT_SETTLE = 2.5


@dataclass(frozen=True)
class RoomState:
	occupied: int
	light_on: int
	contact_open: int
	alarm: str


def parse_key_values(line: str) -> dict[str, str]:
	values: dict[str, str] = {}
	for token in line.split()[2:]:
		if "=" not in token:
			continue
		key, value = token.split("=", 1)
		values[key] = value
	return values


def parse_state(line: str) -> RoomState:
	if not line.startswith("ROOM STATE "):
		raise ValueError(f"not a state line: {line}")
	values = parse_key_values(line)
	return RoomState(
		occupied=int(values["occupied"]),
		light_on=int(values["light_on"]),
		contact_open=int(values["contact_open"]),
		alarm=values["alarm"],
	)


def open_serial(port: str, baudrate: int, settle: float):
	if serial is None:
		raise SystemExit("pyserial is required. Install with: python -m pip install -r requirements.txt")
	ser = serial.Serial(port=port, baudrate=baudrate, timeout=0.2, write_timeout=1.0)
	if settle > 0:
		time.sleep(settle)
	ser.reset_input_buffer()
	ser.reset_output_buffer()
	return ser


def write_command(ser: serial.Serial, command: str) -> None:
	ser.reset_input_buffer()
	ser.write((command.strip() + "\n").encode("utf-8"))
	ser.flush()


def read_line(ser: serial.Serial) -> str | None:
	raw = ser.readline()
	if not raw:
		return None
	return raw.decode("utf-8", errors="replace").strip()


def wait_for_prefix(ser: serial.Serial, prefix: str, timeout: float) -> str:
	deadline = time.monotonic() + timeout
	while time.monotonic() < deadline:
		line = read_line(ser)
		if line is None:
			continue
		print(line)
		if line.startswith("ROOM ERR"):
			raise RuntimeError(line)
		if line.startswith(prefix):
			return line
	raise TimeoutError(f"timed out waiting for {prefix!r}")


def command_ping(ser: serial.Serial, timeout: float) -> int:
	write_command(ser, "PING")
	wait_for_prefix(ser, "ROOM PONG", timeout)
	return 0


def command_state(ser: serial.Serial, timeout: float) -> int:
	write_command(ser, "GET STATE")
	line = wait_for_prefix(ser, "ROOM STATE", timeout)
	parse_state(line)
	return 0


def command_set(ser: serial.Serial, property_name: str, value: str, timeout: float) -> int:
	write_command(ser, f"SET {property_name} {value}")
	wait_for_prefix(ser, "ROOM ACK", timeout)
	return 0


def command_watch(ser: serial.Serial) -> int:
	while True:
		line = read_line(ser)
		if line is not None:
			print(line, flush=True)


def build_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Smart Room UART gateway")
	parser.add_argument("--port", required=True, help="Serial port, for example COM5 or /dev/ttyACM0")
	parser.add_argument("--baudrate", type=int, default=DEFAULT_BAUDRATE)
	parser.add_argument("--timeout", type=float, default=DEFAULT_TIMEOUT)
	parser.add_argument(
		"--settle",
		type=float,
		default=DEFAULT_SETTLE,
		help="Seconds to wait after opening the port (default: %(default)s)",
	)

	sub = parser.add_subparsers(dest="command", required=True)
	sub.add_parser("ping")
	sub.add_parser("state")

	set_parser = sub.add_parser("set")
	set_parser.add_argument("property", choices=["occupied", "light_on", "contact_open", "alarm"])
	set_parser.add_argument("value")

	sub.add_parser("watch")
	return parser


def main(argv: list[str] | None = None) -> int:
	args = build_parser().parse_args(argv)
	with open_serial(args.port, args.baudrate, args.settle) as ser:
		if args.command == "ping":
			return command_ping(ser, args.timeout)
		if args.command == "state":
			return command_state(ser, args.timeout)
		if args.command == "set":
			return command_set(ser, args.property, args.value, args.timeout)
		if args.command == "watch":
			return command_watch(ser)
	return 2


if __name__ == "__main__":
	raise SystemExit(main(sys.argv[1:]))
