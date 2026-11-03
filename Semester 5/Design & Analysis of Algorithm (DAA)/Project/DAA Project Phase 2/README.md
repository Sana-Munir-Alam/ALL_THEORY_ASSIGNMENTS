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

- **Levels shown** drop-down (2 to 11) sets how many recursion levels are recorded and drawn.
- Each extra level **doubles** the number of steps (for `cp01.txt`: 3 levels = 15 steps, 5 levels = 63 steps).
- Pieces of 3 or fewer points are solved directly. Deeper levels are computed the same way but not drawn.
- The plot shows the current sub-problem, the split line, the strip of candidate points and the best pair so far, with a plain-language explanation on the right.
- **Zoom to current subproblem** and **Magnifier** help with dense plots.

### Using the Karatsuba view

- Set the base-case cutoff and the number of levels shown, then step through (or play) the split, the three sub-products and the final combination.
- A call tree and a per-level table of call counts are shown alongside.

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