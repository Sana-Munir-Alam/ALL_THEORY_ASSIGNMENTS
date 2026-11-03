# ------------------------------------------------------------------------------
# PROJECT MEMBERS
# Name: Sana Munir Alam [24K-0573]
# Name: Sharjeel Memon [24K-0574]
# Name: Adeena Asif [24K-0628]
# ------------------------------------------------------------------------------
import math
import time
import tkinter as tk
from pathlib import Path
from tkinter import ttk, filedialog, messagebox

import numpy as np
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.lines import Line2D

matplotlib.rcParams.update({"font.size": 12, "axes.titlesize": 14, "axes.labelsize": 12})

import closestPair as cp
import karatsuba as kt
import sys

UI_FONT = "UI_FONT" if sys.platform.startswith("win") else "Helvetica Neue" if sys.platform == "darwin" else "DejaVu Sans"
MONO_FONT = "MONO_FONT" if sys.platform.startswith("win") else "Menlo" if sys.platform == "darwin" else "DejaVu Sans Mono"

LENS_PX = 300  # size of the magnifier lens in pixels
DEFAULT_DEPTH = 5  # recursion levels drawn for closest pair (changeable in the window)

BG = "#f4f6fa"
ACCENT = "#2b5fb4"


# ---------------------------------------------------------------- tracing helpers
def traceClosest(points, maxDepth):
    """Same algorithm as closestPair.py, but records the divide/combine steps of the top levels."""
    events = []
    indexed = [(x, y, i) for i, (x, y) in enumerate(points)]
    px = sorted(indexed)
    py = sorted(indexed, key=lambda p: (p[1], p[0]))

    def rec(px, py, depth):
        n = len(px)
        if n <= 3:
            return cp.bruteForce(px)
        mid = n // 2
        midX = px[mid][0]
        lo, hi = px[0][0], px[-1][0]
        if depth < maxDepth:
            events.append(dict(kind="divide", depth=depth, n=n, lo=lo, hi=hi, midX=midX))
        leftIds = {p[2] for p in px[:mid]}
        leftY = [p for p in py if p[2] in leftIds]
        rightY = [p for p in py if p[2] not in leftIds]
        best = min(rec(px[:mid], leftY, depth + 1), rec(px[mid:], rightY, depth + 1), key=lambda r: r[0])
        dHalves = best[0]
        d = dHalves
        strip = [p for p in py if abs(p[0] - midX) < d]
        for i in range(len(strip)):
            for j in range(i + 1, len(strip)):
                if strip[j][1] - strip[i][1] >= d:
                    break
                dist = cp.distance(strip[i], strip[j])
                if dist < d:
                    d = dist
                    best = (dist, strip[i], strip[j])
        if depth < maxDepth:
            events.append(dict(kind="combine", depth=depth, n=n, lo=lo, hi=hi, midX=midX,
                               dHalves=dHalves, dFinal=d, strip=[(p[0], p[1]) for p in strip],
                               pair=(best[1][:2], best[2][:2])))
        return best

    d, p, q = rec(px, py, 0)
    return d, p[:2], q[:2], events


def traceKaratsuba(x, y, cutoff):
    """Same recursion as karatsuba.py; counts calls per level."""
    levels = {}

    def rec(x, y, depth):
        xs, ys = str(x), str(y)
        row = levels.setdefault(depth, [0, 0, 0])  # calls, base calls, largest operand (digits)
        row[0] += 1
        row[2] = max(row[2], len(xs), len(ys))
        if min(len(xs), len(ys)) <= cutoff:
            row[1] += 1
            return x * y
        n = max(len(xs), len(ys))
        if n % 2:
            n += 1
        half = n // 2
        xs, ys = xs.zfill(n), ys.zfill(n)
        a, b = int(xs[:half]), int(xs[half:])
        c, d = int(ys[:half]), int(ys[half:])
        ac = rec(a, c, depth + 1)
        bd = rec(b, d, depth + 1)
        cross = rec(a + b, c + d, depth + 1) - ac - bd
        return ac * 10 ** (2 * half) + cross * 10 ** half + bd

    return rec(x, y, 0), levels


def buildKaratsubaTrace(x, y, cutoff, maxShow):
    """Record every call of the top maxShow levels, plus the order of split/combine steps."""
    nodes, steps = [], []

    def rec(x, y, depth, parent, label):
        nd = dict(id=len(nodes), parent=parent, depth=depth, label=label, x=x, y=y)
        nodes.append(nd)
        xs, ys = str(x), str(y)
        if min(len(xs), len(ys)) <= cutoff:
            nd["result"] = x * y
            steps.append(("base", nd["id"]))
            return nd
        if depth >= maxShow:
            nd["result"] = kt.karatsubaRec(x, y)
            steps.append(("hidden", nd["id"]))
            return nd
        n = max(len(xs), len(ys))
        nd["pad"] = bool(n % 2)
        if n % 2:
            n += 1
        half = n // 2
        xp, yp = xs.zfill(n), ys.zfill(n)
        a, b, c, d = int(xp[:half]), int(xp[half:]), int(yp[:half]), int(yp[half:])
        nd.update(n=n, half=half, a=a, b=b, c=c, d=d)
        steps.append(("split", nd["id"]))
        r1 = rec(a, c, depth + 1, nd["id"], "ac")
        r2 = rec(b, d, depth + 1, nd["id"], "bd")
        r3 = rec(a + b, c + d, depth + 1, nd["id"], "(a+b)(c+d)")
        nd["ac"], nd["bd"], nd["mid"] = r1["result"], r2["result"], r3["result"]
        nd["cross"] = nd["mid"] - nd["ac"] - nd["bd"]
        nd["result"] = nd["ac"] * 10 ** (2 * half) + nd["cross"] * 10 ** half + nd["bd"]
        steps.append(("combine", nd["id"]))
        return nd

    rec(x, y, 0, None, "x * y")
    return nodes, steps


def detectKind(path):
    tokens = Path(path).read_text().split()
    if len(tokens) == 2 and all(t.isdigit() for t in tokens):
        return "mult"
    if tokens and tokens[0].isdigit() and len(tokens) == 1 + 2 * int(tokens[0]):
        return "points"
    return None


def short(number, keep=10):
    s = str(number)
    return s if len(s) <= 2 * keep + 3 else f"{s[:keep]}...{s[-keep:]} ({len(s)} digits)"


# ---------------------------------------------------------------- application
class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Divide and Conquer Visualiser - Closest Pair and Karatsuba")
        self.geometry("1200x760")
        self.minsize(980, 620)
        self.configure(bg=BG)
        self.path = None
        self.kind = None
        self.points = None
        self.events = []
        self.step = 0
        self.result = None
        self.kNodes, self.kSteps, self.kIdx, self.kPlaying, self.kProduct = [], [], 0, False, 0
        self.lens = None
        self.lensLabel = None
        self.scene = None
        self.magZoom = 6.0
        self.lastMove = 0.0
        self.style = ttk.Style(self)
        self.style.theme_use("clam")
        self.style.configure("TFrame", background=BG)
        self.style.configure("TLabel", background=BG, font=("UI_FONT", 12))
        self.style.configure("Head.TLabel", font=("UI_FONT", 18, "bold"), foreground=ACCENT)
        self.style.configure("Result.TLabel", font=("UI_FONT", 12, "bold"), foreground="#1a1a1a",
                             background="#e4ebf7", padding=8)
        self.style.configure("Treeview", font=("UI_FONT", 12), rowheight=28)
        self.style.configure("Treeview.Heading", font=("UI_FONT", 11, "bold"))
        self.style.configure("TButton", font=("UI_FONT", 12), padding=8)
        self.style.configure("Accent.TButton", font=("UI_FONT", 12, "bold"))
        self.buildTopBar()
        self.body = ttk.Frame(self)
        self.body.pack(fill="both", expand=True, padx=12, pady=(0, 6))
        self.buildClosestView()
        self.buildKaratsubaView()
        self.resultVar = tk.StringVar(value="Choose a dataset file to begin.")
        ttk.Label(self, textvariable=self.resultVar, style="Result.TLabel", anchor="w").pack(
            fill="x", padx=12, pady=(0, 12))

    # ---- layout
    def buildTopBar(self):
        bar = ttk.Frame(self)
        bar.pack(fill="x", padx=12, pady=10)
        ttk.Label(bar, text="Divide and Conquer Visualiser", style="Head.TLabel").pack(side="left")
        ttk.Button(bar, text="Run", style="Accent.TButton", command=self.run).pack(side="right")
        ttk.Button(bar, text="Browse file...", command=self.browse).pack(side="right", padx=8)
        self.fileVar = tk.StringVar(value="no file selected")
        ttk.Label(bar, textvariable=self.fileVar).pack(side="right", padx=8)

    def buildClosestView(self):
        self.cpView = ttk.Frame(self.body)
        left = ttk.Frame(self.cpView)
        left.pack(side="left", fill="both", expand=True)
        opts = ttk.Frame(left)
        opts.pack(fill="x", pady=(0, 4))
        ttk.Label(opts, text="Levels shown:").pack(side="left")
        self.cpDepthVar = tk.StringVar(value=str(DEFAULT_DEPTH))
        cbd = ttk.Combobox(opts, textvariable=self.cpDepthVar, values=[str(i) for i in range(2, 12)],
                           width=4, state="readonly")
        cbd.pack(side="left", padx=6)
        cbd.bind("<<ComboboxSelected>>", lambda e: self.changeDepth())
        ttk.Label(opts, text="(each extra level doubles the number of steps)").pack(side="left")
        self.fig = Figure(figsize=(6.5, 5.2), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.canvas = FigureCanvasTkAgg(self.fig, master=left)
        self.canvas.get_tk_widget().pack(fill="both", expand=True)
        self.canvas.mpl_connect("motion_notify_event", self.onMove)
        self.canvas.mpl_connect("scroll_event", self.onScroll)
        self.canvas.mpl_connect("axes_leave_event", lambda e: self.hideLens())
        nav = ttk.Frame(left)
        nav.pack(fill="x", pady=4)
        ttk.Button(nav, text="<< First", command=lambda: self.goto(0)).pack(side="left")
        ttk.Button(nav, text="< Back", command=lambda: self.goto(self.step - 1)).pack(side="left", padx=4)
        ttk.Button(nav, text="Next >", command=lambda: self.goto(self.step + 1)).pack(side="left")
        ttk.Button(nav, text="Last >>", command=lambda: self.goto(len(self.events))).pack(side="left", padx=4)
        self.zoomVar = tk.BooleanVar(value=True)
        ttk.Checkbutton(nav, text="Zoom to current subproblem", variable=self.zoomVar,
                        command=self.draw).pack(side="right")
        self.magVar = tk.BooleanVar(value=False)
        ttk.Checkbutton(nav, text="Magnifier (hover, scroll = zoom)", variable=self.magVar,
                        command=self.hideLens).pack(side="right", padx=10)
        self.stepVar = tk.StringVar(value="")
        ttk.Label(nav, textvariable=self.stepVar).pack(side="right", padx=10)
        right = ttk.Frame(self.cpView, width=400)
        right.pack(side="right", fill="y", padx=(10, 0))
        right.pack_propagate(False)
        ttk.Label(right, text="What is happening", style="Head.TLabel").pack(anchor="w")
        self.cpText = tk.Text(right, wrap="word", font=(UI_FONT, 12), bg="white", fg="#1a1a1a",
                              insertbackground="#1a1a1a", relief="flat", padx=10, pady=8, height=18)
        self.cpText.pack(fill="both", expand=True, pady=6)
        self.cpText.configure(state="disabled")

    def buildKaratsubaView(self):
        self.ktView = ttk.Frame(self.body)
        opts = ttk.Frame(self.ktView)
        opts.pack(fill="x", pady=(0, 6))
        ttk.Label(opts, text="Base-case cutoff (digits):").pack(side="left")
        self.cutoffVar = tk.StringVar(value="32")
        cb1 = ttk.Combobox(opts, textvariable=self.cutoffVar, values=["4", "8", "16", "32", "64"],
                           width=5, state="readonly")
        cb1.pack(side="left", padx=6)
        ttk.Label(opts, text="Levels shown:").pack(side="left", padx=(16, 0))
        self.depthVar = tk.StringVar(value="5")
        cb2 = ttk.Combobox(opts, textvariable=self.depthVar, values=["2", "3", "4", "5", "6", "7"], width=4, state="readonly")
        cb2.pack(side="left", padx=6)
        self.fullVar = tk.BooleanVar(value=False)
        ttk.Checkbutton(opts, text="Show full numbers", variable=self.fullVar,
                        command=self.kDraw).pack(side="left", padx=16)
        for c in (cb1, cb2):
            c.bind("<<ComboboxSelected>>", lambda e: self.run())
        ctl = ttk.Frame(self.ktView)
        ctl.pack(side="bottom", fill="x", pady=(6, 0))
        ttk.Button(ctl, text="<< First", command=lambda: self.kGoto(0)).pack(side="left")
        ttk.Button(ctl, text="< Back", command=lambda: self.kGoto(self.kIdx - 1)).pack(side="left", padx=4)
        self.playBtn = ttk.Button(ctl, text="Play", command=self.kTogglePlay)
        self.playBtn.pack(side="left")
        ttk.Button(ctl, text="Next >", command=lambda: self.kGoto(self.kIdx + 1)).pack(side="left", padx=4)
        ttk.Button(ctl, text="Last >>", command=lambda: self.kGoto(len(self.kSteps) - 1)).pack(side="left")
        self.kStepVar = tk.StringVar(value="")
        ttk.Label(ctl, textvariable=self.kStepVar, width=16).pack(side="right")
        self.kSlider = tk.Scale(ctl, from_=0, to=1, orient="horizontal", showvalue=0, bg=BG,
                                highlightthickness=0, troughcolor="#cdd6e6", sliderlength=26,
                                command=self.onSlide)
        self.kSlider.pack(side="left", fill="x", expand=True, padx=12)
        split = ttk.Frame(self.ktView)
        split.pack(fill="both", expand=True)
        left = ttk.Frame(split)
        left.pack(side="left", fill="both", expand=True)
        ttk.Label(left, text="Calculation at this step", style="Head.TLabel").pack(anchor="w")
        self.ktText = tk.Text(left, wrap="word", font=(MONO_FONT, 12), bg="white", fg="#1a1a1a",
                              insertbackground="#1a1a1a", relief="flat", padx=12, pady=10)
        self.ktText.pack(fill="both", expand=True, pady=6)
        self.ktText.configure(state="disabled")
        right = ttk.Frame(split, width=520)
        right.pack(side="right", fill="y", padx=(10, 0))
        right.pack_propagate(False)
        book = ttk.Notebook(right)
        book.pack(fill="both", expand=True, pady=(4, 6))
        treeTab = ttk.Frame(book)
        levelTab = ttk.Frame(book)
        book.add(treeTab, text="Call tree")
        book.add(levelTab, text="All levels")
        self.callTree = ttk.Treeview(treeTab, columns=("size", "result"), show="tree headings")
        self.callTree.heading("#0", text="Call")
        self.callTree.heading("size", text="digits (x \u00d7 y)")
        self.callTree.heading("result", text="result")
        self.callTree.column("#0", width=190)
        self.callTree.column("size", width=110, anchor="center")
        self.callTree.column("result", width=170)
        self.callTree.tag_configure("pending", foreground="#8a93a5")
        self.callTree.tag_configure("done", foreground="#1b7f3b")
        self.callTree.tag_configure("current", background="#fff0b3", foreground="#000000")
        self.callTree.pack(fill="both", expand=True)
        cols = ("level", "calls", "base", "digits")
        self.tree = ttk.Treeview(levelTab, columns=cols, show="headings")
        for c, t, w in zip(cols, ("Level", "Calls (3^level)", "Base cases", "Largest operand (digits)"),
                           (60, 130, 110, 190)):
            self.tree.heading(c, text=t)
            self.tree.column(c, width=w, anchor="center")
        self.tree.pack(fill="both", expand=True)

    # ---- file handling
    def browse(self):
        start = Path(__file__).resolve().parent.parent / "datasets"
        name = filedialog.askopenfilename(initialdir=start if start.exists() else None,
                                          title="Select input file", filetypes=[("Text files", "*.txt")])
        if name:
            self.loadFile(name)
            self.run()

    def loadFile(self, name):
        kind = detectKind(name)
        if kind is None:
            messagebox.showerror("Unrecognised file",
                                 "Expected either a points file (first line n, then n lines 'x y') "
                                 "or an integer file (two integers).")
            return
        self.path, self.kind = Path(name), kind
        self.fileVar.set(self.path.name)

    def run(self):
        if not self.path:
            messagebox.showinfo("No file", "Please choose an input file first.")
            return
        self.cpView.pack_forget()
        self.ktView.pack_forget()
        self.update_idletasks()
        if self.kind == "points":
            self.runClosest()
        else:
            self.runKaratsuba()

    # ---- closest pair
    def runClosest(self):
        self.points = cp.readPoints(self.path)
        self.xa = np.array([p[0] for p in self.points])
        self.ya = np.array([p[1] for p in self.points])
        start = time.perf_counter()
        d, p, q = cp.closestPair(self.points)
        elapsed = time.perf_counter() - start
        d2, p2, q2, self.events = traceClosest(self.points, int(self.cpDepthVar.get()))
        assert math.isclose(d, d2)
        self.result = (d, p, q)
        n = len(self.points)
        check = "skipped (n > 5000)"
        if n <= 5000:
            check = "PASS" if math.isclose(d, cp.bruteForce(self.points)[0]) else "FAIL"
        self.resultVar.set(f"Closest pair: ({p[0]:.4f}, {p[1]:.4f}) and ({q[0]:.4f}, {q[1]:.4f})   |   "
                           f"distance = {d:.6f}   |   n = {n}   |   time = {elapsed:.4f} s   |   "
                           f"brute-force check: {check}")
        self.cpView.pack(fill="both", expand=True)
        self.goto(0)

    def changeDepth(self):
        if self.kind == "points" and self.points is not None:
            self.events = traceClosest(self.points, int(self.cpDepthVar.get()))[3]
            self.goto(0)

    def goto(self, i):
        self.step = max(0, min(i, len(self.events)))
        self.draw()

    def paint(self, target, scene, ms, mark=12):
        for lo, hi, colour, alpha in scene["spans"]:
            target.axvspan(lo, hi, color=colour, alpha=alpha, zorder=1)
        for v in scene["vlines"]:
            target.axvline(v, color=ACCENT, ls="--", lw=1.5, zorder=3)
        target.plot(self.xa, self.ya, "o", ms=ms, color="#3d4a63", mec="white", mew=0.5, alpha=0.9, zorder=2)
        if scene["strip"]:
            target.plot([q[0] for q in scene["strip"]], [q[1] for q in scene["strip"]], "o",
                        ms=ms + 4, color="#f59f00", mec="black", mew=0.8, zorder=4)
        if scene["pair"]:
            a, b = scene["pair"]
            target.plot([a[0], b[0]], [a[1], b[1]], "-o", color="#d62828", lw=2.5, ms=mark,
                        mfc="#d62828", mec="black", mew=1.2, zorder=5)

    def emptiestSpots(self, p, q):
        """Pick a free corner for the zoom inset and a free spot for the label (axes fractions)."""
        ax = self.ax
        xl, yl = ax.get_xlim(), ax.get_ylim()
        fx = (self.xa - xl[0]) / (xl[1] - xl[0])
        fy = (self.ya - yl[0]) / (yl[1] - yl[0])
        pc = ((p[0] + q[0]) / 2 - xl[0]) / (xl[1] - xl[0]), ((p[1] + q[1]) / 2 - yl[0]) / (yl[1] - yl[0])

        def inside(x0, y0, w, h):
            return int(np.count_nonzero((fx >= x0) & (fx <= x0 + w) & (fy >= y0) & (fy <= y0 + h)))

        size, m = 0.36, 0.02
        best = None
        for x0, y0 in [(m, m), (1 - size - m, m), (m, 1 - size - m), (1 - size - m, 1 - size - m)]:
            score = inside(x0, y0, size, size)
            if x0 - 0.06 <= pc[0] <= x0 + size + 0.06 and y0 - 0.06 <= pc[1] <= y0 + size + 0.06:
                score += 10 ** 6
            if best is None or score < best[0]:
                best = (score, (x0, y0, size, size))
        rect = best[1]
        bw, bh = 0.26, 0.14
        bestLabel = None
        for dx, dy in [(-0.22, 0.16), (0.22, 0.16), (-0.22, -0.16), (0.22, -0.16),
                       (0, 0.22), (0, -0.22), (-0.3, 0), (0.3, 0)]:
            cx = min(max(pc[0] + dx, bw / 2 + 0.01), 1 - bw / 2 - 0.01)
            cy = min(max(pc[1] + dy, bh / 2 + 0.01), 1 - bh / 2 - 0.01)
            score = inside(cx - bw / 2, cy - bh / 2, bw, bh)
            if abs(cx - pc[0]) < bw / 2 + 0.03 and abs(cy - pc[1]) < bh / 2 + 0.03:
                score += 10 ** 6  # would cover the pair
            if (cx - bw / 2 < rect[0] + rect[2] and cx + bw / 2 > rect[0] and
                    cy - bh / 2 < rect[1] + rect[3] and cy + bh / 2 > rect[1]):
                score += 10 ** 7  # would overlap the inset
            if bestLabel is None or score < bestLabel[0]:
                bestLabel = (score, (cx, cy))
        return rect, bestLabel[1]

    def draw(self):
        if self.kind != "points" or self.points is None:
            return
        ax = self.ax
        if self.lens is not None:
            self.lens.remove()
            self.lens = None
        ax.clear()
        n = len(self.xa)
        ms = 8 if n < 500 else 6 if n < 2000 else 4 if n < 6000 else 2.5
        scene = dict(spans=[], vlines=[], strip=[], pair=None, ms=ms)
        text = ""
        total = len(self.events)
        xlim = None
        if self.step < total:
            ev = self.events[self.step]
            scene["spans"].append((ev["lo"], ev["hi"], "#cfe0fb", 0.5))
            scene["vlines"].append(ev["midX"])
            if ev["kind"] == "divide":
                text = (f"DIVIDE  (depth {ev['depth']})\n\n"
                        f"This subproblem has {ev['n']} points (blue band).\n\n"
                        f"It is split at the median x = {ev['midX']:.2f} (dashed line) into "
                        f"{ev['n'] // 2} points on the left and {ev['n'] - ev['n'] // 2} on the right.\n\n"
                        f"Each half is solved recursively before the results are combined.")
            else:
                d0, d1 = ev["dHalves"], ev["dFinal"]
                scene["spans"].append((ev["midX"] - d0, ev["midX"] + d0, "#ffe2b8", 0.7))
                scene["strip"] = ev["strip"]
                scene["pair"] = ev["pair"]
                k = len(ev["strip"])
                text = (f"COMBINE  (depth {ev['depth']})\n\n"
                        f"Best distance found inside the two halves: d = {d0:.4f}.\n\n"
                        f"Only points within d of the dividing line can form a closer crossing pair. "
                        f"The orange strip holds {k} point{'' if k == 1 else 's'}; each is compared only "
                        f"with the next points whose y differs by less than d.\n\n")
                text += (f"A closer crossing pair was found: d = {d1:.4f}." if d1 < d0
                         else "No crossing pair is closer, so d stays the same.")
                text += f"\n\nBest pair so far in this subproblem is drawn in red (distance {d1:.4f})."
            if self.zoomVar.get():
                pad = (ev["hi"] - ev["lo"]) * 0.05 + 1e-9
                xlim = (ev["lo"] - pad, ev["hi"] + pad)
            self.stepVar.set(f"Step {self.step + 1} of {total + 1}")
            text += (f"\n\n(The top {self.cpDepthVar.get()} recursion levels are shown. Pieces of 3 or fewer "
                     f"points are solved directly; any deeper levels are computed the same way but not drawn.)")
        else:
            d, p, q = self.result
            scene["pair"] = (p, q)
            text = (f"RESULT\n\nThe closest pair over all {len(self.points)} points is\n"
                    f"  ({p[0]:.4f}, {p[1]:.4f})\n  ({q[0]:.4f}, {q[1]:.4f})\n\n"
                    f"distance = {d:.6f}\n\n"
                    f"Recurrence: T(n) = 2T(n/2) + O(n), so the running time is O(n log n).")
            self.stepVar.set(f"Step {total + 1} of {total + 1}")
        self.scene = scene
        self.paint(ax, scene, ms)
        if xlim:
            ax.set_xlim(*xlim)
        if self.step >= total:
            d, p, q = self.result
            rect, (lx, ly) = self.emptiestSpots(p, q)
            cx, cy = (p[0] + q[0]) / 2, (p[1] + q[1]) / 2
            ax.annotate(f"closest pair\nd = {d:.4f}", xy=(cx, cy), xytext=(lx, ly),
                        textcoords="axes fraction", ha="center", va="center",
                        fontsize=12, fontweight="bold", color="#d62828",
                        bbox=dict(boxstyle="round,pad=0.4", fc="white", ec="#d62828"),
                        arrowprops=dict(arrowstyle="->", color="#d62828", lw=2), zorder=6)
            r = max(d * 4, 1e-6)
            axin = ax.inset_axes(list(rect))
            self.paint(axin, dict(spans=[], vlines=[], strip=[], pair=(p, q)), 9)
            axin.set_xlim(cx - r, cx + r)
            axin.set_ylim(cy - r, cy + r)
            axin.set_title("zoom on closest pair", fontsize=11)
            axin.tick_params(labelsize=9)
            ax.indicate_inset_zoom(axin, edgecolor="#d62828")
        ax.legend(handles=[
            Line2D([], [], marker="o", ls="", color="#3d4a63", label="point"),
            Line2D([], [], marker="s", ls="", color="#cfe0fb", ms=12, label="current subproblem"),
            Line2D([], [], ls="--", color=ACCENT, label="split line"),
            Line2D([], [], marker="o", ls="", color="#f59f00", mec="black", label="strip point"),
            Line2D([], [], marker="o", ls="-", color="#d62828", mec="black", label="closest pair so far")],
            loc="upper center", bbox_to_anchor=(0.5, -0.12), ncol=5, fontsize=10, frameon=False)
        ax.set_title(f"{self.path.name}   ({len(self.points)} points)")
        ax.set_xlabel("x")
        ax.set_ylabel("y")
        self.fig.tight_layout()
        self.canvas.draw_idle()
        self.cpText.configure(state="normal")
        self.cpText.delete("1.0", "end")
        self.cpText.insert("1.0", text)
        self.cpText.configure(state="disabled")

    # ---- magnifier (like the Windows lens)
    def hideLens(self):
        if self.lens is not None:
            self.lens.remove()
            self.lens = None
            self.canvas.draw_idle()

    def makeLens(self):
        self.lens = self.fig.add_axes([0, 0, 0.2, 0.2], zorder=20)
        self.paint(self.lens, self.scene, self.scene["ms"] + 3, mark=15)
        self.lens.set_xticks([])
        self.lens.set_yticks([])
        for sp in self.lens.spines.values():
            sp.set_edgecolor("#d62828")
            sp.set_linewidth(3)
        self.lensLabel = self.lens.text(0.04, 0.94, "", transform=self.lens.transAxes, fontsize=12,
                                        fontweight="bold", color="#d62828", va="top",
                                        bbox=dict(fc="white", ec="none", pad=1.5))

    def moveLens(self, ev):
        fw, fh = self.fig.bbox.width, self.fig.bbox.height
        w, h = LENS_PX / fw, LENS_PX / fh
        x0 = min(max(ev.x / fw - w / 2, 0), 1 - w)
        y0 = min(max(ev.y / fh - h / 2, 0), 1 - h)
        self.lens.set_position([x0, y0, w, h])
        bb = self.ax.get_window_extent()
        xl, yl = self.ax.get_xlim(), self.ax.get_ylim()
        sx = (xl[1] - xl[0]) * LENS_PX / bb.width / self.magZoom
        sy = (yl[1] - yl[0]) * LENS_PX / bb.height / self.magZoom
        self.lens.set_xlim(ev.xdata - sx / 2, ev.xdata + sx / 2)
        self.lens.set_ylim(ev.ydata - sy / 2, ev.ydata + sy / 2)
        self.lensLabel.set_text(f"{self.magZoom:.0f}x")
        self.canvas.draw_idle()

    def onMove(self, ev):
        if self.kind != "points" or self.scene is None or not self.magVar.get():
            return
        if ev.inaxes is not self.ax or ev.xdata is None:
            self.hideLens()
            return
        now = time.perf_counter()
        if now - self.lastMove < 0.03:
            return
        self.lastMove = now
        if self.lens is None:
            self.makeLens()
        self.moveLens(ev)

    def onScroll(self, ev):
        if self.kind != "points" or not self.magVar.get() or ev.inaxes is not self.ax:
            return
        self.magZoom = min(80.0, max(2.0, self.magZoom * (1.3 if ev.step > 0 else 1 / 1.3)))
        if self.lens is None:
            self.makeLens()
        self.moveLens(ev)

    # ---- karatsuba
    def runKaratsuba(self):
        self.kStop()
        x, y = kt.readNumbers(self.path)
        cutoff = int(self.cutoffVar.get())
        kt.cutoffDigits = cutoff
        product, elapsed = kt.bestTime(kt.karatsuba, x, y)
        base = kt.baseCalls
        traced, levels = traceKaratsuba(x, y, cutoff)
        ok = product == x * y == traced
        self.kProduct = product
        self.kNodes, self.kSteps = buildKaratsubaTrace(x, y, cutoff, int(self.depthVar.get()))
        for row in self.tree.get_children():
            self.tree.delete(row)
        for lvl in sorted(levels):
            calls, bases, digits = levels[lvl]
            self.tree.insert("", "end", values=(lvl, calls, bases, digits))
        self.callTree.delete(*self.callTree.get_children())
        for nd in self.kNodes:
            self.callTree.insert(str(nd["parent"]) if nd["parent"] is not None else "", "end",
                                 iid=str(nd["id"]), text=nd["label"], open=True,
                                 values=(f"{len(str(nd['x']))} \u00d7 {len(str(nd['y']))}", "pending"),
                                 tags=("pending",))
        self.resultVar.set(f"Product has {len(str(product))} digits ({short(product, 8)})   |   "
                           f"base multiplications = {base}   |   time = {elapsed:.4f} s   |   "
                           f"check against built-in multiplication: {'PASS' if ok else 'FAIL'}")
        self.ktView.pack(fill="both", expand=True)
        self.kIdx = 0
        self.kSlider.configure(to=max(1, len(self.kSteps) - 1))
        self.kSlider.set(0)
        self.kDraw()

    def onSlide(self, value):
        i = int(float(value))
        if self.kSteps and i != self.kIdx:
            self.kGoto(i)

    def kGoto(self, i):
        if not self.kSteps:
            return
        self.kIdx = max(0, min(i, len(self.kSteps) - 1))
        self.kSlider.set(self.kIdx)
        self.kDraw()

    def kTogglePlay(self):
        if self.kPlaying:
            self.kStop()
        else:
            if self.kIdx >= len(self.kSteps) - 1:
                self.kGoto(0)
            self.kPlaying = True
            self.playBtn.configure(text="Pause")
            self.kTick()

    def kStop(self):
        self.kPlaying = False
        self.playBtn.configure(text="Play")

    def kTick(self):
        if not self.kPlaying:
            return
        if self.kIdx >= len(self.kSteps) - 1:
            self.kStop()
            return
        self.kGoto(self.kIdx + 1)
        self.after(800, self.kTick)

    def num(self, v):
        s = str(v)
        if self.fullVar.get() or len(s) <= 30:
            return s
        return f"{s[:12]}...{s[-12:]}  ({len(s)} digits)"

    def kPath(self, nd):
        names = []
        while nd is not None:
            names.append(nd["label"])
            nd = self.kNodes[nd["parent"]] if nd["parent"] is not None else None
        return "  ->  ".join(reversed(names))

    def kDraw(self):
        if not self.kSteps or self.kind != "mult":
            return
        kind, nid = self.kSteps[self.kIdx]
        nd = self.kNodes[nid]
        f = self.num
        head = f"Call: {self.kPath(nd)}   (depth {nd['depth']})\n" + "-" * 60 + "\n"
        if kind == "split":
            h = nd["half"]
            body = (f"x = {f(nd['x'])}\ny = {f(nd['y'])}\n\n"
                    f"Longest operand has {max(len(str(nd['x'])), len(str(nd['y'])))} digits"
                    f"{' (padded to ' + str(nd['n']) + ' so both halves are equal)' if nd['pad'] else ''}, "
                    f"so split at half = {h} digits:\n\n"
                    f"  x = a * 10^{h} + b\n  y = c * 10^{h} + d\n\n"
                    f"  a = {f(nd['a'])}\n  b = {f(nd['b'])}\n  c = {f(nd['c'])}\n  d = {f(nd['d'])}\n\n"
                    f"Karatsuba needs only three multiplications (not four):\n"
                    f"  1) ac        = a * c\n  2) bd        = b * d\n"
                    f"  3) (a+b)(c+d) = {f(nd['a'] + nd['b'])} * {f(nd['c'] + nd['d'])}\n\n"
                    f"Press Next to solve them one by one.")
        elif kind == "combine":
            h = nd["half"]
            body = (f"The three sub-products are ready:\n"
                    f"  ac         = {f(nd['ac'])}\n  bd         = {f(nd['bd'])}\n"
                    f"  (a+b)(c+d) = {f(nd['mid'])}\n\n"
                    f"Cross term, without a fourth multiplication:\n"
                    f"  ad + bc = (a+b)(c+d) - ac - bd\n          = {f(nd['cross'])}\n\n"
                    f"Combine:\n  x*y = ac * 10^{2 * h} + (ad+bc) * 10^{h} + bd\n"
                    f"      = {f(nd['result'])}\n\n"
                    f"Check against x * y: {'MATCH' if nd['result'] == nd['x'] * nd['y'] else 'MISMATCH'}")
        elif kind == "base":
            body = (f"x = {f(nd['x'])}\ny = {f(nd['y'])}\n\n"
                    f"The smaller number has at most {kt.cutoffDigits} digits (base case), "
                    f"so it is multiplied directly:\n\n  x * y = {f(nd['result'])}")
        else:
            body = (f"x = {f(nd['x'])}\ny = {f(nd['y'])}\n\n"
                    f"This call and everything below it follow the same split/combine steps, "
                    f"but only the top {self.depthVar.get()} levels are shown.\n\n"
                    f"Result: x * y = {f(nd['result'])}")
        self.ktText.configure(state="normal")
        self.ktText.delete("1.0", "end")
        self.ktText.insert("1.0", head + body)
        self.ktText.configure(state="disabled")
        self.kStepVar.set(f"Step {self.kIdx + 1} of {len(self.kSteps)}")
        done = set()
        for k, i in self.kSteps[:self.kIdx + 1]:
            if k != "split":
                done.add(i)
        for node in self.kNodes:
            i = node["id"]
            if i == nid:
                tag = "current"
            elif i in done:
                tag = "done"
            else:
                tag = "pending"
            res = short(node["result"], 6) if i in done and "result" in node else "pending"
            self.callTree.item(str(i), tags=(tag,))
            self.callTree.set(str(i), "result", res)
        self.callTree.selection_set(str(nid))
        self.callTree.see(str(nid))


if __name__ == "__main__":
    App().mainloop()
