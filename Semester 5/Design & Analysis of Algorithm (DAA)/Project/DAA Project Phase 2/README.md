# Divide-and-Conquer Visualiser

**Closest Pair of Points and Karatsuba Integer Multiplication**
Course: Design and Analysis of Algorithms (DAA), Sir Farrukh Salim

**Group members**

| Name | Roll No. |
|---|---|
| Sana Munir Alam | 24K-0573 |
| Sharjeel Memon | 24K-0555 |
| Adeena Asif | 24K-0628 |

---

## About

This project implements two classic divide-and-conquer algorithms in Python, tests them on 20 seeded datasets, and checks every answer against an independent method.

| Problem | Method | Complexity |
|---|---|---|
| Closest pair of n points | Sort by x and y, split at the median, check the strip around the split line | Θ(n log n) |
| Multiply two n-digit integers | Karatsuba: 3 sub-products instead of 4 | Θ(n^1.585) |

A Tkinter desktop app loads any dataset file and lets you step through the recursion visually.

## Tech stack

- **Python 3.8+**
- **Tkinter** for the desktop interface (ships with Python)
- **matplotlib** for the plots embedded in the interface
- Everything else uses only the Python standard library

## Setup

```bash
pip install matplotlib
```

Tkinter normally comes with Python. On Linux, if it is missing: `sudo apt install python3-tk`.

## How to run

1. Open a terminal in the project folder.
2. Start the app:
   ```bash
   python3 app.py
   ```
3. Click **Browse file...** and choose a dataset (e.g. `cp01.txt` or `im01.txt`). The app detects whether the file holds points or integers.
4. Click **Run**. The status bar shows the result, running time and the correctness check.
5. Use **<< First / < Back / Next > / Last >>** to step through the recursion.

### Using the Closest Pair view

Load a point file (e.g. `cp01.txt`) and click **Run**. The closest pair algorithm finds the two nearest points among n points in Θ(n log n), instead of checking all n² pairs. The points are sorted once by x and once by y, then split at the median into a left and a right half, and each half is solved recursively. Pieces of 3 or fewer points are solved directly by brute force. To combine, let d be the smaller of the two half-answers. Only points within d of the dividing line can form a closer crossing pair. These "strip" points are scanned in y-order, and each is compared only with the next few points whose y differs by less than d (at most seven).

**Controls (top and bottom of the window)**

- **Levels shown:** a drop-down (2 to 11) that sets how many recursion levels are recorded and drawn. Each extra level doubles the number of steps, because there are twice as many sub-problems and each has a divide step and a combine step (for `cp01.txt`: 3 levels = 15 steps, 5 levels = 63 steps).
- **First / Back / Next / Last:** move through the steps. The counter shows your position (e.g. "Step 9 of 63").
- **Zoom to current subproblem:** zooms the plot to the region being worked on.
- **Magnifier (hover, scroll = zoom):** zooms in on a spot under the cursor, which helps with dense plots.

**What you see**

- **Plot (left):** the legend explains the colours.
  - Light blue shading: the current sub-problem.
  - Dashed blue line: the split line.
  - Orange points: the strip of candidate points.
  - Red segment: the closest pair found so far in this sub-problem.
- **What is happening (right):** a plain-language explanation of the current step.
  - A **DIVIDE** step shows how the points are split at the median.
  - A **COMBINE** step reports the best distance d from the two halves, how many points are in the strip, and whether a closer crossing pair was found (if not, d stays the same).
  - The last step zooms in on the closest pair.
- **Status bar (bottom):** the closest pair's coordinates, the distance, n, the running time, and the brute-force check (PASS/FAIL).

**What is and isn't drawn**

Pieces of 3 or fewer points are solved directly. Levels deeper than the one chosen in the drop-down are computed in exactly the same way, but not drawn, so the answer in the status bar always comes from the full recursion. For large files, the deepest levels create a very large number of steps, so start with a lower level and increase it only when you need more detail.

### Using the Karatsuba view

Load an integer file (e.g. `im01.txt`) and click **Run**. Karatsuba multiplies two n-digit numbers using 3 sub-products instead of 4. Each number is split into halves, so x = a·10^h + b and y = c·10^h + d. The app computes `ac`, `bd` and `(a+b)(c+d)`, then recovers the cross term by subtraction: `ad + bc = (a+b)(c+d) − ac − bd`.

**Controls (top of the window)**

- **Base-case cutoff (digits):** below this many digits the recursion stops and an ordinary multiplication is used. The app starts at 4 digits for display, while the report's benchmarks use 32.
- **Levels shown:** how many recursion levels are recorded and drawn.
- **Show full numbers:** switches between abbreviated numbers (e.g. `696044...223664`) and the complete digits.
- **Play / Next / Back / First / Last** and a slider: move through the steps by hand or play them automatically.

**What you see**

- **Calculation at this step (left):** the current call and its depth, the three sub-products, the cross-term calculation, the combine formula `x·y = ac·10^(2h) + (ad+bc)·10^h + bd`, and a "Check against x * y" line.
- **Call tree (right):** every sub-multiplication with its digit sizes and result, so you can see the recursion unfold.
- **Status bar (bottom):** product length, number of base multiplications, running time, and the check against Python's built-in multiplication (PASS/FAIL).

**Effect of the cutoff**

A smaller cutoff means more recursion. For `im01.txt` (101 digits), a cutoff of 4 digits gives 243 base-case calls, while the cutoff of 32 used in the report gives 9.

## File guide

| File | What it does |
|---|---|
| `app.py` | Tkinter + matplotlib interface; loads files, runs the solvers, draws the step-by-step views |
| `closestPair.py` | Divide-and-conquer closest-pair solver |
| `karatsuba.py` | Karatsuba multiplication solver (base-case cutoff 32 digits by default) |
| `generateDatasets.py` | Creates the 20 input files from fixed seeds and writes a manifest |
| `verifyClosestPair.py` | Checks every closest-pair answer against brute force |
| `stressTests.py` | Randomised tests: duplicate points, shared coordinates, zero and negative operands |
| `cp01.txt` – `cp10.txt` | Point datasets (uniform, clustered, near-collinear; 101 to 5,000 points) |
| `im01.txt` – `im10.txt` | Integer datasets (101 to 20,000 digits) |
| `report.docx` | Project report |

## Input formats

- **Point file:** first line is `n`, then one `x y` pair per line (four decimals).
- **Integer file:** two integers, one per line.

## Optional: regenerate and verify

```bash
python3 generateDatasets.py     # recreate all datasets
python3 verifyClosestPair.py    # compare against brute force
python3 stressTests.py          # randomised stress test
```

## Results (summary)

All 10 point files matched brute force and all 10 integer products matched Python's built-in multiplication. See the report for the full tables.
