# Task 4: GBN vs SR, 200 packets, N=4, k=3, scenarios A, B, C.
# Writes  results/results.csv, results/results.md, results/gbn_vs_sr.png  and one log per run.
import argparse, csv, os, statistics
from rdt import simulate

SCENARIOS = [
    ("A: reliable", dict()),
    ("B: loss 0.10", dict(loss=0.10)),
    ("C: loss .05 corr .05 delay .10", dict(loss=0.05, corrupt=0.05, delay=0.10)),
]
FIELDS = ["tx", "retx", "delivered", "sim_ms", "throughput_kbps", "wall_ms", "timeouts", "acks", "rx_discard"]


def run_all(args):
    rows = []
    os.makedirs("results/logs", exist_ok=True)
    for name, extra in SCENARIOS:
        for proto in ("gbn", "sr"):
            samples = []
            for i in range(args.runs):
                cfg = dict(protocol=proto, packets=args.packets, window=args.window, k=args.k, timeout=args.timeout, seed=args.seed + i, **extra)
                lines = []
                r = simulate(cfg, lines.append if i == 0 else None)
                if i == 0:
                    tag = name.split(":")[0]
                    with open(f"results/logs/{tag}_{proto}_seed{args.seed}.log", "w", encoding="utf-8") as f:
                        f.write("\n".join(lines) + "\n")
                assert r["ok"], f"DELIVERY ERROR in {name} {proto} seed {cfg['seed']}"
                samples.append(r)
            row = dict(scenario=name, protocol=proto.upper(), runs=args.runs)
            for fld in FIELDS:
                vals = [s[fld] for s in samples]
                row[fld] = statistics.mean(vals)
                row[fld + "_sd"] = statistics.pstdev(vals) if len(vals) > 1 else 0.0
            rows.append(row)
    return rows


def write_outputs(rows, args):
    with open("results/results.csv", "w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
        w.writeheader(); w.writerows(rows)
    hdr = ("| Scenario | Protocol | Packets transmitted | Retransmissions | Delivered correctly | "
           "Sim. time (s) | Throughput (kbps) | Timeouts | Wall time (ms) |\n|---|---|---|---|---|---|---|---|---|\n")
    body = ""
    for r in rows:
        body += (f"| {r['scenario']} | {r['protocol']} | {r['tx']:.0f} | {r['retx']:.0f} | "
                 f"{r['delivered']:.0f}/{args.packets} | {r['sim_ms'] / 1000:.2f} | "
                 f"{r['throughput_kbps']:.2f} | {r['timeouts']:.0f} | {r['wall_ms']:.1f} |\n")
    title = (f"N={args.window}, k={args.k}, {args.packets} packets, timeout={args.timeout} ms, seed={args.seed}"
             + (f", mean of {args.runs} runs" if args.runs > 1 else ""))
    with open("results/results.md", "w") as f:
        f.write(f"**{title}**\n\n{hdr}{body}")
    print(f"\n{title}\n\n{hdr}{body}")
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("matplotlib not installed: pip install matplotlib   (table/CSV were still written)")
        return
    names = [s[0].split(":")[0] + ":\n" + s[0].split(":")[1].strip() for s in SCENARIOS]
    fig, ax = plt.subplots(1, 2, figsize=(11, 4.2))
    for a, key, ylabel, title2 in ((ax[0], "retx", "Retransmitted packets", "Retransmissions"),
                                   (ax[1], "throughput_kbps", "Throughput (kbps)", "Throughput")):
        for j, (proto, col, hatch) in enumerate((("GBN", "#6b1526", ""), ("SR", "#c9a24b", "//"))):
            vals = [r[key] for r in rows if r["protocol"] == proto]
            errs = [r[key + "_sd"] for r in rows if r["protocol"] == proto]
            xs = [i + (j - 0.5) * 0.38 for i in range(3)]
            bars = a.bar(xs, vals, 0.38, yerr=errs if args.runs > 1 else None, color=col,
                         edgecolor="black", hatch=hatch, label=proto, capsize=3)
            for b, v in zip(bars, vals):
                a.text(b.get_x() + b.get_width() / 2, b.get_height(), f"{v:.1f}" if key != "retx" else f"{v:.0f}",
                       ha="center", va="bottom", fontsize=8)
        a.set_xticks(range(3)); a.set_xticklabels(names, fontsize=8)
        a.set_ylabel(ylabel); a.set_title(title2); a.legend(); a.grid(axis="y", alpha=.3)
    fig.suptitle(f"GBN vs SR (200 pkts, N={args.window}, k={args.k}, seed {args.seed}" + (f", mean of {args.runs} runs)" if args.runs > 1 else ")"))
    fig.tight_layout(); fig.savefig("results/gbn_vs_sr.png", dpi=160)
    print("saved results/gbn_vs_sr.png, results/results.csv, results/results.md, results/logs/*.log")


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--seed", type=int, default=573, help="last 4 digits of roll number")
    ap.add_argument("--runs", type=int, default=1, help="average over this many seeds")
    ap.add_argument("--packets", type=int, default=200)
    ap.add_argument("--window", type=int, default=4)
    ap.add_argument("--k", type=int, default=3)
    ap.add_argument("--timeout", type=float, default=850.0)
    a = ap.parse_args()
    write_outputs(run_all(a), a)