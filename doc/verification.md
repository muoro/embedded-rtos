# Verification

## Repository preparation — 2026-10-05

The unified source snapshot was built in separate Windows and WSL directories.
The previous working installation and firmware repository were retained.

| Check | Result |
|---|---|
| Native C++ protocol/controller suite | Passed |
| Real executable with pseudo-UART and TCP peer | Passed |
| Syslog severity/stderr integration | Passed |
| ARM64 cross-compilation with Buildroot GCC 13.4 | Passed |
| External-tree defconfig in a separate Buildroot output directory | Passed |
| Windows Qt 6.8.3/MinGW 13.1 build and Qt UI/network suite | Passed |
| Firmware Python helper parser tests | 3 passed |
| nRF52832 firmware build, NCS v3.2.4 | Passed |
| PowerShell/Bash script syntax | Passed |

Firmware linker report: 41,376 bytes flash and 25,120 bytes static RAM allocation
on the 512 KiB flash / 64 KiB RAM target. This is not runtime peak-memory profiling.
NCS reports a non-fatal BOOT_BANNER configuration warning because it supplies its
own banner. Qt deployment reports missing optional DirectX compiler DLLs; the
offscreen software-rendered test suite passes. Other graphics backends need
validation on the deployment PC.

## Existing system evidence

Before this repository consolidation, the same application stack was run using
Windows QEMU and a physical nRF52832. UART state changes and light control were
observed through the gateway/dashboard. On 2026-10-01 the Buildroot service was
verified to start at boot and write `daemon.info`/`daemon.warn` syslog messages;
RAM-backed logging and rotation arguments were checked.

## Boundaries of this validation

- Repository preparation did not flash the board or repeat physical button tests.
- The ARM64 app was cross-built using the existing Buildroot toolchain. A full
  clean toolchain/kernel/rootfs rebuild was not repeated for this import.
- Native integration tests simulate protocol behavior, not MCU electrical or
  timing behavior. Firmware Python tests cover a host helper, not Zephyr threads.
- No custom kernel driver, physical ARM64 board bring-up, secure boot, production
  hardening, or real-time timing guarantees are claimed.

## Repeatable checks

Use the commands in the root README and each component README. For hardware:
close Qt, ensure the room is vacant and the contact is closed, and run
`python tools/tests/test_hardware.py`. Then test all four buttons, a board reset,
USB removal/reconnection and QEMU restart. Record the firmware/image revisions,
board target and observed behavior when documenting new results.
