import math
import time
from pathlib import Path 

def readPoints(path):
    tokens = Path(path).read_text().split()
    n = int(tokens[0])
    values = list(map(float, tokens[1:1 + 2 * n]))
    return list(zip(values[0::2], values[1::2]))

def distance(p, q):
    return math.hypot(p[0] - q[0], p[1] - q[1])

def bruteForce(points):
    best = (math.inf, None, None)
    for i in range(len(points)):
        for j in range(i + 1, len(points)):
            d = distance(points[i], points[j])
            if d < best[0]:
                best = (d, points[i], points[j])
    return best

def closestRec(px, py):
    # px: points sorted by x, py: the same points sorted by y
    n = len(px)
    if n <= 3:
        return bruteForce(px)
    mid = n // 2
    midX = px[mid][0]
    leftIds = {p[2] for p in px[:mid]}
    leftY = [p for p in py if p[2] in leftIds]
    rightY = [p for p in py if p[2] not in leftIds]
    best = min(closestRec(px[:mid], leftY), closestRec(px[mid:], rightY), key=lambda r: r[0])
    d = best[0]
    strip = [p for p in py if abs(p[0] - midX) < d]
    for i in range(len(strip)):
        for j in range(i + 1, len(strip)):
            if strip[j][1] - strip[i][1] >= d:
                break
            dist = distance(strip[i], strip[j])
            if dist < d:
                d = dist
                best = (dist, strip[i], strip[j])
    return best

def closestPair(points):
    if len(points) < 2:
        raise ValueError("at least two points are required")
    indexed = [(x, y, i) for i, (x, y) in enumerate(points)]
    px = sorted(indexed)
    py = sorted(indexed, key=lambda p: (p[1], p[0]))
    d, p, q = closestRec(px, py)
    return d, p[:2], q[:2]

def main():
    folder = Path(__file__).resolve().parent.parent / "datasets" / "closestPair"
    print(f"{'file':<10}{'n':>8}{'distance':>14}{'point 1':>28}{'point 2':>28}{'time (s)':>12}")
    for path in sorted(folder.glob("cp*.txt")):
        points = readPoints(path)
        start = time.perf_counter()
        d, p, q = closestPair(points)
        elapsed = time.perf_counter() - start
        p1 = f"({p[0]:.4f}, {p[1]:.4f})"
        p2 = f"({q[0]:.4f}, {q[1]:.4f})"
        print(f"{path.name:<10}{len(points):>8}{d:>14.6f}{p1:>28}{p2:>28}{elapsed:>12.4f}")

if __name__ == "__main__":
    main()