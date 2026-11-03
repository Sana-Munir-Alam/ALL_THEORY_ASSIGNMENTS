import sys
import time
from pathlib import Path 

if hasattr(sys, "set_int_max_str_digits"):
    sys.set_int_max_str_digits(0)

cutoffDigits = 32  # below this size use plain multiplication (set to 1 for the pure textbook version)
baseCalls = 0

def karatsubaRec(x, y):
    # x and y are non-negative integers
    global baseCalls
    xs, ys = str(x), str(y)
    if min(len(xs), len(ys)) <= cutoffDigits:
        baseCalls += 1
        return x * y
    n = max(len(xs), len(ys))
    if n % 2:
        n += 1  # pad to an even length so both halves are equal
    
    half = n // 2
    xs, ys = xs.zfill(n), ys.zfill(n)
    a, b = int(xs[:half]), int(xs[half:])
    c, d = int(ys[:half]), int(ys[half:])
    ac = karatsubaRec(a, c)
    bd = karatsubaRec(b, d)
    cross = karatsubaRec(a + b, c + d) - ac - bd  # equals ad + bc
    
    return ac * 10 ** (2 * half) + cross * 10 ** half + bd

def karatsuba(x, y):
    global baseCalls
    baseCalls = 0
    sign = -1 if (x < 0) != (y < 0) else 1
    return sign * karatsubaRec(abs(x), abs(y))

def readNumbers(path):
    tokens = Path(path).read_text().split()
    return int(tokens[0]), int(tokens[1])

def bestTime(function, *args, runs=3):
    best = float("inf")
    result = None
    for i in range(runs):
        start = time.perf_counter()
        result = function(*args)
        best = min(best, time.perf_counter() - start)
    return result, best

def preview(number):
    s = str(number)
    return s if len(s) <= 20 else f"{s[:8]}...{s[-8:]}"

def main():
    folder = Path(__file__).resolve().parent.parent / "datasets" / "integerMult"
    outFolder = folder / "outputs"
    outFolder.mkdir(exist_ok=True)
    
    print(f"{'file':<10}{'digits':>8}{'product digits':>16}{'product (preview)':>24}{'base mults':>12}{'correct':>10}{'time (s)':>11}")
    for path in sorted(folder.glob("im*.txt")):
        x, y = readNumbers(path)
        product, elapsed = bestTime(karatsuba, x, y)
        calls = baseCalls
        status = "PASS" if product == x * y else "FAIL" # Comparision check of our algo with Python's built-in multiplication
        (outFolder / f"{path.stem}Out.txt").write_text(f"{product}\n")
        print(f"{path.name:<10}{len(str(x)):>8}{len(str(product)):>16}{preview(product):>24}{calls:>12}{status:>10}{elapsed:>11.4f}")

if __name__ == "__main__":
    main()