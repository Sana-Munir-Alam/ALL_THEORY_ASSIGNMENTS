# CN Assignment 2 — Go-Back-N vs Selective Repeat over an unreliable satellite link

**Course:** Computer Networks, Chapter 3 (Transport Layer)
**Student:** Sana Munir Alam, Roll No. 24K0573
**Random seed:** 573 (last four digits of the roll number, 0573; an integer seed has no leading zero)
**Language:** Python 3.9+, standard library only (`matplotlib` is needed only for the Task 4 graph)
**Demo video:** https://drive.google.com/file/d/12dfZIEUSmtUn4zIgII2CfsGtaDKxRrSp/view?usp=sharing

## 1. What this project does
`rdt.py` implements Go-Back-N (GBN) and Selective Repeat (SR) sender/receiver pairs on top of a
channel simulated by the program itself (a discrete-event simulator). The channel can lose, corrupt and
delay both data packets and ACKs. Corruption is detected with the Internet checksum. rdt 3.0
(checksum + sequence numbers + ACKs + timer + retransmission) is the foundation; pipelining with a
window N turns it into GBN or SR. No ready-made GBN/SR/reliable-transport library is used.

Scenario modelled (Task description): satellite link R = 2 Mbps, RTT = 500 ms, L = 1000 bytes
(L/R = 4 ms, one-way propagation = 250 ms).

## 2. Files
| File | Purpose |
|---|---|
| `rdt.py` | Simulator + GBN/SR protocols + command-line interface (Task 3) |
| `experiments.py` | Runs scenarios A, B, C for GBN and SR; writes table, CSV, graph, logs (Task 4) |
| `selftest.py` | 23 automatic checks (traces, ACK/packet loss, corruption, delay, wrap-around, window rules) |
| `results/results.md`, `results/results.csv` | Task 4 results table |
| `results/gbn_vs_sr.png` | Task 4 graph (GBN vs SR) |
| `results/logs/*.log` | Full event log of each Task 4 run (send, receive, ACK, loss, timeout, retransmission) |

## 3. Setup
```
python3 --version            # 3.9 or newer
pip3 install matplotlib      # only needed to draw the graph
```

## 4. How to run
Sample command (SR, 200 packets, N=4, k=3, 10 % loss, log written to a file):
```
python3 rdt.py --protocol sr --packets 200 --window 4 --k 3 --loss 0.10 --seed 573 --quiet --log run.log
```
Without `--quiet` the event log is printed on screen. A summary block is always printed at the end.

### 4.1 Parameters
| Option | Meaning | Default |
|---|---|---|
| `--protocol gbn\|sr` | protocol | gbn |
| `--packets` | number of data packets | 200 |
| `--window` | window size N | 4 |
| `--k` | sequence-number bits; numbers wrap modulo 2^k | 3 |
| `--loss` | probability a packet or ACK is lost | 0.0 |
| `--corrupt` | probability a packet or ACK is corrupted (1 bit flipped) | 0.0 |
| `--delay` | probability a packet or ACK is delayed (extra 50–150 ms) | 0.0 |
| `--timeout` | retransmission timeout in ms | 850 |
| `--seed` | random seed (last 4 digits of roll number) | 573 |
| `--log FILE` / `--quiet` | save log to file / do not print log | – |
| `--rate-mbps`, `--rtt-ms`, `--packet-bytes` | link model | 2, 500, 1000 |
| `--delay-min`, `--delay-max` | extra-delay range in ms | 50, 150 |
| `--force` | allow a window larger than the safe limit (to show the ambiguity failure) | off |

### 4.2 Scripted faults (for deterministic demonstrations)
`--lose-data`, `--lose-ack`, `--corrupt-data`, `--corrupt-ack`, `--delay-data`, `--delay-ack` take a
comma-separated list of packet indices, e.g. `--lose-data 2`. The fault hits the FIRST transmission
only, so retransmissions get through. `--script-delay-ms` sets the delay (default 400 ms).

### 4.3 Log format
`[ time ms ] WHO  EVENT  details`, where WHO = SENDER, RECEIVER, CHANNEL or SYSTEM. Every send,
receive, ACK, loss (`X LOST`), corruption (`~ CORRUPTED`), delay (`> DELAYED`), timeout and
retransmission (`** RETRANSMISSION **`) produces one line. `idx` is the absolute packet number;
`seq` is the k-bit number carried in the header (`idx mod 2^k`).

## 5. Model and assumptions
1. One data packet leaves the sender every L/R = 4 ms; one-way propagation 250 ms, so an undisturbed
   packet is ACKed after RTT + L/R = 504 ms. ACK transmission time is ignored.
2. Packet and ACK headers are protected by the 16-bit Internet checksum (one's-complement sum,
   inverted). Corruption flips one random bit; the receiver/sender discards a packet whose checksum fails.
3. Loss, corruption and delay are applied independently to data packets AND ACKs.
4. Timers are started when a packet is handed to the link. Default timeout 850 ms, which is above the
   worst-case RTT (500 + 4 + 2 x 150 = 804 ms). A shorter timeout makes premature retransmissions
   whose stale copies can wrap around a small sequence space; the program prints a WARNING in that case.
5. Window limits: GBN needs N <= 2^k - 1 and SR needs N <= 2^(k-1); the program refuses larger
   windows unless `--force` is given. With reordering (delay) use N <= 2^(k-1) for both protocols.
6. Every run verifies the delivered data byte-for-byte and in order, and reports YES/NO.
7. Throughput = correctly delivered payload bits / simulated time to the last ACK.

## 6. Task 3 checklist: where each requirement is demonstrated
(Log lines below are copied from real runs with seed 573.)

| Requirement | Evidence in the log |
|---|---|
| **GBN** sliding window (base, nextseqnum) | `SEND pkt idx=4 seq=4 (new) [base=1 nextseqnum=5 window=1..4]` |
| GBN cumulative ACKs | ACK1 lost, then `cumulative ACK seq=2: everything up to idx=2 ACKed; base 1->3`: no retransmission needed |
| GBN one timer for the oldest unACKed packet | `TIMEOUT oldest unACKed idx=2 -> go back N: resend idx 2..5` fires 850 ms after the last ACK (508 + 850 = 1358 ms) |
| GBN timeout resends all outstanding | four `** RETRANSMISSION **` lines: idx 2, 3, 4, 5 |
| GBN receiver discards out-of-order | `pkt seq=3 but expectedseqnum=2 -> OUT OF ORDER, DISCARD (no buffering), re-ACK highest in-order` |
| **SR** sliding sender and receiver windows | sender `[base=2 nextseqnum=6 window=2..5 ACKed-in-window=[3]]`; receiver `rcv_base -> 6 (window 6..9)` |
| SR individual ACKs | `individual ACK seq=3: mark idx=3 ACKed; not the smallest unACKed (base stays 2)` |
| SR timer per packet | `TIMEOUT timer of idx=2 -> resend ONLY idx=2` (each packet has its own timer) |
| SR receiver buffers out-of-order packets | `pkt seq=3 in window but rcv_base=2 -> OUT OF ORDER, BUFFER it (buffered=[3])` |
| SR resends only missing, delivers in order | one retransmission, then `DELIVER in order idx [2, 3, 4, 5]` |
| Configurable parameters | all options in section 4.1 |
| Loss, corruption and delay on packets and ACKs | Scenario C, seed 573 (data/ACK): GBN lost 14/20, corrupted 9/11, delayed 34/27; SR lost 15/8, corrupted 8/11, delayed 20/27 |
| Corruption detected by checksum | `~ CORRUPTED pkt idx=2 (bit 8010 flipped)` then `corrupted packet (sim-meta idx=2) -> ignored, no ACK`; for ACKs `corrupted ACK (for idx=2) -> checksum failed, ignored` |
| Sequence numbers wrap modulo 2^k | `--k 2` run delivers 60/60 correctly (section 7, test 9) |
| Log line for every event | `results/logs/*.log`, or run without `--quiet` |

## 7. Test commands and expected results
Run from the project folder (seed 573 is the default).

| # | Command | What it shows | Expected key result |
|---|---|---|---|
| 1 | `python3 selftest.py` | all automatic checks | `ALL TESTS PASSED` |
| 2 | `python3 rdt.py --protocol gbn --packets 10 --window 4 --k 4 --lose-data 2` | GBN, packet 2 lost (Task 2) | 14 sent (4 retx), 13 ACKs, 3 discards, timeout at 1358 ms |
| 3 | `python3 rdt.py --protocol sr --packets 10 --window 4 --k 4 --lose-data 2` | SR, packet 2 lost (Task 2) | 11 sent (1 retx), 10 ACKs, 3 buffered, timeout at 850 ms |
| 4 | `python3 rdt.py --protocol gbn --packets 8 --window 4 --k 4 --lose-ack 1` | cumulative ACK hides a lost ACK | 0 retransmissions |
| 5 | `python3 rdt.py --protocol sr --packets 8 --window 4 --k 4 --lose-ack 1` | SR must resend and re-ACK | 1 retransmission, 1 duplicate |
| 6 | `python3 rdt.py --protocol sr --packets 8 --window 4 --k 4 --corrupt-data 2` | checksum catches a corrupted packet | 1 corrupted, 1 retransmission, 8/8 YES |
| 7 | `python3 rdt.py --protocol gbn --packets 8 --window 4 --k 4 --corrupt-ack 2` (and `--protocol sr`) | corrupted ACK ignored | GBN 0 retx, SR 1 retx |
| 8 | `python3 rdt.py --protocol gbn --packets 8 --window 4 --k 4 --delay-data 2` (and `sr`) | delayed packet | GBN 4 retx, SR 1 retx |
| 9 | `python3 rdt.py --protocol gbn --window 3 --k 2 --packets 60 --loss 0.15 --quiet` | sequence numbers wrap (k = 2) | 60/60 delivered, YES |
| 10 | `python3 rdt.py --protocol sr --window 5 --k 3` | unsafe window refused | `ERROR: SR with k=3 needs window N <= 4` |
| 11 | `python3 rdt.py --protocol sr --window 5 --k 3 --packets 12 --lose-ack 0,1,2,3,4 --force --quiet` | ambiguity with N > 2^(k-1) | `8/12 ... NO <-- DELIVERY ERROR` |
| 12 | `python3 rdt.py --protocol sr --packets 20 --delay 0.2 --timeout 400` | short timeout warning | `WARNING: timeout 400 ms < worst-case RTT 804 ms` |
| 13 | `python3 experiments.py --seed 573` | Task 4 table and graph | section 8 |

## 8. Task 4: experiments (200 packets, N = 4, k = 3, seed 573)
```
python3 experiments.py --seed 573
```
Creates `results/results.md`, `results/results.csv`, `results/gbn_vs_sr.png` and the six run logs.

| Scenario | Protocol | Packets transmitted | Retransmissions | Delivered | Sim. time (s) | Throughput (kbps) | Timeouts |
|---|---|---|---|---|---|---|---|
| A: reliable | GBN | 200 | 0 | 200/200 | 25.21 | 63.46 | 0 |
| A: reliable | SR | 200 | 0 | 200/200 | 25.21 | 63.46 | 0 |
| B: loss 0.10 | GBN | 312 | 112 | 200/200 | 54.72 | 29.24 | 28 |
| B: loss 0.10 | SR | 248 | 48 | 200/200 | 50.27 | 31.83 | 48 |
| C: loss .05, corrupt .05, delay .10 | GBN | 312 | 112 | 200/200 | 57.88 | 27.64 | 28 |
| C: loss .05, corrupt .05, delay .10 | SR | 242 | 42 | 200/200 | 48.54 | 32.96 | 42 |

Optional robustness check (mean over seeds 573..592): `python3 experiments.py --seed 573 --runs 20`
(it overwrites `results/`, so re-run the single-seed command afterwards).

## 9. Limitations
- A timeout shorter than the worst-case RTT can let stale duplicates wrap around the sequence space.
- GBN's N = 2^k - 1 limit assumes an in-order channel; with delay-induced reordering use N <= 2^(k-1).
- Results come from one seeded run per scenario; other seeds change B and C numbers by several percent.
