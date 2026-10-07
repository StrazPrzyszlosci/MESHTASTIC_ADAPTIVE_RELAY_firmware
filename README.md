# MESHTASTIC_ADAPTIVE_RELAY_firmware

**Experimental Meshtastic firmware with the Adaptive Relay routing layer.**

This is NOT official Meshtastic firmware. It is an experimental research
build based on the official
[meshtastic/firmware](https://github.com/meshtastic/firmware) **v2.8.2**
(commit `9e4d301`), extended with the **Adaptive Relay N3** routing
mechanism — a feature that **does not exist in the official firmware**.
It comes from the research project
[StrazPrzyszlosi/MESHTASTIC_ADAPTIVE_RELAY](https://github.com/StrazPrzyszlosi/MESHTASTIC_ADAPTIVE_RELAY),
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
official firmware also has directed next-hop routing — but **for
direct messages only**. Broadcasts get no relay coordination.

This firmware adds **Adaptive Relay N3 (ranked whisper census)** for the
broadcast path, purely as a local heuristic (no protocol changes, no new
packets, no on-wire fields):

- the stock SNR-weighted TX delay already makes strong links answer
  first — the "whisper" ordering is emergent;
- this port adds **evidence-ranked census thresholds** on the yield path:
  a node that clearly decoded a packet (**rx_snr ≥ 6 dB**) never yields
  its rebroadcast to overheard copies; a medium-read node (**−4..6 dB**)
  yields only after **3 corroborating copies**; a weak-read node keeps
  the stock quick yield. Official firmware cancels on the first copy for
  all of them.

Everything else — roles, ACK handling, hop limits, duty cycle, NextHop
routing for DMs — is untouched upstream code. The whole mechanism is
**OFF by default at compile time** (`ADAPTIVE_RELAY_N3`): without the
build flag the decision path compiles byte-identically to stock
(verified by post-link disassembly: both integration points produce
identical machine code and size-identical images).

Release binaries in this repository are built **with the feature ON**.

## Supported devices

| Device | Platform | Radio |
|---|---|---|
| Heltec V3 | ESP32-S3 | SX1262 |
| Heltec V4 | ESP32-S3 | SX1262 |
| RAK4631 (WisBlock) | nRF52840 | SX1262 |

These are relay-node targets (no audio devices are in scope of this port).

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
2. Easiest — [Meshtastic web flasher](https://flasher.meshtastic.org)
   does not host this experimental build; use one of:
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

### Rolling back to official firmware

Flash the official release for your device from
[meshtastic/firmware releases](https://github.com/meshtastic/firmware/releases)
(the same way as above), then restore your config backup. Your channels,
keys and NodeDB survive normal re-flashing; the config backup covers the
rest.

## Building from source

```bash
git clone https://github.com/StrazPrzyszlosi/MESHTASTIC_ADAPTIVE_RELAY_firmware.git
cd MESHTASTIC_ADAPTIVE_RELAY_firmware

# PlatformIO 6.1.19 is required (6.2.x breaks on SCons tool packaging)
python3 -m venv pio-venv
pio-venv/bin/pip install "platformio==6.1.19"

# feature ON (as shipped in Releases):
PLATFORMIO_BUILD_FLAGS="-DADAPTIVE_RELAY_N3=1" pio-venv/bin/pio run -e heltec-v3

# feature OFF (byte-identical to upstream v2.8.2):
pio-venv/bin/pio run -e heltec-v3
```

Build the other targets with `-e heltec-v4` / `-e rak4631`. The build
path must not contain spaces. On ESP32-S3 targets the nested Arduino
build may need a SCons shim depending on the PlatformIO version — if you
hit `No module named 'SCons.Tool.FortranCommon'`, run the failing build
with PlatformIO 6.1.19 (as pinned above), which avoids the broken
scons-local package.

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
cannot tell WHO re-broadcast a copy — the port counts copies, not
relayers, exactly matching the validated semantics). Headline simulator
results: on bridged/hub/chain topologies the adaptive layer delivers
9-19 pp more reach at up to 10% fewer transmissions; under the degraded
observability model the advantage fully survives (bridge +8.75 pp,
hub +7.41 pp, all seeds). All numbers are simulation results — a
high-confidence proof of concept, not a hardware validation. See the
research repo for full tables, raw panels and audit documents.

## Scope and status

Ported and built (this repository): **N3 ranked whisper census** (MVP,
~650 bytes of flash on ESP32-S3).

Planned next stages (researched and validated in simulation, not yet
ported): SHEP2D bounded rescue (watchdog/regret layer), CEF_BOOST
congestion mode.

## License

This repository is a derivative of the official Meshtastic firmware
(v2.8.2, `9e4d301`) and inherits its
[GPL-3.0 license](LICENSE) and all upstream attribution. The Adaptive
Relay additions are released under the same GPL-3.0 terms. Meshtastic is
a project of the Meshtastic Software Foundation — this experimental fork
is not affiliated with or endorsed by them.
