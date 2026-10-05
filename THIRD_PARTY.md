# Third-party software

## Vendored

- Standalone Asio 1.36.0: `embedded-linux/third_party/asio/`.
  Distributed under the Boost Software License 1.0; the upstream license is
  preserved in `COPYING`, with source/version information in `VERSION.md`.

## External build/runtime dependencies

Buildroot, the Linux kernel, BusyBox, Dropbear, Zephyr/Nordic nRF Connect SDK,
Qt, QEMU and the respective toolchains are installed or built separately.
They are not relicensed by this project. Respect their own licenses when
redistributing binaries or a complete firmware/Linux image.

Generated images and SDK installations are deliberately not committed here.
For a distributed Buildroot image, use Buildroot's `make legal-info` and review
the license obligations of every included component.
