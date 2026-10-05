# Architecture

## State ownership

The nRF firmware owns room state and applies room rules. In particular, occupancy
or an open contact forces the light on. Linux caches reported state; it does not
pretend that a requested command has already changed the hardware.

## Linux responsibilities

| Class/module | Responsibility |
|---|---|
| `GatewayApplication` | Own event loop; connect UART, controller, server and logger |
| `SerialLink` | Open/reopen raw 115200 8N1 UART; queue asynchronous writes |
| `LineFramer` / protocol | Assemble bounded lines and validate messages |
| `Controller` | State validity, heartbeat, pending command and recovery |
| `TcpServer` / `TcpSession` | One client, framed commands and queued responses |
| `Logger` | Severity filtering and safe, bounded syslog messages |

One process and one Asio event loop handle UART, TCP and timers. No worker
threads or shared-memory IPC are needed for the current workload. Blocking or
CPU-intensive work must not be added to event handlers without revisiting this.

## Zephyr responsibilities

GPIO input events feed the room-domain thread. The domain owns transitions and
publishes state/events through zbus. Indicator, diagnostics and UART protocol
handling are separated. UART commands enter the domain through a message queue.
Diagnostic output uses RTT, leaving the UART for the ROOM protocol.

## Transport

| Endpoint | Purpose |
|---|---|
| Windows COM port | J-Link CDC UART connected to the physical nRF |
| Windows loopback TCP 5555 | Byte transport from the bridge to QEMU's second UART |
| Linux `/dev/ttyAMA1` | Device opened by the gateway |
| Windows loopback 5556 -> guest 5556 | Dashboard connection |
| Windows loopback 2222 -> guest 22 | Development SSH |

Only one application may own the UART. Stop the gateway before using `cat`,
`tio`, or another diagnostic reader. Close QEMU/the bridge before opening the
physical COM port directly in another Windows application.

## Logging

The gateway sends `daemon` facility messages to syslog. BusyBox syslogd writes
`/var/log/messages`; the image maps `/var/log` to `/tmp` on tmpfs. Logs are volatile,
with a 256 KiB rotation threshold and one backup file. `info` is the default;
`debug` adds raw UART RX/TX. The development command adds `--log-stderr` so SSH
also carries logs into the VS Code terminal.

Set `LOG_LEVEL` in `/etc/default/device-gateway` and restart the service to change
verbosity. Change the overlay copy for future image builds. Root SSH and loopback
networking are development conveniences, not a production security configuration.
