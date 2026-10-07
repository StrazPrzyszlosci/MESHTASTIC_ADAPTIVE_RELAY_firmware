#pragma once

/**
 * Adaptive Relay N3 — ranked whisper census (firmware port v1, MVP).
 *
 * Ported from the MESHTASTIC_ADAPTIVE_RELAY research project
 * (https://github.com/StrazPrzyszlosi/MESHTASTIC_ADAPTIVE_RELAY): the
 * N3 "ranked whisper" mechanism, validated in the discrete-event
 * simulator and confirmed to survive firmware-realistic observability
 * (REALISTIC_WIRE panel): the relay-side advantage does not depend on
 * relay identity, only on own-RX evidence.
 *
 * Design (mapping sim -> firmware):
 *   sim: relay response windows ordered by evidence rank; census counts
 *        overheard copies; rank0 (strong margin) forwards, rank1 waits and
 *        yields after K=3 corroborating copies, rank2 fires only in
 *        near-silence (K=2).
 *   fw:  the stock SNR-weighted TX delay already orders the response
 *        (strong links answer first — the whisper structure is emergent);
 *        this module adds the MISSING piece: evidence-rank-dependent
 *        yielding thresholds on the stock dupe-cancel path.
 *   stock behavior: a CLIENT-role node cancels its pending rebroadcast on
 *        the FIRST overheard copy (K=1). With this module: rank0 (clear
 *        read) never yields to the census, rank1 yields only after
 *        K=3 copies, rank2 keeps the stock-quick yield (K=2).
 *
 * Zero-oracle guarantee (research doctrine): every input is locally
 * observable — rx_snr of the copy we are relaying, (from, id) of the
 * packet, our own role (the stock role gates stay authoritative).
 * Firmware counts COPIES, not distinct relayers (a real node cannot
 * attribute a relayed copy's transmitter) — exactly the REALISTIC_WIRE
 * semantics validated in simulation.
 *
 * Compile-time OFF contract: the whole module is empty unless
 * ADAPTIVE_RELAY_N3 is defined; the two call sites degrade to the exact
 * stock decision path (byte-identical behavior, no size cost).
 *
 * Enable with:  PLATFORMIO_BUILD_FLAGS="-DADAPTIVE_RELAY_N3=1" pio run -e <target>
 */

#if ADAPTIVE_RELAY_N3

#include "MeshTypes.h"
#include <stdint.h>

// forward declaration (protobuf C bindings use a typedef; full type comes
// with the generated headers at the call sites)
struct _meshtastic_MeshPacket;
typedef struct _meshtastic_MeshPacket meshtastic_MeshPacket;

namespace AdaptiveRelayN3 {

/** evidence rank from the SNR of the copy we are relaying.
 *  Port-v1 calibration (SNR domain; sim bands were RSSI-margin based).
 *  Revisit after hardware logs from the FW+ pilot. */
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
#define AR_N3_ENTRY_TTL_MS 60000u // TX delays are seconds-scale; generous bound
#endif

/** Called when our rebroadcast of `p` has been queued (TX pending).
 *  Opens/refreshes the census entry: count = 1 (the triggering copy),
 *  rank from p->rx_snr. Bounded static pool, oldest-eviction. */
void onRebroadcastQueued(const meshtastic_MeshPacket *p);

/** Called from the stock dupe path AFTER the stock role gate passed.
 *  Counts the overheard copy and decides:
 *    true  -> yield (cancel our pending rebroadcast) — stock K=1 for
 *             packets without a census entry, or census threshold reached
 *    false -> keep our pending rebroadcast (threshold not reached yet)
 */
bool shouldYieldOnDupe(const meshtastic_MeshPacket *p);

/** Diagnostics: entries currently held (0..AR_N3_CENSUS_MAX). */
uint32_t censusSize();

} // namespace AdaptiveRelayN3

#endif // ADAPTIVE_RELAY_N3
