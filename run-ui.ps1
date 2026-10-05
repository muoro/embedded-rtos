param([string]$SerialPort = "COM4")
$ErrorActionPreference = "Stop"
$configPath = Join-Path $PSScriptRoot "runtime.local.json"
$config = if (Test-Path -LiteralPath $configPath) {
    Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
} else { [pscustomobject]@{} }
$sshKey = if ($config.SshKey) { $config.SshKey } else { Join-Path $env:USERPROFILE ".ssh\embedded-rtos_ed25519" }
$knownHosts = if ($config.SshKnownHosts) { $config.SshKnownHosts } else { Join-Path $env:USERPROFILE ".ssh\embedded-rtos_known_hosts" }
if (-not (Test-Path -LiteralPath $sshKey -PathType Leaf)) {
    throw "SSH key not found: $sshKey. Configure runtime.local.json; see doc/setup.md."
}
$ssh = (Get-Command ssh.exe -ErrorAction Stop).Source
$sshOptions = @("-F", "NUL", "-p", "2222", "-i", $sshKey,
    "-o", "UserKnownHostsFile=$knownHosts", "-o", "StrictHostKeyChecking=accept-new",
    "-o", "IdentitiesOnly=yes", "-o", "BatchMode=yes", "-o", "ConnectTimeout=2")
& "$PSScriptRoot\tools\qemu\start-qemu-dev.ps1" -SerialPort $SerialPort
Write-Output "Waiting for Linux SSH..."
New-Item -ItemType Directory -Force -Path "$PSScriptRoot\logs" | Out-Null
$sshLog = Join-Path $PSScriptRoot "logs\launcher-ssh.log"
$guestReady = $false
for ($attempt = 0; $attempt -lt 20; $attempt++) {
    & $ssh @sshOptions root@127.0.0.1 true 2> $sshLog
    if ($LASTEXITCODE -eq 0) { $guestReady = $true; break }
    Start-Sleep -Milliseconds 500
}
if (-not $guestReady) { throw "Linux SSH is unavailable. See $sshLog" }
# Preserve an active development instance; never open the UART twice.
$ensureGateway = 'if ! pidof device-gateway >/dev/null; then /etc/init.d/S60device-gateway start || exit 1; fi; for attempt in 1 2 3 4 5; do if pidof device-gateway >/dev/null && netstat -ltn | grep -q ":5556 "; then exit 0; fi; sleep 1; done; echo "Gateway did not become ready; inspect /var/log/messages" >&2; exit 1'
& $ssh @sshOptions root@127.0.0.1 $ensureGateway
if ($LASTEXITCODE -ne 0) { throw "Linux gateway startup failed; the GUI was not launched." }
Write-Output "Linux gateway is ready on TCP 5556."
$executable = "$PSScriptRoot\desktop-ui\build\smart-room-ui.exe"
if (-not (Test-Path -LiteralPath $executable)) { & "$PSScriptRoot\desktop-ui\build.ps1" }
$runningUi = @(Get-Process smart-room-ui -ErrorAction SilentlyContinue)
if ($runningUi | Where-Object { $_.Path -ne $executable }) {
    throw "An older GUI is open. Close it, then run this command again."
}
if (-not $runningUi) {
    Start-Process -FilePath $executable -WindowStyle Normal
}
Write-Output "Smart Room UI is running. Device state appears when UART communication is available."
