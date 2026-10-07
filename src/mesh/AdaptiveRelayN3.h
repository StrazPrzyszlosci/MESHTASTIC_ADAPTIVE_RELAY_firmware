#pragma once

/**
 * Adaptive Relay — firmware port of the research routing layer
 * (https://github.com/StrazPrzyszlosci/MESHTASTIC_ADAPTIVE_RELAY): the
 * full three-mechanism architecture validated in the official
 * Meshtasticator simulator, implemented as purely local heuristics.
 *
 * Three layers (build-time gated, all under ADAPTIVE_RELAY_N3):
 *
 * 1. N3 ranked whisper census — broadcast relay coordination on the
 *    dupe-yield path. The stock SNR-weighted TX delay already orders
 *    responses (strong links first); this adds evidence-ranked census
 *    thresholds: clear reads never yield, medium reads yield after
 *    K corroborating copies, weak reads keep the stock quick yield.
 *
 * 2. SHEP2D bounded rescue (local port) — the node watches packets it
 *    suppressed: if a suppressed packet is never heard again (it died at
 *    this hop), that is regret. Sustained regret opens a bounded rescue
 *    gate: for a short lease the node stops yielding (forwards what it
 *    would have suppressed), within a strict rescue budget. Research
 *    note: the simulator's COLLECT broadcast coordinated NEIGHBOR gates;
 *    a broadcast lease needs a protocol (protobuf) change, so this port
 *    implements the local, self-rescue semantics — same trigger, same
 *    lease/budget bounds, no new packets on air.
 *
 * 3. CEF_BOOST congestion mode — a small FSM over local evidence
 *    (channel utilization + overheard-copy redundancy). After sustained
 *    busy+redundant windows it switches to aggressive yielding (even
 *    strong reads yield quickly) to protect a saturated channel, and
 *    exits instantly on any rescue (regret) signal or when congestion
 *    clears. Calibrated for genuinely saturated meshes: real-world
 *    quiet networks sit at 0.15-3% utilization and never trigger it.
 *
 * Zero-oracle guarantee (research doctrine): every input is locally
 * observable — rx_snr of the relayed copy, (from,id), channel
 * utilization, own timers. Copies are counted, not relayers (a real node
 * cannot attribute a relayed copy) — the REALISTIC_WIRE semantics under
 * which the mechanism was validated in simulation.
 *
 * Static budget (FIRMWARE_RESOURCE_MODEL.md): census 64 x ~16 B, park
 * 32 x ~12 B, FSM scalars — under 1.5 KB RAM total, no allocation.
 *
 * OFF contract: the whole module is empty unless ADAPTIVE_RELAY_N3 is
 * defined; both call sites degrade to the exact stock decision path
 * (verified byte-identical by post-link disassembly).
 *
 * Enable: PLATFORMIO_BUILD_FLAGS="-DADAPTIVE_RELAY_N3=1" pio run -e <target>
 */

#if ADAPTIVE_RELAY_N3

#include "MeshTypes.h"
#include <stdint.h>

struct _meshtastic_MeshPacket;
typedef struct _meshtastic_MeshPacket meshtastic_MeshPacket;

namespace AdaptiveRelayN3 {

/** ---- layer 1: N3 census (evidence rank from the SNR of the copy
 *  we are relaying; port-v1 SNR-domain calibration) ---- */
enum Rank {
    RANK_STRONG = 0,   // clear read: never yield to the census (K=99)
    RANK_MEDIUM = 1,   // corroborated yielding: K = AR_N3_K_MEDIUM
    RANK_WEAK = 2      // near-silence only: K = AR_N3_K_WEAK
};

#ifndef AR_N3_SNR_STRONG_DB
#define AR_N3_SNR_STRONG_DB 6.0f
#endif
#ifndef AR_N3_SNR_WEAK_DB
#define AR_N3_SNR_WEAK_DB -4.0f
#endif
#ifndef AR_N3_K_MEDIUM
#define AR_N3_K_MEDIUM 3 // copies (incl. the one that triggered us) before yielding
#endif
#ifndef AR_N3_K_WEAK
#define AR_N3_K_WEAK 2
#endif
#ifndef AR_N3_CENSUS_MAX
#define AR_N3_CENSUS_MAX 64 // static bound (FIRMWARE_RESOURCE_MODEL.md)
#endif
#ifndef AR_N3_ENTRY_TTL_MS
#define AR_N3_ENTRY_TTL_MS 60000u
#endif

/** ---- layer 2: SHEP2D local rescue ---- */
#ifndef AR_SHEP_PARK_MAX
#define AR_SHEP_PARK_MAX 32
#endif
#ifndef AR_SHEP_PARK_TTL_MS
#define AR_SHEP_PARK_TTL_MS 45000u // no further copy within this -> death
#endif
#ifndef AR_SHEP_WINDOW_MS
#define AR_SHEP_WINDOW_MS 30000u // deaths within this window count together
#endif
#ifndef AR_SHEP_DEATHS
#define AR_SHEP_DEATHS 3 // sustained regret threshold -> open gate
#endif
#ifndef AR_SHEP_LEASE_MS
#define AR_SHEP_LEASE_MS 45000u
#endif
#ifndef AR_SHEP_BUDGET
#define AR_SHEP_BUDGET 4 // kept-forward decisions per lease
#endif

/** ---- layer 3: CEF_BOOST congestion FSM ---- */
#ifndef AR_CB_WINDOW_MS
#define AR_CB_WINDOW_MS 30000u
#endif
#ifndef AR_CB_UTIL_MIN
#define AR_CB_UTIL_MIN 35.0f // enter: channel utilization % (real busy meshes: ~50%, quiet: 0.15-3%)
#endif
#ifndef AR_CB_EXIT_UTIL
#define AR_CB_EXIT_UTIL 25.0f
#endif
#ifndef AR_CB_COPIES_MIN
#define AR_CB_COPIES_MIN 2.5f // enter: mean overheard copies per judged dupe
#endif
#ifndef AR_CB_STABLE
#define AR_CB_STABLE 3 // consecutive busy+redundant windows to enter
#endif
#ifndef AR_CB_K
#define AR_CB_K 2 // yield threshold for ALL ranks while boosting
#endif

/** Called when our rebroadcast of `p` has been queued (TX pending).
 *  Opens/refreshes the census entry: count = 1 (the triggering copy),
 *  rank from p->rx_snr. Bounded static pool, oldest-eviction. */
void onRebroadcastQueued(const meshtastic_MeshPacket *p);

/** Called from the stock dupe path AFTER the stock role gate passed.
 *  Counts the overheard copy, runs regret bookkeeping (layer 2) and
 *  the congestion window (layer 3), and decides:
 *    true  -> yield (cancel our pending rebroadcast) — stock K=1 for
 *             packets without a census entry, or the effective
 *             threshold reached (census / boost / gate)
 *    false -> keep our pending rebroadcast
 *  channelUtilPercent: local channel utilization (airTime->...), the
 *  layer-3 congestion input. */
bool shouldYieldOnDupe(const meshtastic_MeshPacket *p, float channelUtilPercent);

/** Diagnostics: census entries currently held (0..AR_N3_CENSUS_MAX). */
uint32_t censusSize();

/** Diagnostics: layer-2/3 state for logs/metrics. */
uint8_t rescueGateOpen(); // 0/1
uint8_t boostActive();    // 0/1

} // namespace AdaptiveRelayN3

#endif // ADAPTIVE_RELAY_N3
