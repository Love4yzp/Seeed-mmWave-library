# Seeed Arduino mmWave Library

Drivers for Seeed 60 GHz mmWave sensor modules on Seeed XIAO ESP32 boards —
**MR60BHA2** (breathing & heartbeat) and **MR60FDA2** (fall detection).

The default factory combo is the **Seeed XIAO ESP32-C6** carrier paired with
one of the mmWave modules. The library itself works on any XIAO ESP32 series
board (C6 / S3 / C3 / S2) as long as the UART pins are wired through.

## Hardware overview

Each mmWave module carries more than just the 60 GHz radar IC:

| Component        | Purpose                                                    |
| ---------------- | ---------------------------------------------------------- |
| 60 GHz mmWave IC | Breathing/heartbeat (MR60BHA2) or fall detection (MR60FDA2)|
| BH1750           | Onboard ambient-light sensor (1–65,535 lux, I²C)           |
| WS2812           | Onboard programmable RGB indicator LED                     |
| Grove port       | Two GPIO lines + V + G, for optional I²C/UART/digital add-ons |

| Device                                                                                         | Purpose                    |
| ---------------------------------------------------------------------------------------------- | -------------------------- |
| [**MR60BHA2**](https://www.seeedstudio.com/MR60BHA2-60GHz-mmWave-Sensor-Breathing-and-Heartbeat-Module-p-5945.html) | Breathing and heartbeat    |
| [**MR60FDA2**](https://www.seeedstudio.com/MR60FDA2-60GHz-mmWave-Sensor-Fall-Detection-Module-p-5946.html)         | Fall detection             |

## Installation

**Arduino IDE (Library Manager):** Search for `Seeed Arduino mmWave` and click
Install. `Adafruit NeoPixel` and `hp_BH1750` will be pulled in automatically;
they are only used by the *optional* helpers under `src/extras/` and are not
referenced by the core driver.

**Manual:** Download the repository as a ZIP and use *Sketch → Include Library
→ Add .ZIP Library…*.

**PlatformIO:** Add `seeed-studio/Seeed Arduino mmWave` to `lib_deps`.

## Quick start

```cpp
#include <Arduino.h>
#include "Seeed_Arduino_mmWave.h"

HardwareSerial mmWaveSerial(0);
SEEED_MR60BHA2 mmWave;

void setup() {
  Serial.begin(115200);
  mmWave.begin(&mmWaveSerial);
}

void loop() {
  if (mmWave.update(100)) {
    float rate;
    if (mmWave.getBreathRate(rate)) Serial.printf("breath: %.2f\n", rate);
    if (mmWave.getHeartRate(rate))  Serial.printf("heart:  %.2f\n", rate);
  }
}
```

## API (1.x)

Both device classes inherit from `SeeedmmWave` and share the transport API:

```cpp
void begin(HardwareSerial*, uint32_t baud = 115200, uint32_t wait_delay = 1, int rst = -1);
bool update(uint32_t timeout = 100);       // pump UART and dispatch frames
bool send(uint16_t type, const uint8_t* data = nullptr, size_t len = 0);
```

### `SEEED_MR60BHA2`

```cpp
bool getHeartBreathPhases(float& total_phase, float& breath_phase, float& heart_phase);
bool getBreathRate(float& rate);
bool getHeartRate(float& rate);
bool getDistance(float& distance);
bool isHumanDetected();
bool getPeopleCountingPointCloud(PeopleCounting& out);
```

### `SEEED_MR60FDA2`

```cpp
bool getHuman(bool& is_human);
bool getFall(bool& is_fall);

bool setInstallationHeight(float height_m);
bool setThreshold(float threshold);
bool setSensitivity(uint32_t sensitivity);
bool setUserLog(uint8_t level);

bool getRadarParameters(float& h, float& th, uint32_t& sens,
                        float& rect_XL, float& rect_XR,
                        float& rect_ZF, float& rect_ZB);
```

All getters return `true` only when the requested frame has arrived since the
last read. Call `update()` from your `loop()` first. Setters block briefly
until the module acknowledges or the request times out.

### Optional helpers: `extras/`

```cpp
#include "extras/OnboardIndicator.h"      // pulls in Adafruit_NeoPixel
#include "extras/OnboardAmbientLight.h"   // pulls in hp_BH1750

seeed::mmwave::OnboardIndicator    indicator;  // defaults to D1 WS2812
seeed::mmwave::OnboardAmbientLight light;      // BH1750 on default I2C

indicator.begin();       indicator.set(0, 255, 0);
float lux;
if (light.begin() && light.read(lux)) Serial.println(lux);
```

Including the core `Seeed_Arduino_mmWave.h` never pulls in NeoPixel or BH1750 —
those dependencies are activated only by the `extras/` headers.

## Frame protocol

Every packet on the UART link has this shape:

```
+------+------+------+------+-------------+----------+-------------+
| SOF  | ID   | LEN  | TYPE | HEAD_CKSUM  | DATA     | DATA_CKSUM  |
| 1 B  | 2 B  | 2 B  | 2 B  | 1 B         | LEN B    | 1 B         |
+------+------+------+------+-------------+----------+-------------+
```

* `SOF` is always `0x01`.
* `LEN` is the length of the `DATA` field in bytes (big-endian).
* `HEAD_CKSUM` is the XOR of the seven header bytes before it.
* `DATA_CKSUM` is the XOR of all `DATA` bytes.
* `TYPE` identifies what the frame contains; each device defines its own type
  codes (see the corresponding `SEEED_MR60*.h` headers).

All data payloads are little-endian.

## XIAO ESP32-C6 × mmWave module pin map

| XIAO pin | Connects to                  |
| -------- | ---------------------------- |
| U0RXD    | mmWave module UART TX        |
| U0TXD    | mmWave module UART RX        |
| D1       | Onboard WS2812 data line     |
| SDA/SCL  | Onboard BH1750 + Grove I²C   |
| D0       | Grove port SIG1 (first GPIO) |
| D10      | Grove port SIG2 (second GPIO)|

The Grove port is just "two GPIO lines + V + G". When you plug a Grove I²C
module into it you talk I²C over the same SDA/SCL bus; when you plug a Grove
relay or button module you use D0 as a digital pin. No extra library-level
abstraction is needed.

## Examples

| Example                         | Device          | Depends on `extras/` | Description                                                    |
| ------------------------------- | --------------- | -------------------- | -------------------------------------------------------------- |
| `mmwave_breath_heartrate`       | MR60BHA2        | –                    | Minimal breathing / heartbeat / distance polling               |
| `mmwave_fall_detection`         | MR60FDA2        | –                    | Minimal human presence / fall detection polling                |
| `mmwave_people_counting`        | MR60BHA2        | –                    | Iterate the 3-D point cloud and per-target info                |
| `mmwave_diagnostics`            | –               | –                    | Raw byte passthrough for sniffing the protocol                 |
| `onboard_ambient_light`         | –               | BH1750               | Poll the onboard BH1750 and print lux values                   |
| `onboard_indicator_led`         | –               | NeoPixel             | Drive the onboard WS2812 indicator                             |
| `grove_oled_display`            | –               | –                    | Grove SSD1306 OLED "Hello World" over the Grove I²C port       |
| `integration_breath_monitor`    | MR60BHA2        | NeoPixel             | Breathing demo with OLED UI + onboard indicator                |
| `integration_fall_alert`        | MR60FDA2        | NeoPixel, BH1750     | Fall + light-aware relay trigger with onboard indicator        |

## Troubleshooting

* **No data, `update()` always returns `false`.** Check the UART wiring and
  that you pass the correct `HardwareSerial*` to `begin()`. The module
  transmits on a fixed 115200 baud; do not change it.
* **`set*()` returns `false`.** The module acknowledges configuration writes;
  a `false` return means either a transport issue or that the module did not
  respond within the default timeout. Retry, and verify serial wiring first.
* **Readings drift or freeze for a few seconds.** The frame queue defaults to
  a multi-second backlog. Call `update()` at 10–20 Hz in `loop()`; if your
  application uses a slow main loop, raise `MMWAVE_QUEUE_CAPACITY` via a
  compile-time `-D` flag.
* **FreeRTOS / multi-task usage.** The library is intended for single-task
  polling. Call `update()` from exactly one task and keep it out of ISRs.

## Event callbacks, Status, stats

```cpp
SEEED_MR60BHA2 mmWave;

mmWave.onBreathRate([](float r)            { Serial.printf("breath %.2f\n", r); });
mmWave.onHeartRate([](float r)             { Serial.printf("heart  %.2f\n", r); });
mmWave.onPresence([](bool present)         { Serial.printf("in room: %d\n", present); });
mmWave.onError([](seeed::mmwave::Status s) { Serial.println(seeed::mmwave::statusName(s)); });

void loop() {
  mmWave.update(100);
  const auto& s = mmWave.stats();
  // s.framesRx, s.checksumErrors, s.droppedByQueueFull, s.bytesRx
}
```

Every polling getter also has a `Status read*(&out)` sibling that distinguishes
`Ok` from `NoData`. See `examples/mmwave_event_callbacks` for a full sketch.

> Coming from **v1.x**? v2 is a major release and is not guaranteed
> source-compatible. If you have an existing sketch that you do not plan to
> update, pin your dependency to the latest **v1.x** release.

## License

Released under the [MIT License](LICENSE).

## Contributing

Pull requests welcome. Please run `clang-format` on any C/C++ changes and keep
example code aligned with the existing naming convention
(`mmwave_*`, `onboard_*`, `grove_*`, `integration_*`).
