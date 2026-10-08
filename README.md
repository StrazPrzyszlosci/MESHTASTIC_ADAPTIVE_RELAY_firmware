# MESHTASTIC_ADAPTIVE_RELAY_firmware

**Experimental Meshtastic firmware with the Adaptive Relay routing layer.**

This is NOT official Meshtastic firmware. It is an experimental research
build based on the official
[meshtastic/firmware](https://github.com/meshtastic/firmware) **v2.8.2**
(commit `9e4d301`), extended with the **Adaptive Relay N3** routing
mechanism - a feature that **does not exist in the official firmware**.
It comes from the research project
[StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY),
where it was designed, tuned and validated in the official Meshtasticator
discrete-event simulator.

> ⚠️ **Use at your own risk.** The routing layer is validated in
> simulation only (including a firmware-realistic observability model),
> and has not yet been field-tested on hardware meshes. Before flashing,
> back up your device (see below). You can always return to official
> firmware.

## What is different from official firmware

Official Meshtastic relays broadcasts with pure managed flooding: every
node that hears a packet re-broadcasts it unless it hears someone else do
it first (the first overheard copy cancels yours, K=1). Since 2.6,
official firmware also has directed next-hop routing - but **for
direct messages only**. Broadcasts get no relay coordination.

This firmware adds **Adaptive Relay N3 (ranked whisper census)** for the
broadcast path, purely as a local heuristic (no protocol changes, no new
packets, no on-wire fields):

- the stock SNR-weighted TX delay already makes strong links answer
  first - the "whisper" ordering is emergent;
- **N3 census (yield path)**: a node that clearly decoded a packet
  (**rx_snr ≥ 6 dB**) never yields its rebroadcast to overheard copies; a
  medium-read node (**−4..6 dB**) yields only after **3 corroborating
  copies**; a weak-read node keeps the stock quick yield. Official
  firmware cancels on the first copy for all of them;
- **SHEP2D rescue (watchdog regret, fully local)**: a node watches the
  packets it suppressed; if a suppressed packet is never heard again,
  that packet died at this hop. Sustained regret (**3 deaths** in a
  30 s window) opens a bounded **rescue gate**: for a 45 s lease the node
  forwards what it would have suppressed, strictly within a budget of
  4 forwards - then the gate closes until regret builds again;
- **CEF_BOOST congestion mode**: 30 s evidence windows over local
  channel utilization (firmware ChannelUtilization) and overheard-copy
  redundancy; after 3 consecutive busy+redundant windows (util ≥ 35%,
  ≥ 2.5 copies/dupe) every rank yields at K=2 - protecting a saturated
  channel. The FSM exits instantly on any rescue signal, and on real
  quiet networks (0.15-3% utilization - see the research repo's
  REAL_WORLD_CALIBRATION.md) it never triggers by design.

Everything else - roles, ACK handling, hop limits, duty cycle, NextHop
routing for DMs - is untouched upstream code. The whole mechanism is
**OFF by default at compile time** (`ADAPTIVE_RELAY_N3`): without the
build flag the decision path compiles byte-identically to stock
(verified by post-link disassembly: both integration points produce
identical machine code and size-identical images).

Release binaries in this repository are built **with the feature ON**.

## Supported devices

| Device                               | Platform | Radio  |
| ------------------------------------ | -------- | ------ |
| Heltec V3                            | ESP32-S3 | SX1262 |
| Heltec V4                            | ESP32-S3 | SX1262 |
| RAK4631 (WisBlock)                   | nRF52840 | SX1262 |
| Seeed XIAO nRF52840 Kit + Wio-SX1262 | nRF52840 | SX1262 |

These are relay-node targets (no audio devices are in scope of this port).
The Seeed kit is the same nRF52840 + SX1262 silicon as the RAK4631 on a
smaller board - it uses the stock upstream `seeed_xiao_nrf52840_kit`
variant (HW model 88).

## Downloads (direct binaries)

Prebuilt images with the feature ON, from the
[**v2.8.2-ar.2 release**](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/tag/v2.8.2-ar.2)
(Heltec V3/V4 and RAK4631 binaries are unchanged from `v2.8.2-ar.1`, built
from the three-layer source commit `e1521c776`; the Seeed XIAO kit image is
built from `1c9f3598d` - only docs commits on top; base upstream
`v2.8.2 @ 9e4d301`):

| Device                  | Image                                       | Direct link                                                                                                                                                                                         |
| ----------------------- | ------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Heltec V3               | OTA/app image (`.bin`)                      | [firmware-heltec-v3-2.8.2-ar.bin](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-heltec-v3-2.8.2-ar.bin)                             |
| Heltec V3               | factory image (`.factory.bin`, first flash) | [firmware-heltec-v3-2.8.2-ar.factory.bin](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-heltec-v3-2.8.2-ar.factory.bin)             |
| Heltec V4               | OTA/app image (`.bin`)                      | [firmware-heltec-v4-2.8.2-ar.bin](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-heltec-v4-2.8.2-ar.bin)                             |
| Heltec V4               | factory image (`.factory.bin`, first flash) | [firmware-heltec-v4-2.8.2-ar.factory.bin](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-heltec-v4-2.8.2-ar.factory.bin)             |
| RAK4631                 | UF2 (drag & drop)                           | [firmware-rak4631-2.8.2-ar.uf2](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-rak4631-2.8.2-ar.uf2)                                 |
| RAK4631                 | hex (SWD/programmer)                        | [firmware-rak4631-2.8.2-ar.hex](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-rak4631-2.8.2-ar.hex)                                 |
| RAK4631                 | DFU update package (`.zip`)                 | [firmware-rak4631-2.8.2-ar.zip](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-rak4631-2.8.2-ar.zip)                                 |
| Seeed XIAO nRF52840 kit | UF2 (drag & drop)                           | [firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.uf2](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.uf2) |
| Seeed XIAO nRF52840 kit | hex (SWD/programmer)                        | [firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.hex](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.hex) |
| Seeed XIAO nRF52840 kit | DFU update package (`.zip`)                 | [firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.zip](https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware/releases/download/v2.8.2-ar.2/firmware-seeed_xiao_nrf52840_kit-2.8.2-ar.zip) |

Which file to use:

- **Heltec V3/V4, first flash or recovery** → the `.factory.bin` (single
  image at offset `0x0`).
- **Heltec V3/V4, updating an existing Meshtastic device** → the `.bin`
  (OTA partition image) via esptool/PlatformIO, or simply flash the
  factory image again - both work.
- **RAK4631 / Seeed XIAO nRF52840 kit** → the `.uf2` (double-tap reset,
  drag & drop).
- **RAK4631 / Seeed XIAO kit, updating a running device** → the `.zip`
  (DFU update package applied **by the bootloader** - via the
  Meshtastic Web Flasher or a BLE firmware update).

**None of the nRF52840 images overwrite the bootloader or the SoftDevice.**
Every shipped image contains only the application partition (verified
from the `.hex` address map: `0x26000..0xCFB3C` on the RAK4631,
`0x27000..0xCE8F4` on the XIAO kit; the MBR/SoftDevice sit at the bottom
of flash, the bootloader at the top). The `.uf2` and the `.zip` are
applied by the bootloader itself (USB drive / DFU), so they physically
cannot overwrite it.

> ⚠️ **Never use "erase all" / mass-erase / chip-erase on these devices**
> (J-Link, `nrfjprog --eraseall`, pyocd, STM32CubeProgrammer "Erase
> chip", etc.). It wipes the **entire chip** - bootloader, SoftDevice and
> UICR settings included. The device then no longer boots and the UF2
> drive never appears; recovery requires re-flashing the bootloader and
> SoftDevice over SWD. Use only the safe methods:
>
> 1. **`.uf2`** - double-tap reset → drag & drop onto the UF2 drive
>    (bootloader-applied, cannot touch itself);
> 2. **`.zip`** - DFU update via the Meshtastic Web Flasher or a BLE
>    firmware update (also bootloader-applied);
> 3. **`.hex`** via SWD - app-only image, program **without any
>    erase-all**; erasing only the application sectors is fine.

## Before flashing: BACK UP

Flashing over an existing Meshtastic device normally keeps its
configuration, but always back up first:

```bash
# config + channels (device connected over USB / BLE / serial)
meshtastic --export-config > my-backup-2026-10-07.yaml

# your NodeDB (known nodes list)
meshtastic --nodes > my-nodes-2026-10-07.txt

# (optional) full config JSON, if your firmware supports it
meshtastic --get > my-settings-2026-10-07.json
```

Keep the files somewhere safe. If anything goes wrong, re-flash official
firmware and restore with `meshtastic --configure my-backup-2026-10-07.yaml`.

## Installation

### Heltec V3 / V4 (ESP32-S3)

1. Install the USB driver (CP210x / CH9102) and connect via USB.
2. Easiest - [Meshtastic Web Flasher](https://flasher.meshtastic.org)
   supports custom firmware: choose **Upload your own firmware release ZIP
   or bin** and select the matching `.factory.bin` for your board. This
   experimental build is not in the standard release list. Use Chrome or
   Edge.
   - **esptool** (factory image, first flash / recovery):
     ```bash
     pip install esptool
     esptool.py --chip esp32s3 --port /dev/ttyUSB0 \
         write_flash -z 0x0 firmware-<target>-2.8.2.<hash>.factory.bin
     ```
   - **PlatformIO upload** (from a source checkout):
     ```bash
     pio run -e heltec-v3 -t upload     # or -e heltec-v4
     ```
3. The device reboots into Meshtastic. Re-pair the phone app / restore
   the backed-up config.

### RAK4631 (nRF52840)

1. Put the device into UF2 bootloader: **double-tap the RESET button**.
   A `RAK4631` USB drive appears.
2. Drag & drop `firmware-rak4631-2.8.2.<hash>.uf2` onto the drive.
3. The drive disconnects and the device reboots into Meshtastic.

### Seeed XIAO nRF52840 Kit (nRF52840 + Wio-SX1262)

Same UF2 procedure as the RAK4631:

1. Put the board into UF2 bootloader: **double-tap the RESET button**
   (a `XIAO-SENSE` USB drive appears).
2. Drag & drop
   `firmware-seeed_xiao_nrf52840_kit-2.8.2.<hash>.uf2` onto the drive.
3. The drive disconnects and the board reboots into Meshtastic.

Note: there are two Wio-SX1262 SKUs - this target is for the **Meshtastic
kit** (XIAO nRF52840 + Wio-SX1262, Seeed SKU 102010710 / standalone
113010003). If your module is the 30-pin board-to-board version from the
ESP32-S3 kit, build the `seeed_xiao_nrf52840_btb` env instead.

### Rolling back to official firmware

Flash the official release for your device from
[meshtastic/firmware releases](https://github.com/meshtastic/firmware/releases)
(the same way as above), then restore your config backup. Your channels,
keys and NodeDB survive normal re-flashing; the config backup covers the
rest.

## Building from source

```bash
git clone https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY_firmware.git
cd MESHTASTIC_ADAPTIVE_RELAY_firmware

# PlatformIO 6.1.19 is required (6.2.x breaks on SCons tool packaging)
python3 -m venv pio-venv
pio-venv/bin/pip install "platformio==6.1.19"

# feature ON (as shipped in Releases):
PLATFORMIO_BUILD_FLAGS="-DADAPTIVE_RELAY_N3=1" pio-venv/bin/pio run -e heltec-v3

# feature OFF (byte-identical to upstream v2.8.2):
pio-venv/bin/pio run -e heltec-v3
```

Build the other targets with `-e heltec-v4` / `-e rak4631` /
`-e seeed_xiao_nrf52840_kit`. The build path must not contain spaces. On
ESP32-S3 targets the nested Arduino build may need a SCons shim depending
on the PlatformIO version - if you hit
`No module named 'SCons.Tool.FortranCommon'`, run the failing build with
PlatformIO 6.1.19 (as pinned above), which avoids the broken scons-local
package.

The port code lives in `src/mesh/AdaptiveRelayN3.h/.cpp` with two
integration points: `NextHopRouter::perhapsRebroadcast` (census opens
when our rebroadcast is queued) and `FloodingRouter::perhapsCancelDupe`
(rank-dependent yield). The full design, the OFF-contract proof and the
simulation results are documented in the research repository.

## How it was validated (before this port)

The mechanism was tuned and falsification-tested in the official
[meshtastic/Meshtasticator](https://github.com/meshtastic/Meshtasticator)
simulator, on a clean base, across ~10,000 paired runs: 17 topologies,
traffic loads up to 6x, three PHY presets, node counts up to 500, a
frozen holdout, and a firmware-realistic observability model (a real node
cannot tell WHO re-broadcast a copy - the port counts copies, not
relayers, exactly matching the validated semantics). Headline simulator
results: on bridged/hub/chain topologies the adaptive layer delivers
9-19 pp more reach at up to 10% fewer transmissions; under the degraded
observability model the advantage fully survives (bridge +8.75 pp,
hub +7.41 pp, all seeds). All numbers are simulation results - a
high-confidence proof of concept, not a hardware validation. See the
research repo for full tables, raw panels and audit documents.

## Scope and status

Ported and built (this repository, release `v2.8.2-ar.2`): **all three
validated layers** - N3 ranked whisper census, SHEP2D local rescue
(regret → bounded rescue gate) and CEF_BOOST congestion FSM - on four
relay targets (Heltec V3/V4, RAK4631, Seeed XIAO nRF52840 kit). Flash
cost: ~2.0-4.1 KB (see the release size table); static RAM ~1.5 KB.

Nothing else touches the stock path. Out of scope per the research
doctrine (needs a protocol change, deferred): the simulator's COLLECT
broadcast lease coordinating NEIGHBOR gates - the port implements the
local self-rescue semantics, same trigger, same lease/budget, no new
packets on air.

## License

This repository is a derivative of the official Meshtastic firmware
(v2.8.2, `9e4d301`) and inherits its
[GPL-3.0 license](LICENSE) and all upstream attribution. The Adaptive
Relay additions are released under the same GPL-3.0 terms. Meshtastic is
a project of the Meshtastic Software Foundation - this experimental fork
is not affiliated with or endorsed by them.
