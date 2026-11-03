import random
import sys
from pathlib import Path 

if hasattr(sys, "set_int_max_str_digits"):
    sys.set_int_max_str_digits(0)

rootDir = Path(__file__).resolve().parent.parent / "datasets"
pointConfigs = [
    ("uniform", 101), ("uniform", 500), ("uniform", 2000), ("uniform", 5000),
    ("clustered", 200), ("clustered", 1000), ("clustered", 3000),
    ("collinear", 300), ("collinear", 1500), ("collinear", 4000),
]
digitCounts = [101, 200, 500, 1000, 2000, 3000, 5000, 8000, 12000, 20000]

def uniformPoints(rng, n):
    return [(rng.uniform(0, 10000), rng.uniform(0, 10000)) for i in range(n)]

def clusteredPoints(rng, n):
    centers = [(rng.uniform(1000, 9000), rng.uniform(1000, 9000)) for i in range(5)]
    points = []
    for i in range(n):
        cx, cy = rng.choice(centers)
        points.append((rng.gauss(cx, 150), rng.gauss(cy, 150)))
    return points

def collinearPoints(rng, n):
    points = []
    for i in range(n):
        x = rng.uniform(0, 10000)
        points.append((x, 0.5 * x + 100 + rng.gauss(0, 2)))
    return points

generators = {
    "uniform": uniformPoints,
    "clustered": clusteredPoints,
    "collinear": collinearPoints,
}

def writeManifest(folder, rows):
    lines = [f"{'file':<10}{'type':<12}{'size':>8}{'seed':>8}"]
    lines += [f"{name:<10}{kind:<12}{size:>8}{seed:>8}" for name, kind, size, seed in rows]
    (folder / "manifest.txt").write_text("\n".join(lines) + "\n")
    print("\n".join(lines))

def generatePointFiles():
    folder = rootDir / "closestPair"
    folder.mkdir(parents=True, exist_ok=True)
    rows = []
    for i, (kind, n) in enumerate(pointConfigs, start=1):
        seed = 1000 + i
        points = generators[kind](random.Random(seed), n)
        lines = [str(n)] + [f"{x:.4f} {y:.4f}" for x, y in points]
        name = f"cp{i:02d}.txt"
        (folder / name).write_text("\n".join(lines) + "\n")
        rows.append((name, kind, n, seed))
    writeManifest(folder, rows)

def generateNumberFiles():
    folder = rootDir / "integerMult"
    folder.mkdir(parents=True, exist_ok=True)
    rows = []
    for i, digits in enumerate(digitCounts, start=1):
        seed = 2000 + i
        rng = random.Random(seed)
        x = rng.randrange(10 ** (digits - 1), 10 ** digits)
        y = rng.randrange(10 ** (digits - 1), 10 ** digits)
        name = f"im{i:02d}.txt"
        (folder / name).write_text(f"{x}\n{y}\n")
        rows.append((name, "random", digits, seed))
    writeManifest(folder, rows)

if __name__ == "__main__":
    generatePointFiles()
    print()
    generateNumberFiles()