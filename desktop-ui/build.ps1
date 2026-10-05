param([switch]$Run)
$ErrorActionPreference = "Stop"
$ui = $PSScriptRoot
$qt = if ($env:QT_ROOT) { $env:QT_ROOT } else { "C:\Qt\6.8.3\mingw_64" }
$mingw = if ($env:MINGW_ROOT) { $env:MINGW_ROOT } else { "C:\Qt\Tools\mingw1310_64" }
foreach ($file in @("$qt\bin\qmake.exe", "$mingw\bin\g++.exe")) {
    if (-not (Test-Path -LiteralPath $file)) { throw "Missing Qt/MinGW kit file: $file" }
}
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$ninja = (Get-Command ninja -ErrorAction Stop).Source
$oldPath = $env:PATH
try {
    $env:PATH = "$qt\bin;$mingw\bin;$oldPath"
    & $cmake -S $ui -B "$ui\build" -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja" "-DCMAKE_CXX_COMPILER=$mingw\bin\g++.exe" "-DCMAKE_PREFIX_PATH=$qt" -DCMAKE_BUILD_TYPE=Release
    if ($LASTEXITCODE -ne 0) { throw "Qt configure failed" }
    & $cmake --build "$ui\build" --parallel 4
    if ($LASTEXITCODE -ne 0) { throw "Qt build failed" }
    & "$qt\bin\windeployqt.exe" --release --qmldir "$ui\qml" --no-translations "$ui\build\smart-room-ui.exe"
    if ($LASTEXITCODE -ne 0) { throw "Qt deployment failed" }
    if ($Run) { Start-Process -FilePath "$ui\build\smart-room-ui.exe" }
} finally { $env:PATH = $oldPath }
