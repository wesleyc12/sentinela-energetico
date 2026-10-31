# plot_results.py
# Draws one panel per session: the raw sensor reading, the real anomaly
# periods (grey) and what each detector flagged (colored bars below).
# The tree predictions come from the leave-one-session-out test, so each
# panel shows the tree working on a session it never saw during training.

import os

import matplotlib
matplotlib.use("Agg")          # draw to a file, no window needed
import matplotlib.pyplot as plt
from matplotlib.patches import Patch

from load_data import load_sessions
from train_eval import RELATIVE_FEATURES, add_features, leave_one_session_out

OUT_PATH = os.path.join(os.path.dirname(__file__), "..", "docs", "comparison.png")

DETECTORS = [
    ("Limiar fixo (590)", "fixed_flag", "tab:red"),
    ("Adaptativo", "adaptive_flag", "tab:orange"),
    ("Árvore aprendida", "tree", "tab:green"),
]


def main():
    data = add_features(load_sessions())
    data["tree"] = leave_one_session_out(data, RELATIVE_FEATURES)

    sessions = list(data["session"].unique())
    fig, axes = plt.subplots(len(sessions), 1, figsize=(12, 3.2 * len(sessions)))

    for ax, name in zip(axes, sessions):
        g = data[data["session"] == name]
        t = (g["ms"] - g["ms"].iloc[0]) / 1000.0
        raw = g["raw"]

        bar = (raw.max() - raw.min()) * 0.07
        base = raw.min() - 3.6 * bar
        top = raw.max() + bar

        # Real anomaly periods (the labels you set with A and N).
        ax.fill_between(t, base, top, where=(g["label"] == 1).to_numpy(),
                        color="grey", alpha=0.25, step="mid", linewidth=0)
        ax.plot(t, raw, color="black", linewidth=1)

        # One colored bar per detector, drawn below the signal.
        for k, (_, column, color) in enumerate(DETECTORS):
            y0 = base + 0.2 * bar + k * 1.1 * bar
            ax.fill_between(t, y0, y0 + bar, where=(g[column] == 1).to_numpy(),
                            color=color, step="mid", linewidth=0)

        rest = g.loc[g["label"] == 0, "raw"].median()
        ax.set_ylim(base, top)
        ax.set_xlim(t.iloc[0], t.iloc[-1])
        ax.set_title(f"{name} (repouso ~{rest:.0f})", loc="left", fontsize=10)
        ax.set_ylabel("leitura")
    axes[-1].set_xlabel("tempo (s)")

    legend = [Patch(color="grey", alpha=0.25, label="Anomalia real (rótulo)")]
    legend += [Patch(color=c, label=n) for n, _, c in DETECTORS]
    fig.legend(handles=legend, loc="upper center", ncol=4, frameon=False)
    fig.tight_layout(rect=(0, 0, 1, 0.96))

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
    fig.savefig(OUT_PATH, dpi=150)
    print(f"Saved {os.path.abspath(OUT_PATH)}")


if __name__ == "__main__":
    main()