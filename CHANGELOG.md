# Changelog

All notable changes to this project are documented here. The project follows
[Semantic Versioning](https://semver.org/).

## [1.1.0] — 2026-04-19

Housekeeping release. No public API breakage; existing sketches continue to
compile and run unchanged.

### Added
- `src/extras/OnboardIndicator.h` — optional thin wrapper around the onboard
  WS2812 RGB indicator (defaults to `D1`, single pixel). Including this header
  is what pulls in `Adafruit_NeoPixel`; the core driver remains independent.
- `src/extras/OnboardAmbientLight.h` — optional thin wrapper around the
  onboard BH1750 ambient-light sensor. Including this header is what pulls
  in `hp_BH1750`; the core driver remains independent.
- `examples/mmwave_fall_detection/` — minimal MR60FDA2 example matching the
  existing `mmwave_breath_heartrate` example for MR60BHA2.
- `CHANGELOG.md` (this file).

### Changed
- All example folders renamed to the new naming convention
  (`mmwave_*`, `onboard_*`, `grove_*`, `integration_*`):
  - `mmwaveBreath`         → `mmwave_breath_heartrate`
  - `PeopleCounting_demo`  → `mmwave_people_counting`
  - `ReadByte`             → `mmwave_diagnostics`
  - `ReadLuxValue`         → `onboard_ambient_light`
  - `LightRGB`             → `onboard_indicator_led`
  - `Grove_U8x8`           → `grove_oled_display`
  - `breath_demo`          → `integration_breath_monitor`
  - `fall_demo`            → `integration_fall_alert`
- `integration_breath_monitor` and `integration_fall_alert` now use the new
  `extras/OnboardIndicator` and `extras/OnboardAmbientLight` helpers instead
  of instantiating `Adafruit_NeoPixel` / `hp_BH1750` by hand.
- `onboard_indicator_led` and `onboard_ambient_light` likewise updated to
  use the extras helpers, demonstrating the idiomatic access pattern.
- `library.properties` / `library.json` platform metadata tightened from
  `*` to `esp32` / `espressif32` to match the existing
  `#error "…only supports ESP32"` guard.
- `library.properties` `paragraph` rewritten to call out the zero-dependency
  core and the optional nature of `extras/`.
- `library.json` `description` updated similarly and `xiao`/`esp32` added to
  `keywords`.
- The old `_VERSION_MMWAVEBREATH_0_0_1` macro in `Seeed_Arduino_mmWave.h`
  replaced by `SEEED_ARDUINO_MMWAVE_VERSION`.
- README replaced with a real reference: hardware overview, pin map, frame
  protocol, full API surface, example index, troubleshooting, and a
  roadmap section for the upcoming v2.0 changes.

### Removed
- **Dead code cleanup** (no user-facing impact — these identifiers had zero
  references in `src/` and `examples/`):
  - The `MMWAVE_DEVICE` enum (including the wrongly-named
    `MMWAVE_FALL_MR60BHA2` and `MMWAVE_BREATH_MR60FDA2` members) has been
    removed from `Seeed_Arduino_mmWave.h`.
  - The unused `MAX_QUEUE_SIZE` macro has been removed from
    `SeeedmmWave.h`.
  - The empty trailing `#ifndef ESP32` block in `SeeedmmWave.h` has been
    removed.

### Changed (internal)
- `MMWaveMaxQueueSize` is now defined in terms of `MMWAVE_QUEUE_CAPACITY`,
  which users can override via a compile-time `-D` flag. The default
  value (120) is unchanged. `MMWaveMaxQueueSize` remains as a back-compat
  alias and will be removed in v2.0.

## [1.0.0]

Initial public release.
