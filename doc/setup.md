# Setup: Windows + WSL2 + QEMU

## 1. Checkouts and prerequisites

Use two checkouts of the same repository: one in Windows for Qt/firmware and one
under your WSL Linux home for Linux builds. Commit/push/pull source changes to
keep them synchronized. Do not copy build directories between them.

Example locations used below:

- Windows: `C:\embedded-rtos`
- WSL: `~/embedded-rtos`
- Buildroot: `~/buildroot-2025.02.18` (separate download, not a submodule)

Windows needs QEMU with `qemu-system-aarch64`, OpenSSH client, Git, Qt 6.8.3 with
matching MinGW, CMake/Ninja, and the nRF Connect SDK v3.2.4 toolchain for firmware.
The baseline used QEMU's ARM64 `virt` machine with Cortex-A53 and 512 MiB RAM.
Check `qemu-system-aarch64 -machine help` for `virt` before proceeding.

In WSL Ubuntu 24.04:

```sh
sudo apt update
sudo apt install build-essential bash bc binutils bison bzip2 cpio file flex \
  g++ gcc git gzip make patch perl python3 python3-pytest python3-serial rsync \
  sed tar unzip wget curl xz-utils libncurses-dev libssl-dev cmake ninja-build
cd ~
git clone https://github.com/muoro/embedded-rtos.git
curl -fLO https://buildroot.org/downloads/buildroot-2025.02.18.tar.xz
tar -xf buildroot-2025.02.18.tar.xz
export BUILDROOT_DIR="$HOME/buildroot-2025.02.18"
```

Use the official Buildroot download/signature information to verify your download.
Keep the build on the Linux filesystem, not under `/mnt/c`. The first build
includes the toolchain and kernel and is substantially slower than app rebuilds.

## 2. Provision development SSH

Create a dedicated SSH key in Windows PowerShell; keep the private key outside
the repository. Choose an appropriate passphrase/key-agent setup for your use.
The current fast-development script uses `BatchMode=yes`, so it needs a key that
can be used noninteractively (for example through the Windows SSH agent).

```powershell
ssh-keygen -t ed25519 -f "$env:USERPROFILE\.ssh\embedded-rtos_ed25519"
```

In WSL, put **only the public key** in the ignored overlay. Replace YOUR_USER:

```sh
cd ~/embedded-rtos/embedded-linux
overlay=buildroot-external/board/qemu-arm64/dev-overlay
mkdir -p "$overlay/root/.ssh"
cp /mnt/c/Users/YOUR_USER/.ssh/embedded-rtos_ed25519.pub "$overlay/root/.ssh/authorized_keys"
chmod 700 "$overlay/root/.ssh"
chmod 600 "$overlay/root/.ssh/authorized_keys"
cp .dev.env.example .dev.env
```

Edit `.dev.env`: set `BUILDROOT_DIR`, Windows checkout path, Windows SSH key and
known-hosts file. Dropbear password authentication is disabled. Host keys are
generated on the guest; a newly rebuilt image may have a new host identity.
After verifying that you replaced your local guest, remove its obsolete entry
from the dedicated known-hosts file with `ssh-keygen -R '[127.0.0.1]:2222' -f FILE`.
Do not disable host-key checking globally. Never commit keys or generated images.

## 3. Build and transfer the Linux image

In WSL:

```sh
cd ~/embedded-rtos/embedded-linux
export BUILDROOT_DIR="$HOME/buildroot-2025.02.18"
bash scripts/build-image.sh
```

Before replacing an existing image, stop QEMU cleanly (`poweroff` in the guest,
or `bash scripts/dev.sh stop`) and wait for its process to exit. Then copy:

```sh
mkdir -p /mnt/c/embedded-rtos/images
cp -L "$BUILDROOT_DIR/output/images/Image" \
      "$BUILDROOT_DIR/output/images/rootfs.ext4" /mnt/c/embedded-rtos/images/
```

The defconfig matches QEMU's ARM64 `virt` machine. Do not use an arbitrary board
image. Buildroot supplies the cross-toolchain; CMake does not use Windows MinGW
for the Linux gateway.

## 4. Firmware and QEMU

Build/flash the [Zephyr firmware](../firmware/zephyr-smart-room/README.md) with
`nrf52dk/nrf52832`. Determine the actual J-Link CDC UART COM port.

In Windows PowerShell:

```powershell
cd C:\embedded-rtos
.\tools\qemu\start-qemu-dev.ps1 -SerialPort COM4
```

`-QemuPath` can override the default QEMU executable location. This starts a
hidden UART bridge and QEMU. It does not launch the GUI. The bridge retries if
the board is absent; the Linux gateway then remains offline until valid data
arrives. Ports 2222, 5555 and 5556 must be free. Stop any older project instance
first; do not run two QEMU guests with the same port forwards or image.

In WSL, once `.dev.env` is configured:

```sh
cd ~/embedded-rtos/embedded-linux
bash scripts/dev.sh shell
```

In the **QEMU guest**:

```sh
uname -m
cat /etc/os-release
pidof device-gateway
tail -f /var/log/messages
```

Expect `aarch64`, Buildroot 2025.02.18 and a running gateway service. Board state
appears only when the physical link and firmware protocol are working.

## 5. Qt and development loop

In Windows PowerShell (CMake and Ninja on PATH):

```powershell
cd C:\embedded-rtos
.\desktop-ui\build.ps1
.\run-ui.ps1 -SerialPort COM4
```

The launcher starts or reuses QEMU, waits for SSH, starts the gateway if no
instance is running, verifies its TCP listener, and opens the GUI. An existing
development gateway is preserved to avoid competing UART readers.

By default it uses `images/`, and the `embedded-rtos_ed25519` and
`embedded-rtos_known_hosts` files under your Windows `.ssh` directory. To reuse
an existing local installation, create `runtime.local.json` in the repository
root (ignored by Git):

```json
{
  "ImageDirectory": "C:\\linux-nrf-stm\\images",
  "SshKey": "C:\\Users\\YOUR_USER\\.ssh\\linux-nrf-stm_ed25519",
  "SshKnownHosts": "C:\\Users\\YOUR_USER\\.ssh\\linux-nrf-stm_known_hosts"
}
```

This references the existing images without copying an active QEMU disk. The
QEMU helper also accepts `-ImageDirectory` to override the configured location.

Open `~/embedded-rtos/embedded-linux` in VS Code WSL. Ctrl+Shift+B runs the app-only
build/upload/run cycle. It stops the boot service first to avoid two UART owners.
Edits to Linux configuration, packages or the overlay require an image rebuild;
C++ app edits usually require only the fast development cycle.

## 6. Tests

- Native Linux tests: commands in the root README.
- Firmware-side Python helper tests: `python -m pytest -q` in the firmware folder.
- Qt tests: see the desktop README.
- Hardware smoke test: close Qt, make the room vacant and the contact closed,
  then run `python tools/tests/test_hardware.py` on Windows. It changes the light
  and restores its initial state. Only one TCP client is supported.

GitHub CI does not replace physical-board tests, QEMU boot tests or hardware
timing measurements. Do not describe the native test peer as full nRF emulation.
