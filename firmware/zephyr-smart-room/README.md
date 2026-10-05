# Zephyr Smart Room firmware

Target: **nRF52832 DK / PCA10040**, board `nrf52dk/nrf52832`.
Validated SDK baseline: Nordic nRF Connect SDK **v3.2.4**.

In a Windows terminal configured by the nRF Connect SDK toolchain:

```powershell
west build -b nrf52dk/nrf52832 -d build -p always .
west flash -d build
```

Run these commands from this directory. Flashing replaces the board application.
The project selects the J-Link runner. Close QEMU/the UART bridge before direct
COM-port diagnostics. Identify the board's J-Link CDC UART port in Device Manager;
it is not necessarily COM4 on another computer.

| Button | Action |
|---|---|
| 1 | Toggle occupancy |
| 2 | Toggle light |
| 3 | Toggle contact |
| 4 | Cycle alarm |

Room rules can refuse an OFF request while occupied or while the contact is open.
The UART carries `ROOM` protocol messages at 115200 8N1; debug logs use RTT.
See [Protocol](../../doc/protocol.md) for commands and responses.

The Python helper tests validate host-side parsing, not firmware thread timing:

```powershell
python -m pip install -r requirements.txt
python -m pytest -q
```

`src/domain/room` owns transitions. `src/app/runtime` wires threads and zbus.
`src/platform/board` handles GPIO; `src/protocol` handles UART commands/replies.
`src/features/indicators` and `src/diagnostics` consume state/events.
