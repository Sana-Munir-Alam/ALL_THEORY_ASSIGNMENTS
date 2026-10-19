# selftest.py -- automatic checks.  Run:  python3 selftest.py   (prints PASS/FAIL per test)
import sys
from rdt import simulate

fails = 0
def check(name, cond, info=""):
    global fails
    print(("PASS  " if cond else "FAIL  ") + name + (f"   [{info}]" if info else ""))
    fails += 0 if cond else 1

def run(**kw):
    return simulate(kw)

# 1. Task 2 trace: 10 packets, N=4, pkt 2 lost -> GBN resends 2,3,4,5 ; SR resends only 2
g = run(protocol="gbn", packets=10, window=4, k=4, lose_data=[2])
s = run(protocol="sr",  packets=10, window=4, k=4, lose_data=[2])
check("Task2 GBN retransmits 4 (pkt 2,3,4,5)", g["retx"] == 4 and g["tx"] == 14 and g["acks"] == 13 and g["ok"], f"retx={g['retx']} tx={g['tx']} acks={g['acks']}")
check("Task2 SR retransmits 1 (pkt 2 only)",   s["retx"] == 1 and s["tx"] == 11 and s["acks"] == 10 and s["ok"], f"retx={s['retx']} tx={s['tx']} acks={s['acks']}")
check("Task2 SR buffered 3 out-of-order pkts", s["rx_buffered"] == 3)
check("Task2 GBN discarded 3 out-of-order pkts", g["rx_discard"] == 3)

# 2. Reliable channel -> zero retransmissions, everything delivered
for p in ("gbn", "sr"):
    r = run(protocol=p, packets=200, window=4, k=3)
    check(f"reliable channel {p.upper()}: 0 retx, 200 delivered", r["retx"] == 0 and r["ok"] and r["tx"] == 200)

# 3. Lost ACK: cumulative ACK covers it in GBN, SR needs a retransmission
g = run(protocol="gbn", packets=8, window=4, k=4, lose_ack=[1])
s = run(protocol="sr",  packets=8, window=4, k=4, lose_ack=[1])
check("lost ACK1: GBN needs NO retransmission (cumulative ACK2 covers it)", g["retx"] == 0 and g["ok"], f"retx={g['retx']}")
check("lost ACK1: SR retransmits only pkt 1 and re-ACKs", s["retx"] == 1 and s["ok"], f"retx={s['retx']}")

# 4. Last ACK lost: nothing covers it -> GBN times out and resends the last packet
g = run(protocol="gbn", packets=4, window=4, k=4, lose_ack=[3])
check("last ACK lost: GBN timeout, resend pkt 3, receiver re-ACKs", g["retx"] == 1 and g["ok"], f"retx={g['retx']}")

# 5. Corruption is caught by the checksum and recovered
for p in ("gbn", "sr"):
    r = run(protocol=p, packets=10, window=4, k=4, corrupt_data=[2])
    check(f"corrupted pkt 2 detected + recovered ({p.upper()})", r["data_corrupt"] == 1 and r["retx"] >= 1 and r["ok"])
    r = run(protocol=p, packets=10, window=4, k=4, corrupt_ack=[2])
    check(f"corrupted ACK 2 detected + recovered ({p.upper()})", r["ack_corrupt"] == 1 and r["ok"])

# 6. Delay -> premature timeout / reordering, still correct
for p in ("gbn", "sr"):
    r = run(protocol=p, packets=10, window=4, k=4, delay_data=[2], script_delay_ms=400)
    check(f"delayed pkt 2 (+400 ms) still delivered in order ({p.upper()})", r["ok"] and r["data_delayed"] == 1, f"retx={r['retx']}")

# 7. Property tests, many seeds, heavy errors, SMALL k so sequence numbers wrap around many times
allok = True
for seed in range(40):                 # N <= 2^(k-1): safe even when delay reorders packets/ACKs
    for p, N, k in (("gbn", 2, 2), ("gbn", 4, 3), ("sr", 2, 2), ("sr", 4, 3)):
        r = run(protocol=p, packets=100, window=N, k=k, loss=0.2, corrupt=0.1, delay=0.2, seed=seed)
        allok &= r["ok"]
check("160 runs, loss .2 corrupt .1 delay .2, wrap-around, N=2^(k-1): all correct", allok)
allok = True
for seed in range(40):                 # no delay (no reordering): GBN may use the full N = 2^k - 1
    for p, N, k in (("gbn", 3, 2), ("gbn", 7, 3), ("sr", 2, 2), ("sr", 4, 3)):
        r = run(protocol=p, packets=100, window=N, k=k, loss=0.2, corrupt=0.1, seed=seed)
        allok &= r["ok"]
check("160 runs, loss .2 corrupt .1, no reordering, GBN N=2^k-1 and SR N=2^(k-1): all correct", allok)
bad = sum(not run(protocol="gbn", packets=100, window=7, k=3, loss=.2, corrupt=.1, delay=.2, seed=s)["ok"] for s in range(10))
print(f"INFO  GBN N=7,k=3 (=2^k-1) WITH delay/reordering: {bad}/10 runs corrupted -> N=2^k-1 needs an in-order channel")

# 8. Window / sequence-number rules
def raises(**kw):
    try: run(**kw); return False
    except ValueError: return True
check("SR k=3 N=5 rejected (max 4)", raises(protocol="sr", window=5, k=3, packets=10))
check("GBN k=3 N=8 rejected (max 7)", raises(protocol="gbn", window=8, k=3, packets=10))
check("SR k=3 N=4 and GBN k=3 N=7 accepted", run(protocol="sr", window=4, k=3, packets=10)["ok"] and run(protocol="gbn", window=7, k=3, packets=10)["ok"])

# 9. Ambiguity demo (Task 5.2c): SR, k=3, N=5, all first ACKs 0-4 lost -> duplicate accepted as new
r = run(protocol="sr", window=5, k=3, packets=12, lose_ack=[0, 1, 2, 3, 4], force=True)
check("SR N=5,k=3 (too big): ambiguity makes the receiver deliver WRONG data", not r["ok"], f"delivered_ok={r['ok']}")
r = run(protocol="sr", window=4, k=3, packets=12, lose_ack=[0, 1, 2, 3], force=True)
check("SR N=4,k=3 (safe): same ACK-loss pattern is handled correctly", r["ok"])

# 10. Determinism
a = run(protocol="sr", packets=50, loss=.1, seed=7); b = run(protocol="sr", packets=50, loss=.1, seed=7)
check("same seed -> identical results", all(a[k] == b[k] for k in ("tx", "retx", "sim_ms")))

print("\nALL TESTS PASSED" if not fails else f"\n{fails} TEST(S) FAILED")
sys.exit(1 if fails else 0)