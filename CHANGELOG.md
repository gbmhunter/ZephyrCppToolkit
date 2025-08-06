# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Added ADC peripheral classes.
- Added Watchdog (hardware watchdog) peripheral classes.
- Added unit tests for the event thread and timer manager classes.
- Added name parameter to Timer constructor for logging/debugging purposes.

### Fixed

- Fixed bug in EventThread::waitForEvent() where multiple calls to `getNextExpiringTimer()` where sometimes occuring when they should not have been.
- Fixed bug where `getNextExpiringTimer()` was updating the timers next expiry time when it should not have been.

### Changed

- GPIO example is now built for the nRF52840 DK board so that the example can demo real GPIO peripherals.
- Zephyr SDK version is now pinned to `v0.17.0` in the CI.
- Zephyr SDK toolchains now specified when installing to reduce download size.

## [1.0.0] - 2025-07-09

### Added

- Initial release.
- Added GPIO real/mock classes.
- Added PWM real/mock classes.
- Added Event loop, timer and timer manager classes.
- Added license file.

[unreleased]: https://github.com/gbmhunter/ZephyrCppToolkit/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/gbmhunter/ZephyrCppToolkit/releases/tag/v1.0.0