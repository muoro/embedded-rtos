#!/usr/bin/env bash
set -euo pipefail
app="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
buildroot="${BUILDROOT_DIR:-$app/../../buildroot-2025.02.18}"
buildroot="$(realpath "$buildroot")"
case "$buildroot" in /mnt/*) echo 'Build Buildroot in the Linux filesystem, not /mnt.' >&2; exit 1;; esac
[[ -f "$buildroot/Makefile" ]] || { echo 'Set BUILDROOT_DIR to Buildroot 2025.02.18.' >&2; exit 1; }
# Avoid Windows executables inherited through WSL PATH.
export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
make -C "$buildroot" BR2_EXTERNAL="$app/buildroot-external" qemu_aarch64_virt_dev_defconfig
make -C "$buildroot" device-gateway-dirclean
make -C "$buildroot" BR2_JLEVEL="${JOBS:-4}"
echo "Images ready in $buildroot/output/images. Shut down QEMU before replacing its disk."
