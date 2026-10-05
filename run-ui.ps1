param([string]$SerialPort = "COM4")
$ErrorActionPreference = "Stop"
& "$PSScriptRoot\tools\qemu\start-qemu-dev.ps1" -SerialPort $SerialPort
$executable = "$PSScriptRoot\desktop-ui\build\smart-room-ui.exe"
if (-not (Test-Path -LiteralPath $executable)) { & "$PSScriptRoot\desktop-ui\build.ps1" }
if (-not (Get-Process smart-room-ui -ErrorAction SilentlyContinue)) {
    Start-Process -FilePath $executable
}
