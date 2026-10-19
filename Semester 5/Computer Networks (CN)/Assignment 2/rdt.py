import argparse, heapq, random, struct, sys, time

DATA, ACK = 1, 2

def inet_checksum(b: bytes) -> int:             #checksum
    if len(b) % 2:
        b += b"\x00"
    s = 0
    for i in range(0, len(b), 2):
        s += (b[i] << 8) | b[i + 1]
        s = (s & 0xFFFF) + (s >> 16)            # wrap the carry around
    return (~s) & 0xFFFF

def make_pkt(kind, seq, payload=b""):           # Wire format: kind(1B) seq(2B) checksum(2B) payload.
    ck = inet_checksum(struct.pack("!BH", kind, seq) + payload)
    return struct.pack("!BHH", kind, seq, ck) + payload

def parse_pkt(raw):
    if len(raw) < 5:
        return None
    kind, seq, ck = struct.unpack("!BHH", raw[:5])
    payload = raw[5:]
    if kind not in (DATA, ACK) or inet_checksum(struct.pack("!BH", kind, seq) + payload) != ck:
        return None
    return kind, seq, payload                    # Return (kind, seq, payload), or None if the checksum fails (corrupt).

DEFAULTS = dict(
    protocol="gbn", packets=200, window=4, k=3,
    loss=0.0, corrupt=0.0, delay=0.0,            # probabilities (packets AND ACKs)
    timeout=850.0,                               # ms (> worst-case RTT, see warning below)
    seed=573,                                   # <-- last 4 digits of roll number technically it's 0573 so had to write 573
    rate_mbps=2.0, rtt_ms=500.0, packet_bytes=1000,
    delay_min=50.0, delay_max=150.0,             # extra delay range (ms) when delayed
    # scripted faults: absolute packet index; hit the FIRST transmission only
    lose_data=[], lose_ack=[], corrupt_data=[], corrupt_ack=[],
    delay_data=[], delay_ack=[], script_delay_ms=400.0,
    force=False, max_events=400_000)


class Sim:
    def __init__(self, cfg, sink=None):
        c = self.c = dict(DEFAULTS, **cfg)
        self.sink = sink
        self.N, self.M, self.total, self.to = c["window"], 2 ** c["k"], c["packets"], c["timeout"]
        if c["protocol"] == "gbn":
            limit = self.M - 1
        else:
            limit = self.M // 2
        if self.N > limit and not c["force"]:
            raise ValueError(f"{c['protocol'].upper()} with k={c['k']} needs window N <= {limit} "
                             f"(got {self.N}). Use --force to see the ambiguity failure.")
        worst_rtt = c["rtt_ms"] + c["packet_bytes"] * 8 / (c["rate_mbps"] * 1e6) * 1000 + \
            (2 * c["delay_max"] if (c["delay"] > 0 or c["delay_data"] or c["delay_ack"]) else 0)
        self.warning = None
        if c["timeout"] < worst_rtt:
            self.warning = (f"WARNING: timeout {c['timeout']:.0f} ms < worst-case RTT {worst_rtt:.0f} ms -> "
                            f"premature timeouts create stale duplicates; with a small k they can wrap "
                            f"around the sequence space and be accepted as NEW data (Task 5.2c).")
        self.rng = random.Random(c["seed"])
        pay = random.Random(c["seed"] ^ 0xBEEF)
        self.payloads = [pay.randbytes(c["packet_bytes"]) for _ in range(self.total)]
        self.tx_ms = c["packet_bytes"] * 8 / (c["rate_mbps"] * 1e6) * 1000     # L/R
        self.prop = c["rtt_ms"] / 2                                             # one-way
        self.sc = {n: set(c[n]) for n in ("lose_data", "lose_ack", "corrupt_data", "corrupt_ack", "delay_data", "delay_ack")}
        self.t, self.q, self.cnt, self.link_free = 0.0, [], 0, 0.0
        self.tgen, self.running, self.acks_seen = {}, {}, set()
        self.delivered, self.done, self.end_t = [], False, 0.0
        self.st = dict(tx=0, retx=0, acks=0, data_lost=0, ack_lost=0, data_corrupt=0, ack_corrupt=0, data_delayed=0, ack_delayed=0, timeouts=0, rx_discard=0, rx_buffered=0, rx_dup=0, dup_ack_ignored=0)
        gbn = c["protocol"] == "gbn"
        self.sender = (GBNSender if gbn else SRSender)(self)
        self.receiver = (GBNReceiver if gbn else SRReceiver)(self)

    def log(self, who, msg):
        if self.sink:
            self.sink(f"[{self.t:9.2f} ms] {who:<8} {msg}")

    def at(self, t, fn, *args):
        heapq.heappush(self.q, (t, self.cnt, fn, args)); self.cnt += 1

    def set_timer(self, key, cb, arg=None):
        g = self.tgen.get(key, 0) + 1
        self.tgen[key] = g; self.running[key] = True
        self.at(self.t + self.to, self._fire, key, g, cb, arg)

    def cancel_timer(self, key):
        self.tgen[key] = self.tgen.get(key, 0) + 1; self.running[key] = False

    def _fire(self, key, g, cb, arg):
        if self.tgen.get(key) == g:
            self.running[key] = False
            cb(arg)

    def finish(self):
        self.done, self.end_t = True, self.t
        self.log("SYSTEM", f"all {self.total} packets ACKed -> transfer complete")

    def xmit_data(self, idx, retx): # Hand a data packet to the link; the link sends one packet every L/R ms.
        start = max(self.t, self.link_free)
        self.link_free = start + self.tx_ms
        self.at(start, self._tx_data, idx, retx)

    def _tx_data(self, idx, retx):
        if self.sender.stale(idx):                   # ACKed while waiting for the link
            return
        raw = make_pkt(DATA, idx % self.M, self.payloads[idx])
        self.st["tx"] += 1
        self.st["retx"] += 1 if retx else 0
        self.log("SENDER", f"SEND   pkt idx={idx} seq={idx % self.M} "
                           f"{'** RETRANSMISSION **' if retx else '(new)'}  {self.sender.state()}")
        self.channel_send(DATA, idx, raw, first=not retx)

    def send_ack(self, seq, ack_idx):
        self.st["acks"] += 1
        first = ack_idx not in self.acks_seen
        self.acks_seen.add(ack_idx)
        self.log("RECEIVER", f"SEND   ACK seq={seq} (for idx={ack_idx})")
        self.channel_send(ACK, ack_idx, make_pkt(ACK, seq), first)

    def channel_send(self, kind, idx, raw, first):
        c, pre = self.c, ("data" if kind == DATA else "ack")
        name = f"pkt idx={idx}" if kind == DATA else f"ACK (for idx={idx})"
        r1, r2, r3 = self.rng.random(), self.rng.random(), self.rng.random()
        if (first and idx in self.sc["lose_" + pre]) or r1 < c["loss"]:
            self.st[pre + "_lost"] += 1
            self.log("CHANNEL", f"X LOST  {name}")
            return
        if (first and idx in self.sc["corrupt_" + pre]) or r2 < c["corrupt"]:
            bit = self.rng.randrange(len(raw) * 8)
            b = bytearray(raw); b[bit // 8] ^= 1 << (bit % 8); raw = bytes(b)
            self.st[pre + "_corrupt"] += 1
            self.log("CHANNEL", f"~ CORRUPTED {name} (bit {bit} flipped)")
        extra = 0.0
        if (first and idx in self.sc["delay_" + pre]):
            extra = c["script_delay_ms"]
        elif r3 < c["delay"]:
            extra = self.rng.uniform(c["delay_min"], c["delay_max"])
        if extra:
            self.st[pre + "_delayed"] += 1
            self.log("CHANNEL", f"> DELAYED {name} by +{extra:.1f} ms")
        arrive = self.t + (self.tx_ms if kind == DATA else 0.0) + self.prop + extra
        self.at(arrive, self.rx_data if kind == DATA else self.rx_ack, raw, idx)

    def rx_data(self, raw, idx):
        self.receiver.on_data(raw, idx)

    def rx_ack(self, raw, idx):
        p = parse_pkt(raw)
        if p is None:
            self.log("SENDER", f"RECV   corrupted ACK (for idx={idx}) -> checksum failed, ignored")
            return
        self.sender.on_ack(p[1])

    def deliver(self, payload):
        self.delivered.append(payload)

    def run(self):
        w0 = time.perf_counter()
        c = self.c
        self.log("SYSTEM", f"{c['protocol'].upper()} packets={self.total} N={self.N} k={c['k']} "
                           f"(seq space 0..{self.M - 1}) loss={c['loss']} corrupt={c['corrupt']} "
                           f"delay={c['delay']} timeout={self.to}ms seed={c['seed']}")
        if self.warning:
            self.log("SYSTEM", self.warning)
        self.log("SYSTEM", f"L/R={self.tx_ms:.2f} ms  one-way={self.prop:.0f} ms  RTT={c['rtt_ms']:.0f} ms")
        self.sender.pump()
        n = 0
        while self.q and not self.done and n < c["max_events"]:
            t, _, fn, args = heapq.heappop(self.q)
            self.t = t; fn(*args); n += 1
        ok_prefix = 0
        for a, b in zip(self.delivered, self.payloads):
            if a != b:
                break
            ok_prefix += 1
        ok = self.done and len(self.delivered) == self.total and ok_prefix == self.total
        sim_ms = self.end_t if self.done else self.t
        s = dict(self.st)
        s.update(protocol=c["protocol"].upper(), packets=self.total, delivered=ok_prefix,
                 delivered_raw=len(self.delivered), ok=ok, finished=self.done, sim_ms=sim_ms,
                 throughput_kbps=(ok_prefix * c["packet_bytes"] * 8 / sim_ms) if sim_ms else 0.0,
                 wall_ms=(time.perf_counter() - w0) * 1000)
        s["efficiency"] = self.total / s["tx"] if s["tx"] else 0.0
        return s


# =============================================================== GO-BACK-N
class GBNSender: # Window [base, base+N-1]; ONE timer for the oldest unACKed packet; cumulative ACKs.
    def __init__(self, sim):
        self.sim, self.base, self.nextseqnum = sim, 0, 0

    def state(self):
        s = self.sim
        return f"[base={self.base} nextseqnum={self.nextseqnum} window={self.base}..{self.base + s.N - 1}]"

    def stale(self, idx):
        return idx < self.base

    def pump(self):                                    # rdt_send(): while window not full
        s = self.sim
        while self.nextseqnum < s.total and self.nextseqnum < self.base + s.N:
            s.xmit_data(self.nextseqnum, False)
            if self.base == self.nextseqnum:
                s.set_timer("gbn", self.on_timeout)
            self.nextseqnum += 1

    def on_ack(self, seq):
        s = self.sim
        for a in range(self.base, self.nextseqnum):
            if a % s.M == seq:
                break
        else:
            s.st["dup_ack_ignored"] += 1
            s.log("SENDER", f"RECV   ACK seq={seq} = duplicate/old -> ignored  {self.state()}")
            return
        old, self.base = self.base, a + 1
        s.log("SENDER", f"RECV   cumulative ACK seq={seq}: everything up to idx={a} ACKed; "
                        f"base {old}->{self.base}")
        if self.base == self.nextseqnum:
            s.cancel_timer("gbn")
        else:
            s.set_timer("gbn", self.on_timeout)
        if self.base >= s.total:
            s.finish()
        else:
            self.pump()

    def on_timeout(self, _):
        s = self.sim
        s.st["timeouts"] += 1
        s.log("SENDER", f"TIMEOUT oldest unACKed idx={self.base} -> go back N: resend "
                        f"idx {self.base}..{self.nextseqnum - 1}")
        s.set_timer("gbn", self.on_timeout)
        for a in range(self.base, self.nextseqnum):
            s.xmit_data(a, True)


class GBNReceiver: # Only remembers expectedseqnum; discards out-of-order packets; re-ACKs last in-order.
    def __init__(self, sim):
        self.sim, self.expectedseqnum = sim, 0

    def _reack(self):
        s, e = self.sim, self.expectedseqnum
        if e > 0:
            s.send_ack((e - 1) % s.M, e - 1)
        else:
            s.log("RECEIVER", "nothing delivered yet -> no ACK to repeat")

    def on_data(self, raw, idx):
        s = self.sim
        p = parse_pkt(raw)
        if p is None:
            s.st["rx_discard"] += 1
            s.log("RECEIVER", f"RECV   corrupted packet (sim-meta idx={idx}) -> discard, "
                              f"resend ACK of last in-order")
            self._reack(); return
        seq = p[1]
        if seq == self.expectedseqnum % s.M:
            s.deliver(p[2])
            s.log("RECEIVER", f"RECV   pkt seq={seq} == expectedseqnum -> DELIVER idx={self.expectedseqnum}; "
                              f"expectedseqnum {self.expectedseqnum}->{self.expectedseqnum + 1}")
            s.send_ack(seq, self.expectedseqnum)
            self.expectedseqnum += 1
        else:
            s.st["rx_discard"] += 1
            s.log("RECEIVER", f"RECV   pkt seq={seq} but expectedseqnum={self.expectedseqnum % s.M} -> "
                              f"OUT OF ORDER, DISCARD (no buffering), re-ACK highest in-order")
            self._reack()


# ============================================================ SELECTIVE REPEAT
class SRSender: # Window [base, base+N-1]; a timer PER packet; individual ACKs; resend only the missing.
    def __init__(self, sim):
        self.sim, self.base, self.nextseqnum, self.acked = sim, 0, 0, set()

    def state(self):
        s = self.sim
        a = sorted(x for x in self.acked if x >= self.base)
        return (f"[base={self.base} nextseqnum={self.nextseqnum} window={self.base}..{self.base + s.N - 1}"
                f" ACKed-in-window={a}]")

    def stale(self, idx):
        return idx < self.base or idx in self.acked

    def pump(self):
        s = self.sim
        while self.nextseqnum < s.total and self.nextseqnum < self.base + s.N:
            s.xmit_data(self.nextseqnum, False)
            s.set_timer(("sr", self.nextseqnum), self.on_timeout, self.nextseqnum)
            self.nextseqnum += 1

    def on_ack(self, seq):
        s = self.sim
        a = None
        for x in range(self.base, self.nextseqnum):
            if x % s.M == seq:
                a = x; break
        if a is None or a in self.acked:
            s.st["dup_ack_ignored"] += 1
            s.log("SENDER", f"RECV   ACK seq={seq} = duplicate/old -> ignored")
            return
        self.acked.add(a)
        s.cancel_timer(("sr", a))
        old = self.base
        while self.base in self.acked:
            self.base += 1
        why = (f"smallest unACKed -> base {old}->{self.base}" if self.base != old
               else f"not the smallest unACKed (base stays {self.base})")
        s.log("SENDER", f"RECV   individual ACK seq={seq}: mark idx={a} ACKed; {why}")
        if self.base >= s.total:
            s.finish()
        else:
            self.pump()

    def on_timeout(self, idx):
        s = self.sim
        if idx in self.acked:
            return
        s.st["timeouts"] += 1
        s.log("SENDER", f"TIMEOUT timer of idx={idx} -> resend ONLY idx={idx}")
        s.set_timer(("sr", idx), self.on_timeout, idx)
        s.xmit_data(idx, True)


class SRReceiver: # Window [rcv_base, rcv_base+N-1]; ACK every good packet; buffer out-of-order ones.
    def __init__(self, sim):
        self.sim, self.rcv_base, self.buf = sim, 0, {}

    def on_data(self, raw, idx):
        s = self.sim
        p = parse_pkt(raw)
        if p is None:
            s.st["rx_discard"] += 1
            s.log("RECEIVER", f"RECV   corrupted packet (sim-meta idx={idx}) -> ignored, no ACK")
            return
        seq, payload = p[1], p[2]
        off = (seq - self.rcv_base) % s.M
        if off < s.N:                                           # in [rcv_base, rcv_base+N-1]
            a = self.rcv_base + off
            s.send_ack(seq, a)
            if a in self.buf:
                s.st["rx_dup"] += 1
                s.log("RECEIVER", f"RECV   pkt seq={seq}: already buffered -> ACK again, keep one copy")
            elif a == self.rcv_base:
                run = [a]
                s.deliver(payload); self.rcv_base += 1
                while self.rcv_base in self.buf:
                    s.deliver(self.buf.pop(self.rcv_base)); run.append(self.rcv_base); self.rcv_base += 1
                s.log("RECEIVER", f"RECV   pkt seq={seq} == rcv_base -> DELIVER in order idx {run}; "
                                  f"rcv_base -> {self.rcv_base} (window {self.rcv_base}.."
                                  f"{self.rcv_base + s.N - 1})")
            else:
                self.buf[a] = payload
                s.st["rx_buffered"] += 1
                s.log("RECEIVER", f"RECV   pkt seq={seq} in window but rcv_base={self.rcv_base % s.M} "
                                  f"-> OUT OF ORDER, BUFFER it (buffered={sorted(self.buf)})")
        elif off >= s.M - s.N:                                  # in [rcv_base-N, rcv_base-1]
            a = self.rcv_base - (s.M - off)
            s.st["rx_dup"] += 1
            s.log("RECEIVER", f"RECV   pkt seq={seq}: already delivered (old window) -> RE-ACK it, "
                              f"do not deliver again")
            s.send_ack(seq, a)
        else:
            s.st["rx_discard"] += 1
            s.log("RECEIVER", f"RECV   pkt seq={seq}: outside both windows -> ignored")

def simulate(cfg, sink=None):
    return Sim(cfg, sink).run()

def _ints(txt):
    return [int(x) for x in txt.split(",") if x.strip() != ""]

def main(argv=None):
    ap = argparse.ArgumentParser(description="GBN / SR over a simulated unreliable satellite link")
    ap.add_argument("--protocol", choices=["gbn", "sr"], default="gbn")
    ap.add_argument("--packets", type=int, default=200)
    ap.add_argument("--window", type=int, default=4, help="window size N")
    ap.add_argument("--k", type=int, default=3, help="sequence-number bits (numbers wrap mod 2^k)")
    ap.add_argument("--loss", type=float, default=0.0)
    ap.add_argument("--corrupt", type=float, default=0.0)
    ap.add_argument("--delay", type=float, default=0.0, help="probability a packet/ACK is delayed")
    ap.add_argument("--timeout", type=float, default=850.0, help="ms (keep > RTT + L/R + 2*delay-max)")
    ap.add_argument("--seed", type=int, default=573, help="last 4 digits of your roll number")
    ap.add_argument("--rate-mbps", type=float, default=2.0)
    ap.add_argument("--rtt-ms", type=float, default=500.0)
    ap.add_argument("--packet-bytes", type=int, default=1000)
    ap.add_argument("--delay-min", type=float, default=50.0)
    ap.add_argument("--delay-max", type=float, default=150.0)
    for n in ("lose-data", "lose-ack", "corrupt-data", "corrupt-ack", "delay-data", "delay-ack"):
        ap.add_argument("--" + n, type=_ints, default=[], help="scripted fault, comma list of packet indices (first transmission only)")
    ap.add_argument("--script-delay-ms", type=float, default=400.0)
    ap.add_argument("--force", action="store_true", help="allow an unsafe window (shows ambiguity)")
    ap.add_argument("--log", help="write the full event log to this file")
    ap.add_argument("--quiet", action="store_true", help="do not print the event log")
    a = ap.parse_args(argv)
    cfg = {k: v for k, v in vars(a).items() if k not in ("log", "quiet")}
    cfg = {k.replace("-", "_"): v for k, v in cfg.items()}
    lines = []
    def sink(line):
        lines.append(line)
        if not a.quiet:
            print(line)
    try:
        r = simulate(cfg, sink)
    except ValueError as e:
        sys.exit(f"ERROR: {e}")
    if a.log:
        with open(a.log, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
    print("\n================ SUMMARY ================")
    print(f"protocol                : {r['protocol']}")
    print(f"data packets transmitted: {r['tx']}  (original {r['tx'] - r['retx']} + retransmissions {r['retx']})")
    print(f"ACKs sent               : {r['acks']}")
    print(f"timeouts                : {r['timeouts']}")
    print(f"lost    data/ACK        : {r['data_lost']}/{r['ack_lost']}")
    print(f"corrupt data/ACK        : {r['data_corrupt']}/{r['ack_corrupt']}")
    print(f"delayed data/ACK        : {r['data_delayed']}/{r['ack_delayed']}")
    print(f"receiver discards       : {r['rx_discard']}   buffered(SR): {r['rx_buffered']}   duplicates: {r['rx_dup']}")
    print(f"delivered correctly     : {r['delivered']}/{r['packets']}  in order, verified byte-for-byte: "
          f"{'YES' if r['ok'] else 'NO  <-- DELIVERY ERROR'}")
    print(f"simulated time          : {r['sim_ms'] / 1000:.3f} s   throughput: {r['throughput_kbps']:.2f} kbps")
    print(f"efficiency (pkts/tx)    : {r['efficiency']:.3f}   wall-clock: {r['wall_ms']:.1f} ms")
    return 0 if r["ok"] else 1

if __name__ == "__main__":
    sys.exit(main())