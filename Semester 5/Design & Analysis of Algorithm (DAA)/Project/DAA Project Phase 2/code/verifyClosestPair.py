import math
from pathlib import Path
from closestPair import closestPair, readPoints 

def bruteForceDistance(points):
    best = math.inf
    n = len(points)
    for i in range(n):
        xi, yi = points[i]
        for j in range(i + 1, n):
            dx = xi - points[j][0]
            dy = yi - points[j][1]
            d = dx * dx + dy * dy
            if d < best:
                best = d
    return math.sqrt(best)

def main():
    folder = Path(__file__).resolve().parent.parent / "datasets" / "closestPair"
    print(f"{'file':<10}{'n':>8}{'divide&conquer':>18}{'brute force':>16}{'result':>10}")
    allPassed = True
    for path in sorted(folder.glob("cp*.txt")):
        points = readPoints(path)
        d, p, q = closestPair(points)
        expected = bruteForceDistance(points)
        pairValid = p in points and q in points and math.isclose(math.hypot(p[0] - q[0], p[1] - q[1]), d)
        passed = math.isclose(d, expected, rel_tol=1e-9, abs_tol=1e-9) and pairValid
        allPassed = allPassed and passed
        print(f"{path.name:<10}{len(points):>8}{d:>18.6f}{expected:>16.6f}{'PASS' if passed else 'FAIL':>10}")
    print("\nAll files verified." if allPassed else "\nSome files FAILED.")

if __name__ == "__main__":
    main()