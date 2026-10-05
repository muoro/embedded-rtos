# Linux gateway

Read [Setup](../doc/setup.md), [Architecture](../doc/architecture.md) and
[Protocol](../doc/protocol.md).

Native tests (Linux/WSL, without hardware):

```sh
cmake -S . -B build/native -DBUILD_TESTING=ON
cmake --build build/native --parallel 4
ctest --test-dir build/native --output-on-failure
```

Cross-build after creating the Buildroot toolchain:

```sh
export BUILDROOT_DIR="$HOME/buildroot-2025.02.18"
cmake --preset arm64
cmake --build --preset arm64
```

Copy `.dev.env.example` to `.dev.env`, set both checkout paths and SSH settings,
then open this directory in VS Code WSL. Ctrl+Shift+B invokes `scripts/dev.sh run`:
build, start/reuse QEMU, stop the packaged service, upload to `/tmp`, and run over
SSH. Ctrl+C stops the foreground application. `bash scripts/dev.sh service`
returns to the packaged executable. Buildroot images are updated separately with
`bash scripts/build-image.sh`; a fast application upload is not a persistent image update.
