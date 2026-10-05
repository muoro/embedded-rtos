param([string]$SerialPort = "COM4", [string]$QemuPath = "C:\Program Files\qemu\qemu-system-aarch64.exe", [string]$ImageDirectory)
$ErrorActionPreference = "Stop"
$projectDir = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$qemu = $QemuPath
$configPath = Join-Path $projectDir "runtime.local.json"
if (-not $ImageDirectory -and (Test-Path -LiteralPath $configPath)) {
    $config = Get-Content -LiteralPath $configPath -Raw | ConvertFrom-Json
    $ImageDirectory = $config.ImageDirectory
}
if (-not $ImageDirectory) { $ImageDirectory = Join-Path $projectDir "images" }
if (-not [IO.Path]::IsPathRooted($ImageDirectory)) { $ImageDirectory = Join-Path $projectDir $ImageDirectory }
$ImageDirectory = [IO.Path]::GetFullPath($ImageDirectory)
$kernel = Join-Path $ImageDirectory "Image"
$rootfs = Join-Path $ImageDirectory "rootfs.ext4"
$logDir = Join-Path $projectDir "logs"
$normalizedDisk = $rootfs.Replace('\', '/')
foreach ($file in @($qemu, $kernel, $rootfs)) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) {
        throw "Required file not found: $file"
    }
}
$instances = @(Get-CimInstance Win32_Process -Filter "name = 'qemu-system-aarch64.exe'")
$guest = @($instances | Where-Object {
    $_.CommandLine -and $_.CommandLine.Replace('\', '/').Contains($normalizedDisk)
})
$listener = @(Get-NetTCPConnection -LocalPort 2222 -State Listen -ErrorAction SilentlyContinue)
if ($guest.Count -gt 0) {
    if (@($listener | Where-Object { $_.OwningProcess -in $guest.ProcessId }).Count -gt 0) {
        if (-not ($guest[0].CommandLine.Contains("5556-:5556"))) { throw "Restart QEMU to enable UI port 5556." }
        Write-Output "Using the running QEMU guest."
        return
    }
    throw "This image is already open in QEMU without SSH forwarding. Run poweroff in that guest, then retry."
}
if (Get-NetTCPConnection -LocalPort 5556 -State Listen -ErrorAction SilentlyContinue) { throw "Windows port 5556 is already in use." }
if ($listener.Count -gt 0) { throw "Windows port 2222 is already in use." }
if (Get-NetTCPConnection -LocalPort 5555 -State Listen -ErrorAction SilentlyContinue) {
    throw "Windows port 5555 is already in use. Check the previous UART bridge."
}
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$bridgeScript = Join-Path $PSScriptRoot "uart-bridge.ps1"
$bridge = Start-Process -FilePath "$env:SystemRoot\System32\WindowsPowerShell\v1.0\powershell.exe" -WindowStyle Hidden -PassThru `
    -ArgumentList @("-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass",
        "-File", ('"{0}"' -f $bridgeScript), "-SerialPort", $SerialPort)
$bridgeReady = $false
for ($attempt = 0; $attempt -lt 40; $attempt++) {
    $bridge.Refresh()
    if ($bridge.HasExited) {
        throw ("UART bridge failed: " + (Get-Content -LiteralPath (Join-Path $logDir "uart-bridge.log") -Raw))
    }
    if (Get-NetTCPConnection -LocalPort 5555 -State Listen -ErrorAction SilentlyContinue |
        Where-Object { $_.OwningProcess -eq $bridge.Id }) {
        $bridgeReady = $true
        break
    }
    Start-Sleep -Milliseconds 100
}
if (-not $bridgeReady) {
    Stop-Process -Id $bridge.Id -ErrorAction SilentlyContinue
    throw "UART bridge did not become ready."
}
$consoleLog = Join-Path $logDir "qemu-console.log"
$qemuArgs = @(
    "-M", "virt", "-cpu", "cortex-a53", "-m", "512", "-smp", "1",
    "-display", "none", "-monitor", "none",
    "-kernel", ('"{0}"' -f $kernel),
    "-append", '"rootwait root=/dev/vda console=ttyAMA0"',
    "-drive", ('"file={0},if=none,format=raw,id=hd0"' -f $rootfs),
    "-device", "virtio-blk-device,drive=hd0",
    "-serial", ('"file:{0}"' -f $consoleLog),
    "-serial", "tcp:127.0.0.1:5555",
    "-netdev", "user,id=net0,hostfwd=tcp:127.0.0.1:2222-:22,hostfwd=tcp:127.0.0.1:5556-:5556",
    "-device", "virtio-net-device,netdev=net0"
)
try {
    $process = Start-Process -FilePath $qemu -ArgumentList $qemuArgs -WindowStyle Hidden -PassThru
    Start-Sleep -Seconds 2
    $process.Refresh()
    if ($process.HasExited) { throw "QEMU exited during startup." }
} catch {
    Stop-Process -Id $bridge.Id -ErrorAction SilentlyContinue
    throw
}
Write-Output "Started QEMU (PID $($process.Id)); SSH 2222, UART bridge $SerialPort -> ttyAMA1."
