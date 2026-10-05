# Smart Room v1 interfaces

All messages are ASCII lines terminated by LF; CRLF is accepted. UART is
115200 baud, 8 data bits, no parity, one stop bit, no flow control.
Firmware commands are shorter than 128 bytes. The gateway rejects received
lines exceeding 1024 bytes and never executes a discarded line's suffix.

## Firmware interface (unchanged)

```text
PING
GET STATE
SET light_on 0
SET light_on 1

ROOM PONG
ROOM BOOT proto=1 node=smart_room
ROOM STATE occupied=0 light_on=1 contact_open=0 alarm=none
ROOM EVENT type=light_toggle changed=0x00000002 occupied=0 light_on=1 contact_open=0 alarm=none
ROOM ACK property=light_on changed=1
ROOM ERR code=invalid_command
```

Alarm values: `none`, `warning`, `alarm`. Boolean values: `0`, `1`.
ACK's `changed=0` can mean an idempotent request or a domain rule preventing
the requested change. The gateway checks the subsequent STATE to distinguish
these outcomes.

## TCP interface

Windows `127.0.0.1:5556` forwards to guest TCP 5556. One client is supported.
Client commands are limited to GET STATE and SET light_on 0/1.
Validated ROOM messages are forwarded, followed by gateway metadata:

```text
GATEWAY STATUS online=1 valid=1 pending=0 ready=1
GATEWAY RESULT status=pending reason=awaiting_device
GATEWAY RESULT status=confirmed reason=state_verified
GATEWAY RESULT status=rejected reason=device_rule
GATEWAY RESULT status=timeout reason=outcome_unknown
GATEWAY NOTICE reason=device_timeout
```

- `online`: a valid firmware reply was received within six seconds.
- `valid`: a state was received since the latest known disconnect/reset.
- `pending`: one light request is awaiting a result.
- `ready`: commands may be sent; includes the above checks and recovery state.
- Cached ROOM STATE may precede STATUS during client connection. Consumers
  must use STATUS to decide whether cached values are current.
- PING every two seconds; GET STATE on startup, BOOT, reconnection, and while
  state is unknown.
- A light request expires after three seconds. It is never retried
  automatically. The gateway queries state and temporarily blocks new light
  requests for three seconds to reduce confusion with a late ACK.
- Firmware has no request identifiers. Arbitrarily delayed replies cannot be
  correlated perfectly in v1; confirmation is ACK plus subsequent STATE, not
  a transactional/exactly-once guarantee.
- Qt retries TCP every two seconds and disables controls until fresh status
  arrives. A disconnect during a command leaves its result unknown.
- UART framing, outbound queues, TCP lines and GUI event history are bounded.
  Only changed states are printed to the service log.

The serial bridge retries the same COM port after USB removal. Commands
received while the COM port is absent are discarded, never replayed.
