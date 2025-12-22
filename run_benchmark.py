#!/usr/bin/env python3

import csv
import os
import statistics
import subprocess
from collections import defaultdict
from concurrent.futures import ProcessPoolExecutor, as_completed

# ---------------- CONFIG ----------------

BINARY = "./build/benchmark"

ALLOCATORS = [
    "Standard",
    "Simple",
    "Pool",
    "Linear",
    "Segregated",
]

TYPES = ["i8", "i16", "i32", "i64", "i128", "mixed"]

SIZES = [1_000, 5_000, 10_000, 50_000, 100_000]

REPEATS = 50

OUTPUT_CSV = "results_avg.csv"

MAX_WORKERS = os.cpu_count()  # safe default

# ----------------------------------------


def run_once(args):
    allocator, typ, n = args

    result = subprocess.run(
        [BINARY, allocator, typ, str(n)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=True,
    )

    lines = result.stdout.strip().splitlines()
    for line in lines:
        if line.startswith("allocator"):
            continue
        fields = line.split(",")
        return (
            allocator,
            typ,
            n,
            float(fields[3]),  # alloc_ms
            float(fields[4]),  # free_ms
            int(fields[5]),  # ram_kb
        )

    raise RuntimeError("No data row found")


def main():
    results = defaultdict(list)

    jobs = []
    for allocator in ALLOCATORS:
        for typ in TYPES:
            for n in SIZES:
                for _ in range(REPEATS):
                    jobs.append((allocator, typ, n))

    print(f"Running {len(jobs)} benchmarks using {MAX_WORKERS} workers...\n")

    with ProcessPoolExecutor(max_workers=MAX_WORKERS) as executor:
        futures = [executor.submit(run_once, job) for job in jobs]

        for future in as_completed(futures):
            allocator, typ, n, alloc_ms, free_ms, ram_kb = future.result()
            results[(allocator, typ, n)].append(
                {
                    "alloc_ms": alloc_ms,
                    "free_ms": free_ms,
                    "ram_kb": ram_kb,
                }
            )

    print("\nWriting CSV:", OUTPUT_CSV)

    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            [
                "allocator",
                "type",
                "n",
                "alloc_ms_avg",
                "free_ms_avg",
                "ram_kb_avg",
                "runs",
            ]
        )

        for (allocator, typ, n), runs in sorted(results.items()):
            writer.writerow(
                [
                    allocator,
                    typ,
                    n,
                    statistics.mean(r["alloc_ms"] for r in runs),
                    statistics.mean(r["free_ms"] for r in runs),
                    statistics.mean(r["ram_kb"] for r in runs),
                    len(runs),
                ]
            )

    print("Done.")


if __name__ == "__main__":
    main()
