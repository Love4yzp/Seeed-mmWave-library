# Seeed Arduino mmWave Library For XIAO

> **Status (2026-04-19):** `main` is still the shipped **v1.0.0**. Two newer
> versions are staged as GitHub prereleases and **have not yet been validated
> on real hardware** — do not rely on them in production yet.
>
> - [**v1.1.0-rc1**](https://github.com/Love4yzp/Seeed-mmWave-library/releases/tag/v1.1.0-rc1)
>   (branch `release/1.1.0-rc`) — housekeeping only: example reorg,
>   optional `extras/` helpers for the onboard BH1750 / WS2812, fixed
>   metadata, rewritten README. No public API changes.
> - [**v2.0.0-rc1**](https://github.com/Love4yzp/Seeed-mmWave-library/releases/tag/v2.0.0-rc1)
>   (branch `v2-dev`) — adds the event callback API (`onBreathRate`,
>   `onFall`, `onError`, …), a `Status` enum and `read*()` siblings to
>   every getter, a `stats()` snapshot, a split-out Arduino-free
>   `FrameCodec` with host-side unit tests, and GitHub Actions CI. One
>   small breaking change: `fetch()` / `fetchType()` /
>   `processQueuedFrames()` were lowered to `protected`.
>
> Both prereleases are flagged `prerelease: true`, so **Arduino Library
> Manager will not surface them** and existing 1.0.0 installs are
> unaffected. Roadmap and deferred work live in
> [`ROADMAP.md`](https://github.com/Love4yzp/Seeed-mmWave-library/blob/v2-dev/ROADMAP.md)
> on the `v2-dev` branch.

## Introduction

This Library is designed to interface with the Seeed Studio XIAO ESP32-C6 Board integrated on the [MR60BHA2-XIAO 60GHz mmWave Human Breathing and Heartbeat Sensor](https://www.seeedstudio.com/MR60BHA2-60GHz-mmWave-Sensor-Breathing-and-Heartbeat-Module-p-5945.html) and [MR60FDA2-XIAO 60GHz mmWave Human Fall Detection Sensor](https://www.seeedstudio.com/MR60FDA2-60GHz-mmWave-Sensor-Fall-Detection-Module-p-5946.html)  sensors. This library enables easy data reading and additional operations for functionalities such as breathing and heartbeat monitoring, and fall detection.

### Supported Seeed Studio Devices

| Device                           | Functionality                  |
| -------------------------------- | ------------------------------ |
| [**MR60BHA2** 60GHz mmWave Sensor](https://www.seeedstudio.com/MR60BHA2-60GHz-mmWave-Sensor-Breathing-and-Heartbeat-Module-p-5945.html) | Breathing and Heartbeat Module |
| [**MR60FDA2** 60GHz mmWave Sensor](https://www.seeedstudio.com/MR60FDA2-60GHz-mmWave-Sensor-Fall-Detection-Module-p-5946.html) | Fall Detection Module          |

## Installation

1. Download the library from GitHub or the Arduino Library Manager.
2. Open the Arduino IDE.
3. Go to `Sketch` > `Include Library` > `Add .ZIP Library...` and select the downloaded file.

## Usage

To begin using the library, include it in your Arduino sketch and refer to the examples folder.

```cpp
#include "Seeed_Arduino_mmWave.h"
```

### Examples

- **GroveU8x8:** Demonstrates how to utilize Grove GPIO pins to interface with the Grove - OLED Display 0.96" using the U8x8 library. This example shows basic text display functions.

- **LightRGB:** Provides an example of controlling an RGB LED.

- **mmWaveBreath:** Demonstrates how to use the MR60BHA2 sensor for monitoring breathing and heartbeat. It covers initializing the sensor, fetching data, and displaying breathing rate, heart rate, and phase information.

- **mmWaveFall:** Illustrates how to use the MR60FDA2 sensor for fall detection. This example includes initializing the sensor and processing fall detection data to trigger alerts.

- **ReadByte:** A basic example that reads bytes of data from a sensor or serial input and displays the raw data. Useful for debugging and understanding data formats.

- **ReadLuxValue:** Shows how to read lux (light intensity) values from the onboard light sensor. It demonstrates sensor initialization, reading, and printing lux values.

## API Reference

Please refer to the comments in the source code for detailed information on the available methods and parameters.

## License

This library is released under the [MIT License](https://github.com/love4yzp/Seeed-mmWave-library/blob/main/LICENSE).

## Contributing
Contributions to the library are welcome. Please follow the standard pull request process to suggest improvements or add new features.
