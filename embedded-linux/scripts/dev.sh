#!/usr/bin/env bash
set -euo pipefail

project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$project_dir"
if [[ ! -f .dev.env ]]; then
    echo "Missing .dev.env. See README.md for the local development setup." >&2
    exit 1
fi
source .dev.env
export BUILDROOT_DIR
: "${BUILDROOT_DIR:?Set BUILDROOT_DIR in .dev.env}"

ssh_options=(
    -F NUL -p "${SSH_PORT:-2222}"
    -i "$SSH_KEY"
    -o "UserKnownHostsFile=$SSH_KNOWN_HOSTS"
    -o StrictHostKeyChecking=accept-new
    -o IdentitiesOnly=yes -o BatchMode=yes
    -o ConnectTimeout=3 -o LogLevel=ERROR
)

ensure_guest() {
    "$WINDOWS_POWERSHELL" -NoProfile -NonInteractive -ExecutionPolicy Bypass \
        -File "$(wslpath -w "$WINDOWS_PROJECT/tools/qemu/start-qemu-dev.ps1")"
    mkdir -p build/dev
    for ((attempt=1; attempt<=30; attempt++)); do
        if "$WINDOWS_SSH" "${ssh_options[@]}" -T root@127.0.0.1 true \
            2>build/dev/ssh-error.log; then
            return
        fi
        sleep 1
    done
    cat build/dev/ssh-error.log >&2
    echo "SSH is not ready. Check the QEMU logs in $WINDOWS_PROJECT/logs." >&2
    return 1
}

case "${1:-run}" in
    run)
        echo "Building the ARM64 application..."
        cmake --preset arm64
        cmake --build --preset arm64
        ensure_guest
        "$WINDOWS_SSH" "${ssh_options[@]}" -T root@127.0.0.1 '[ ! -x /etc/init.d/S60device-gateway ] || /etc/init.d/S60device-gateway stop; killall device-gateway 2>/dev/null || true'
        echo "Uploading to the running guest as /tmp/device-gateway..."
        "$WINDOWS_SSH" "${ssh_options[@]}" -T root@127.0.0.1 \
            'cat > /tmp/device-gateway.upload && chmod 755 /tmp/device-gateway.upload && mv -f /tmp/device-gateway.upload /tmp/device-gateway' \
            < build/arm64/device-gateway
        echo "--- QEMU application output ---"
        tty_option=-T
        if [[ -t 0 && -t 1 ]]; then
            tty_option=-tt
        fi
        exec "$WINDOWS_SSH" "${ssh_options[@]}" "$tty_option" root@127.0.0.1 /tmp/device-gateway --log-stderr
        ;;
    service)
        ensure_guest
        exec "$WINDOWS_SSH" "${ssh_options[@]}" -T root@127.0.0.1 'killall device-gateway 2>/dev/null || true; sleep 1; rm -f /var/run/device-gateway.pid; /etc/init.d/S60device-gateway start'
        ;;
    shell)
        ensure_guest
        exec "$WINDOWS_SSH" "${ssh_options[@]}" -tt root@127.0.0.1
        ;;
    stop)
        "$WINDOWS_SSH" "${ssh_options[@]}" -T root@127.0.0.1 poweroff
        ;;
    *)
        echo "Usage: $0 [run|service|shell|stop]" >&2
        exit 2
        ;;
esac
