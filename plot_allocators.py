import sys

import matplotlib.pyplot as plt
import pandas as pd

if len(sys.argv) < 2:
    print("Usage: python plot_allocators.py results.csv")
    sys.exit(1)

csv_file = sys.argv[1]
df = pd.read_csv(csv_file)
df["n"] = df["n"].astype(int)


def plot_metric(metric, title, ylabel, filename):
    for t in df["type"].unique():
        subset = df[df["type"] == t]

        plt.figure(figsize=(8, 5))
        for alloc in subset["allocator"].unique():
            data = subset[subset["allocator"] == alloc]
            plt.plot(
                data["n"],
                data[metric],
                marker="o",
                label=alloc,
            )

        plt.title(f"{title} ({t})")
        plt.xlabel("Number of elements (n)")
        plt.ylabel(ylabel)
        plt.yscale("log", base=1.5)
        plt.legend()
        plt.grid(True)
        plt.tight_layout()
        plt.savefig(f"{filename}_{t}.png")
        plt.close()


# ---- GENERATE PLOTS ---------------------------------------------------

plot_metric(
    metric="alloc_ms_avg",
    title="Allocation Time",
    ylabel="Time (ms)",
    filename="alloc_time",
)

plot_metric(
    metric="free_ms_avg",
    title="Free Time",
    ylabel="Time (ms)",
    filename="free_time",
)

plot_metric(
    metric="ram_kb_avg",
    title="Memory Usage",
    ylabel="RAM (KB)",
    filename="ram_usage",
)

print("Plots generated:")
print(" - alloc_time_<type>.png")
print(" - free_time_<type>.png")
print(" - ram_usage_<type>.png")
