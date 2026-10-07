/**
 * Adaptive Relay — firmware port of the three-layer research router.
 * See AdaptiveRelayN3.h for the design, budgets and the OFF contract.
 *
 * Layers (all under #if ADAPTIVE_RELAY_N3, all static, no allocation):
 *   1. N3 ranked whisper census (evidence-ranked yielding)
 *   2. SHEP2D local rescue (regret -> bounded rescue gate)
 *   3. CEF_BOOST congestion FSM (sustained busy+redundant -> aggressive
 *      yielding; instant exit on any rescue signal)
 */

#if ADAPTIVE_RELAY_N3

#include "AdaptiveRelayN3.h"
#include "main.h" // LOG_*
#include "mesh-pb-constants.h"

namespace AdaptiveRelayN3 {

static uint32_t nowMs() { return millis(); }

// ---------------- layer 1: census ----------------

struct CensusEntry {
    NodeNum from;     // originator of the packet we are relaying
    PacketId id;      // its packet id
    uint8_t count;    // copies heard (incl. the triggering one)
    uint8_t rank;     // evidence rank of our pending rebroadcast
    uint32_t opened;  // millis when the entry was opened
    bool used;
};

static CensusEntry s_census[AR_N3_CENSUS_MAX];

static uint8_t rankOfSnr(float snr)
{
    if (snr >= AR_N3_SNR_STRONG_DB)
        return RANK_STRONG;
    if (snr >= AR_N3_SNR_WEAK_DB)
        return RANK_MEDIUM;
    return RANK_WEAK;
}

// ---------------- layer 2: SHEP2D local rescue ----------------

struct ParkEntry {         // a rebroadcast we yielded; watching for its death
    NodeNum from;
    PacketId id;
    uint32_t yielded;      // when we yielded it
    bool used;
};

static ParkEntry s_park[AR_SHEP_PARK_MAX];

static uint32_t s_deathTimes[AR_SHEP_DEATHS]; // ring of recent death timestamps
static uint8_t s_deathIdx = 0;
static uint32_t s_gateUntil = 0; // rescue gate lease (0 = closed)
static uint8_t s_gateBudget = 0; // kept-forward decisions remaining

// ---------------- layer 3: CEF_BOOST FSM ----------------

static uint32_t s_cbWindowStart = 0;
static uint32_t s_cbCopiesSum = 0; // summed census counts observed this window
static uint32_t s_cbSamples = 0;   // judged dupes this window
static float s_cbUtilPeak = 0.0f;  // peak channel utilization this window
static uint8_t s_cbStable = 0;     // consecutive busy+redundant windows
static bool s_cbBoost = false;

// ---------------- layer 2 bookkeeping ----------------

static void noteDeath(uint32_t now)
{
    s_deathTimes[s_deathIdx] = now;
    s_deathIdx = (s_deathIdx + 1) % AR_SHEP_DEATHS;
    LOG_DEBUG("AR-SHEP: suppressed packet died (regret)");

    // rescue signal: any death closes boost instantly (research semantics:
    // the FSM exits on the first frontier/regret evidence)
    if (s_cbBoost) {
        s_cbBoost = false;
        s_cbStable = 0;
        LOG_DEBUG("AR-CB: boost EXIT on rescue signal");
    }
}

static void rescueHousekeeping(uint32_t now)
{
    // expire parked packets: no further copy within the TTL -> death
    for (auto &q : s_park) {
        if (q.used && (now - q.yielded) > AR_SHEP_PARK_TTL_MS) {
            q.used = false;
            noteDeath(now);
        }
    }
    // sustained regret: >= AR_SHEP_DEATHS deaths within the window
    uint8_t deaths = 0;
    for (uint8_t i = 0; i < AR_SHEP_DEATHS; i++) {
        uint32_t t = s_deathTimes[i];
        if (t != 0 && (now - t) <= AR_SHEP_WINDOW_MS)
            deaths++;
        if (t != 0 && (now - t) > AR_SHEP_WINDOW_MS)
            s_deathTimes[i] = 0; // aged out
    }
    if (deaths >= AR_SHEP_DEATHS && (s_gateUntil == 0 || s_gateUntil < now)) {
        s_gateUntil = now + AR_SHEP_LEASE_MS;
        s_gateBudget = AR_SHEP_BUDGET;
        for (auto &d : s_deathTimes)
            d = 0; // window consumed by the gate decision
        LOG_INFO("AR-SHEP: rescue gate OPEN for %us (budget %u)", (unsigned)(AR_SHEP_LEASE_MS / 1000),
                 (unsigned)AR_SHEP_BUDGET);
    }
}

static bool gateOpen(uint32_t now)
{
    return s_gateUntil != 0 && now < s_gateUntil;
}

// ---------------- layer 3 bookkeeping ----------------

static void boostHousekeeping(uint32_t now, float utilPercent)
{
    if (utilPercent > s_cbUtilPeak)
        s_cbUtilPeak = utilPercent;

    if (s_cbWindowStart == 0 || (now - s_cbWindowStart) >= AR_CB_WINDOW_MS) {
        const float meanCopies = s_cbSamples ? (float)s_cbCopiesSum / (float)s_cbSamples : 0.0f;
        const bool busyRedundant = s_cbUtilPeak >= AR_CB_UTIL_MIN && meanCopies >= AR_CB_COPIES_MIN;

        if (s_cbBoost) {
            if (s_cbUtilPeak < AR_CB_EXIT_UTIL || meanCopies < AR_CB_COPIES_MIN) {
                s_cbBoost = false;
                s_cbStable = 0;
                LOG_INFO("AR-CB: boost EXIT (util %.1f, copies %.2f)", (double)s_cbUtilPeak, (double)meanCopies);
            }
        } else if (busyRedundant) {
            if (++s_cbStable >= AR_CB_STABLE) {
                s_cbBoost = true;
                LOG_INFO("AR-CB: boost ENTER after %u stable windows (util %.1f, copies %.2f)",
                         (unsigned)s_cbStable, (double)s_cbUtilPeak, (double)meanCopies);
            }
        } else {
            s_cbStable = 0;
        }

        s_cbWindowStart = now;
        s_cbCopiesSum = 0;
        s_cbSamples = 0;
        s_cbUtilPeak = 0.0f;
    }
}

// ---------------- shared helpers ----------------

static CensusEntry *findEntry(NodeNum from, PacketId id, bool pruneStale)
{
    const uint32_t now = nowMs();
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

static void park(NodeNum from, PacketId id, uint32_t now)
{
    for (auto &q : s_park) {
        if (q.used && q.from == from && q.id == id)
            return; // already watching
    }
    for (auto &q : s_park) {
        if (!q.used) {
            q.from = from;
            q.id = id;
            q.yielded = now;
            q.used = true;
            return;
        }
    }
    // park table full: drop the oldest (bounded state, research budget)
    ParkEntry *oldest = &s_park[0];
    for (auto &q : s_park)
        if (q.yielded < oldest->yielded)
            oldest = &q;
    oldest->from = from;
    oldest->id = id;
    oldest->yielded = now;
}

static void unpark(NodeNum from, PacketId id)
{
    for (auto &q : s_park) {
        if (q.used && q.from == from && q.id == id) {
            q.used = false; // it survived without our help
            return;
        }
    }
}

// ---------------- public API ----------------

void onRebroadcastQueued(const meshtastic_MeshPacket *p)
{
    const NodeNum from = getFrom(p);
    const PacketId id = p->id;
    const float snr = p->rx_snr;
    const uint32_t now = nowMs();

    CensusEntry *e = findEntry(from, id, true);
    if (e) {
        // already pending for this packet: refresh rank from the latest copy
        e->count = 1;
        e->rank = rankOfSnr(snr);
        e->opened = now;
        unpark(from, id); // a fresh copy means any older yield decision is moot
        return;
    }
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
    slot->opened = now;
    slot->used = true;

    LOG_DEBUG("AR-N3: census open 0x%08x/0x%x rank=%u (snr=%.1f)", from, id, slot->rank, (double)snr);
}

bool shouldYieldOnDupe(const meshtastic_MeshPacket *p, float channelUtilPercent)
{
    const uint32_t now = nowMs();

    // layer 2/3 bookkeeping runs lazily on the decision path (no timers)
    rescueHousekeeping(now);
    boostHousekeeping(now, channelUtilPercent);

    CensusEntry *e = findEntry(getFrom(p), p->id, true);
    if (!e)
        return true; // no census entry (e.g. our own retransmissions,
                     // already-relayed packets): exact stock K=1 behavior

    if (e->count < 255)
        e->count++;
    unpark(e->from, e->id); // this copy proves the packet still travels

    // layer 3 observation input: census redundancy of this window
    s_cbCopiesSum += e->count;
    s_cbSamples++;

    // layer 2: while the rescue gate is open we keep what we would
    // otherwise have suppressed, strictly within the budget
    if (gateOpen(now) && s_gateBudget > 0) {
        const uint8_t stockK = (e->rank == RANK_STRONG) ? 99 : (e->rank == RANK_MEDIUM ? AR_N3_K_MEDIUM : AR_N3_K_WEAK);
        if (e->count >= stockK) {
            s_gateBudget--; // one rescue forward spent
            LOG_DEBUG("AR-SHEP: rescue KEEP 0x%08x/0x%x (budget left %u)", e->from, e->id, (unsigned)s_gateBudget);
            return false;
        }
        return false; // below threshold anyway — not a budget consumer
    }

    // effective threshold: boost overrides rank (aggressive yielding to
    // protect a saturated channel), otherwise the N3 rank table
    const uint8_t k = s_cbBoost ? AR_CB_K
                    : (e->rank == RANK_STRONG) ? 99
                    : (e->rank == RANK_MEDIUM) ? AR_N3_K_MEDIUM
                                               : AR_N3_K_WEAK;

    if (e->count >= k) {
        LOG_DEBUG("AR-N3: yield 0x%08x/0x%x after %u copies (rank=%u%s)", e->from, e->id, e->count, e->rank,
                  s_cbBoost ? " BOOST" : "");
        park(e->from, e->id, now); // layer 2: watch whether it survives
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

uint8_t rescueGateOpen()
{
    return gateOpen(nowMs()) ? 1 : 0;
}

uint8_t boostActive()
{
    return s_cbBoost ? 1 : 0;
}

} // namespace AdaptiveRelayN3

#endif // ADAPTIVE_RELAY_N3
