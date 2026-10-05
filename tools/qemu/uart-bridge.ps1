param([string]$SerialPort = "COM4", [int]$Port = 5555)
$ErrorActionPreference = "Stop"
$projectDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
New-Item -ItemType Directory -Force -Path (Join-Path $projectDir "logs") | Out-Null
$logFile = Join-Path $projectDir "logs\uart-bridge.log"
$serial = $null
$listener = $null
$client = $null
function Close-Serial {
    if ($script:serial) {
        try { $script:serial.Dispose() } catch {}
        $script:serial = $null
    }
}
try {
    $listener = [System.Net.Sockets.TcpListener]::new([System.Net.IPAddress]::Loopback, $Port)
    $listener.Start()
    "Ready: $SerialPort 115200 8N1 <-> 127.0.0.1:$Port" | Set-Content -LiteralPath $logFile
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    while (-not $listener.Pending()) {
        if ([DateTime]::UtcNow -ge $deadline) { throw "QEMU did not connect within 30 seconds." }
        Start-Sleep -Milliseconds 20
    }
    $client = $listener.AcceptTcpClient()
    $client.NoDelay = $true
    $stream = $client.GetStream()
    $buffer = New-Object byte[] 4096
    $nextOpen = [DateTime]::MinValue
    $waitingLogged = $false
    while ($true) {
        if ($client.Client.Poll(0, [System.Net.Sockets.SelectMode]::SelectRead) -and
            $client.Client.Available -eq 0) { break }
        if (-not $serial -and [DateTime]::UtcNow -ge $nextOpen) {
            try {
                $serial = [System.IO.Ports.SerialPort]::new($SerialPort, 115200,
                    [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
                $serial.Handshake = [System.IO.Ports.Handshake]::None
                $serial.ReadTimeout = 200
                $serial.WriteTimeout = 200
                $serial.Open()
                $waitingLogged = $false
                "UART connected: $SerialPort" | Add-Content -LiteralPath $logFile
            } catch {
                Close-Serial
                if (-not $waitingLogged) {
                    "Waiting for $SerialPort; retrying every two seconds." | Add-Content -LiteralPath $logFile
                    $waitingLogged = $true
                }
                $nextOpen = [DateTime]::UtcNow.AddSeconds(2)
            }
        }
        $count = 0
        if ($serial) {
            try {
                $available = [Math]::Min($serial.BytesToRead, $buffer.Length)
                if ($available -gt 0) { $count = $serial.Read($buffer, 0, $available) }
            } catch {
                Close-Serial
                $nextOpen = [DateTime]::UtcNow.AddSeconds(2)
                "UART disconnected." | Add-Content -LiteralPath $logFile
            }
        }
        if ($count -gt 0) { $stream.Write($buffer, 0, $count) }
        if ($stream.DataAvailable) {
            $count = $stream.Read($buffer, 0, $buffer.Length)
            if ($count -eq 0) { break }
            # Never queue commands while the device is absent.
            if ($serial) {
                try { $serial.Write($buffer, 0, $count) }
                catch {
                    Close-Serial
                    $nextOpen = [DateTime]::UtcNow.AddSeconds(2)
                    "UART disconnected during write." | Add-Content -LiteralPath $logFile
                }
            }
        }
        Start-Sleep -Milliseconds 5
    }
    "QEMU disconnected; COM port released." | Add-Content -LiteralPath $logFile
} catch {
    $_.Exception.Message | Add-Content -LiteralPath $logFile
    exit 1
} finally {
    if ($client) { $client.Dispose() }
    if ($listener) { $listener.Stop() }
    Close-Serial
}
