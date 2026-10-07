/**
 * Adaptive Relay N3 — ranked whisper census (firmware port v1, MVP).
 * See AdaptiveRelayN3.h for the design and the OFF contract.
 */

#if ADAPTIVE_RELAY_N3

#include "AdaptiveRelayN3.h"
#include "main.h" // LOG_*
#include "mesh-pb-constants.h"

namespace AdaptiveRelayN3 {

struct CensusEntry {
    NodeNum from;      // originator of the packet we are relaying
    PacketId id;       // its packet id
    uint8_t count;     // copies heard (incl. the triggering one)
    uint8_t rank;      // evidence rank of our pending rebroadcast
    uint32_t opened;   // millis when the entry was opened
    bool used;
};

static CensusEntry s_census[AR_N3_CENSUS_MAX];

static uint32_t nowMs() { return millis(); }

static uint8_t rankOfSnr(float snr)
{
    if (snr >= AR_N3_SNR_STRONG_DB)
        return RANK_STRONG;
    if (snr >= AR_N3_SNR_WEAK_DB)
        return RANK_MEDIUM;
    return RANK_WEAK;
}

static uint8_t kFor(uint8_t rank)
{
    switch (rank) {
    case RANK_STRONG:
        return 99; // never yield to the census
    case RANK_MEDIUM:
        return AR_N3_K_MEDIUM;
    default:
        return AR_N3_K_WEAK;
    }
}

static CensusEntry *findEntry(NodeNum from, PacketId id, bool pruneStale)
{
    const uint32_t now = nowMs();
    CensusEntry *oldestUnused = nullptr;
    for (auto &e : s_census) {
        if (!e.used)
            continue;
        if (pruneStale && (now - e.opened) > AR_N3_ENTRY_TTL_MS) {
            e.used = false; // prune on access; TX delays are seconds-scale
            continue;
        }
        if (e.from == from && e.id == id)
            return &e;
    }
    return nullptr;
}

void onRebroadcastQueued(const meshtastic_MeshPacket *p)
{
    const NodeNum from = getFrom(p);
    const PacketId id = p->id;
    const float snr = p->rx_snr;

    CensusEntry *e = findEntry(from, id, true);
    if (e) {
        // already pending for this packet: refresh rank from the latest copy
        e->count = 1;
        e->rank = rankOfSnr(snr);
        e->opened = nowMs();
        return;
    }
    // open a new entry; evict the oldest when the static pool is full
    CensusEntry *slot = nullptr;
    uint32_t oldest = UINT32_MAX;
    for (auto &c : s_census) {
        if (!c.used) {
            slot = &c;
            break;
        }
        if (c.opened < oldest) {
            oldest = c.opened;
            slot = &c;
        }
    }
    if (!slot)
        return; // unreachable: pool is non-empty
    slot->from = from;
    slot->id = id;
    slot->count = 1; // the triggering copy
    slot->rank = rankOfSnr(snr);
    slot->opened = nowMs();
    slot->used = true;

    LOG_DEBUG("AR-N3: census open 0x%08x/0x%x rank=%u (snr=%.1f)", from, id, slot->rank, (double)snr);
}

bool shouldYieldOnDupe(const meshtastic_MeshPacket *p)
{
    CensusEntry *e = findEntry(getFrom(p), p->id, true);
    if (!e)
        return true; // no census entry (e.g. our own retransmissions, already-relayed
                     // packets): keep the exact stock K=1 behavior
    if (e->count < 255)
        e->count++;
    const uint8_t k = kFor(e->rank);
    if (e->count >= k) {
        LOG_DEBUG("AR-N3: yield 0x%08x/0x%x after %u copies (rank=%u)", e->from, e->id, e->count, e->rank);
        e->used = false; // decided — free the slot
        return true;
    }
    LOG_DEBUG("AR-N3: hold 0x%08x/0x%x copies=%u < K=%u (rank=%u)", e->from, e->id, e->count, k, e->rank);
    return false;
}

uint32_t censusSize()
{
    uint32_t n = 0;
    for (auto &e : s_census)
        if (e.used)
            n++;
    return n;
}

} // namespace AdaptiveRelayN3

#endif // ADAPTIVE_RELAY_N3
