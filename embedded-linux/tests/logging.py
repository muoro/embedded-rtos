#!/usr/bin/env python3
"""Check severity filtering and stderr mirroring through the real executable."""
import os
import pty
import select
import socket
import subprocess
import sys

binary = sys.argv[1]
for level in ("debug", "info", "warning", "error"):
    master, slave = pty.openpty()
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    process = subprocess.Popen(
        [binary, "--serial", os.ttyname(slave), "--port", str(port),
         "--log-stderr", "--log-level", level],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    )
    try:
        assert select.select([master], [], [], 5)[0], "UART did not open"
        assert b"PING" in os.read(master, 4096)
    finally:
        process.terminate()
        stdout, stderr = process.communicate(timeout=5)
        os.close(master)
        os.close(slave)
    output = stderr.decode()
    assert process.returncode == 0, output
    assert stdout == b""
    assert ("UART TX: PING" in output) == (level == "debug"), output
    assert ("UART opened:" in output) == (level in ("debug", "info")), output
    assert ("GATEWAY STATUS" in output) == (level != "error"), output

invalid = subprocess.run([binary, "--log-level", "invalid"], capture_output=True)
assert invalid.returncode != 0
assert b"Invalid log level" in invalid.stderr
print("PASS: severity filtering, stderr mirror, clean stdout, invalid level")
