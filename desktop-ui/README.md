# Qt/QML dashboard

![Smart Room dashboard](../doc/images/smart-room-ui.png)

Windows Qt **6.8.3 MinGW 64-bit**, matching **MinGW 13.1**, CMake and Ninja were
used for the development baseline. `build.ps1` accepts `QT_ROOT` and `MINGW_ROOT`
environment overrides instead of requiring these installations in fixed locations.

```powershell
.\build.ps1 -Run
```

The dashboard connects to `127.0.0.1:5556`. `GatewayConnection` owns the socket and
reconnect timer; `GatewayProtocol` parses messages; `RoomViewModel` exposes state
to QML. The UI is a Windows application, separate from the Linux image.

To include tests, configure with `-DBUILD_TESTING=ON` using the same Qt/MinGW kit,
then build and run `ctest --test-dir build --output-on-failure` with the Qt and
MinGW `bin` directories on PATH. Qt Test must be installed. Tests use offscreen
rendering and a local TCP peer, without physical hardware.
