# Roadmap / Where We Left Off

This file exists so the next person (or the same person, three months from now)
opening this repo can immediately see what's done, what's in flight, and what
was intentionally skipped. Whenever something moves, **update this file**.

Last touched: **2026-04-19**.

---

## Release status

| Branch             | Published as                                 | State                         |
| ------------------ | -------------------------------------------- | ----------------------------- |
| `main`             | 1.0.0 (Arduino Library Manager)              | stable, unchanged             |
| `release/1.1.0-rc` | [v1.1.0-rc1](https://github.com/Love4yzp/Seeed-mmWave-library/releases/tag/v1.1.0-rc1) (GitHub prerelease) | **awaiting hardware validation** |
| `v2-dev`           | [v2.0.0-rc1](https://github.com/Love4yzp/Seeed-mmWave-library/releases/tag/v2.0.0-rc1) (GitHub prerelease) | **awaiting hardware validation** |

Both prereleases are marked `prerelease: true`, so Arduino Library Manager
will **not** index them and existing 1.0.0 users are unaffected.

## The one thing blocking both releases

**Hardware validation** on the default factory combo: XIAO ESP32-C6 + Seeed
mmWave MR60BHA2 / MR60FDA2. Everything in the two prereleases compiles, passes
`arduino-lint --compliance specification`, and passes the host-side FrameCodec
unit tests (`pio test -e native`). Nothing has been run on real hardware yet.

Minimum acceptance criteria (see also the GitHub issue):

- [ ] `examples/mmwave_breath_heartrate` streams stable values on MR60BHA2
      for ≥ 5 minutes with `stats().checksumErrors == 0`.
- [ ] `examples/mmwave_fall_detection` correctly flips `onFall(true)` on a
      sit-to-lie-down test.
- [ ] `examples/integration_breath_monitor` OLED readout matches serial.
- [ ] `examples/integration_fall_alert` lights the WS2812 + toggles the Grove
      relay based on fall + lux correctly.
- [ ] `examples/mmwave_event_callbacks` prints one event per frame type;
      no assertion failures.
- [ ] `examples/mmwave_configure_fall` (if added) round-trips height /
      threshold / sensitivity writes and reads them back through
      `readRadarParameters`.
- [ ] `setInstallationHeight` / `setThreshold` / `setSensitivity` still work
      after being exercised through the fall-alert example (this is the
      pending-response concern — see "Deferred to v2.1" below).

### Promotion procedure once validated

```bash
# 1.1.0 → stable
git checkout main
git merge --ff-only release/1.1.0-rc
git tag -a v1.1.0 -m "1.1.0"
git push origin main v1.1.0
gh release create v1.1.0 --target main --title "1.1.0" --latest --notes "..."

# 2.0.0 → stable (after 1.1.0)
git checkout main
git merge --ff-only v2-dev
git tag -a v2.0.0 -m "2.0.0"
git push origin main v2.0.0
gh release create v2.0.0 --target main --title "2.0.0" --latest --notes "..."
```

---

## Deferred to v2.1 (or later)

These were considered during v2 planning and intentionally **skipped**
because they need either real hardware measurements or a clearer demand
signal. Do not pick them up without that data.

1. **Pending-response channel for `setXxx` commands.** The current design
   serialises command ACKs through the main frame queue (see
   `SeeedmmWave::fetchType` at `src/SeeedmmWave.cpp`). Risk: under heavy
   data-frame load, `setInstallationHeight` / `setThreshold` /
   `setSensitivity` can wait behind backlog. Fix sketched in the v2 plan:
   give each in-flight command a dedicated slot that `fetch()` fills
   directly, bypassing `byteQueue`. **Needs:** measurement of ACK latency
   jitter on real hardware first; otherwise we're optimising blind.

2. **FreeRTOS multi-task safety.** State flags
   (`_is_human_detected`, `_fall_state`, `_breath_rate`, …) and the main
   queue are accessed without locks. If a user polls a getter from one
   task while `update()` runs on another, they'll race. Fix is
   straightforward (`std::atomic<T>` on primitive fields + `portMUX_TYPE`
   around the queue). **Needs:** a user actually doing multi-task polling
   — no reports yet, and the extra code size is non-zero.

3. **Lower `MMWAVE_QUEUE_CAPACITY` default from 120 to ~32.** The 120
   upper bound is generous (roughly 5–6 s of backlog at 20 Hz). **Needs:**
   real-world frame-rate measurement + an idea of the slowest loop rate
   we want to support. Currently `#define`-overridable, so users who need
   more can set it themselves.

4. **Replace `std::queue<std::vector<uint8_t>>` with a ring buffer of
   fixed-size frame slots.** Zero dynamic allocation, deterministic
   memory footprint. Pairs with #3. Scope-creep unless #3 is merged first.

5. **`PeopleCounting::targets` — fixed `std::array<TargetN, 16>`.** Stops
   per-frame `std::vector` allocations. Trivial change; blocked only by
   picking the right upper bound with hardware (sensor documents ≤ 3 for
   MR60BHA2 breathing, but spec may change).

6. **Tiered debug logging macros** (`MMWAVE_LOG_ERROR` / `_WARN` / `_INFO`
   / `_TRACE`). The current `_MMWAVE_DEBUG 0/1` is binary and dumps raw
   frames to `Serial`. Low priority — `stats()` + `onError()` already
   cover most diagnostics in the field.

7. **Fix `packetFrame()` zero-payload quirk.** When `data == nullptr`
   the function omits the `DATA_CKSUM` byte (`src/SeeedmmWave.cpp`),
   which disagrees with the frame spec. No caller actually exercises
   this path today (confirmed by grep), but if the module turns out to
   require the trailing checksum, FrameCodec already does the right
   thing and `packetFrame` just needs to call it.

8. **Deprecate the `bool get*()` getter family.** Once the `Status
   read*()` API is validated on hardware, we can start marking the old
   bool forms `[[deprecated]]` (in v2.1) and remove them in v3. No hurry.

## Intentionally **not** on the roadmap

Rejected during v2 planning; listed here so the reasoning isn't lost:

- **Rename `update()` → `poll()`** — the user community's cost to migrate
  every example and every downstream sketch outweighs the marginal
  clarity gain. `update()` is a widely understood Arduino idiom.
- **Rename `send()` → `sendCommand()`** — same reasoning.
- **Rename `get*()` → `read*()` with `Status`** — handled additively:
  both coexist, old callers keep working.
- **Stream& transport decoupling** — would let the library run on
  non-ESP32 MCUs. The Seeed mmWave kit is XIAO ESP32 only, so this is
  a non-goal.
- **Device factory / `mmWave.detect(Stream&)` / CustomDevice extension
  point** — only two devices (BHA2, FDA2) exist and no plan for a third
  short term. Speculative abstraction.
- **Root-level `CHANGELOG.md`** — GitHub Release notes fill this role
  and auto-link to diffs. One source of truth.

---

## How to resume

```bash
git fetch --all --tags
git log --oneline main..v2-dev          # see what's unmerged
gh release list --limit 5                # see live prereleases
pio test -e native                       # run host-side unit tests
/tmp/arduino-lint-bin/arduino-lint \
  --compliance specification \
  --library-manager update .             # re-run the strict lint
```

The plan that produced the current state lives at
`/Users/spencer/.claude/plans/crispy-spinning-micali.md` on the machine that
created it (not in this repo). That file is the longer-form "why we chose
this path" companion to the short roadmap above.
