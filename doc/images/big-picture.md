# Big-picture illustration

Updated 2026-10-05 from the original illustrated development-roadmap layout,
using the imagegen tool. All labels are English. Boards and application windows
are illustrative; the actual GUI screenshot is in the desktop component README.

## Diagram constraints

- Physical nRF52832 DK with Zephyr is outside the Windows host boundary.
- WSL2/Ubuntu is the build environment; Buildroot produces the ARM64 kernel,
  root filesystem and application/toolchain outputs.
- The Image/rootfs loading arrow ends at QEMU, never at Qt.
- Windows runs QEMU; Linux and the C++ gateway run inside QEMU.
- The physical COM port is bridged via loopback TCP 5555 to QEMU's virtual UART.
  Linux reads `/dev/ttyAMA1` at 115200 8N1.
- Qt runs on Windows and communicates with the Linux gateway via forwarded TCP 5556.
- Linux boot, nRF state reception and Qt control are implemented.
- STM32/FreeRTOS and additional nodes are planned, indicated by dashed connections.

Preserve the two-lane layout (develop/build above, run/visualize below), hardware
on the left, navy text, turquoise/amber lanes and generous spacing when revising.
