# train_eval.py
# Trains shallow decision trees on the labeled sessions and compares them,
# on sessions they never saw, with the detectors that ran on the Arduino
# (fixed threshold, adaptive threshold and the FSM state).
#
# Evaluation is "leave one session out": train on 3 sessions, test on the
# 4th, and repeat for each session. This checks whether the model still
# works in a room condition (baseline level) it was not trained on.

import numpy as np
import pandas as pd
from sklearn.tree import DecisionTreeClassifier, export_text

from load_data import load_sessions

WINDOW = 20        # same window size as the Arduino firmware
MAX_DEPTH = 2      # shallow tree: becomes a few if/else lines in C
TAIL_MARGIN = 4    # counts above the pre-event rest that still count as anomaly

ABSOLUTE_FEATURES = ["raw", "mean", "stddev"]
RELATIVE_FEATURES = ["stddev", "delta", "slope"]


def trim_late_tails(g):
    # One rule for every session: a label=1 block ends when the reading is
    # back within TAIL_MARGIN counts of the rest level measured just before
    # the block started. Blocks are only shortened, never extended.
    label = g["label"].to_numpy().copy()
    raw = g["raw"].to_numpy()
    n = len(label)
    i = 0
    while i < n:
        if label[i] == 1 and (i == 0 or label[i - 1] == 0):
            j = i
            while j + 1 < n and label[j + 1] == 1:
                j += 1
            before = raw[max(0, i - WINDOW):i]
            rest = np.median(before) if len(before) else raw[i]
            above = np.where(raw[i:j + 1] > rest + TAIL_MARGIN)[0]
            if len(above) > 0:
                label[i + above[-1] + 1:j + 1] = 0
            i = j + 1
        else:
            i += 1
    g = g.copy()
    g["label"] = label
    return g


def add_features(df):
    # delta: how far the newest reading is from the window mean.
    # slope: newest reading minus the oldest one in the window (~4 s ago).
    out = []
    for name, g in df.groupby("session", sort=False):
        g = g.reset_index(drop=True)
        before = int(g["label"].sum())
        g = trim_late_tails(g)
        trimmed = before - int(g["label"].sum())
        print(f"{name}: {trimmed} late-tail rows relabeled to normal")
        g["delta"] = g["raw"] - g["mean"]
        g["slope"] = g["raw"] - g["raw"].shift(WINDOW - 1)
        out.append(g.iloc[WINDOW:])   # drop rows where the window was not full
    return pd.concat(out, ignore_index=True)


def metrics(y_true, y_pred):
    y_true = np.asarray(y_true).astype(int)
    y_pred = np.asarray(y_pred).astype(int)
    tp = ((y_true == 1) & (y_pred == 1)).sum()
    fn = ((y_true == 1) & (y_pred == 0)).sum()
    fp = ((y_true == 0) & (y_pred == 1)).sum()
    tn = ((y_true == 0) & (y_pred == 0)).sum()
    recall = tp / (tp + fn) if (tp + fn) else float("nan")
    false_alarm = fp / (fp + tn) if (fp + tn) else float("nan")
    balanced = (recall + (1 - false_alarm)) / 2
    return recall, false_alarm, balanced


def make_tree():
    return DecisionTreeClassifier(max_depth=MAX_DEPTH, min_samples_leaf=20,
                                  class_weight="balanced", random_state=0)


def leave_one_session_out(data, features):
    preds = pd.Series(0, index=data.index)
    for held_out in data["session"].unique():
        train = data[data["session"] != held_out]
        test = data[data["session"] == held_out]
        tree = make_tree().fit(train[features], train["label"])
        preds.loc[test.index] = tree.predict(test[features])
    return preds


def main():
    data = add_features(load_sessions())
    rel_tree = leave_one_session_out(data, RELATIVE_FEATURES)

    methods = {
        "fixed threshold (590)": data["fixed_flag"],
        "adaptive (3 sigma)": data["adaptive_flag"],
        "FSM state (LED/buzzer)": data["state"],
        "tree: absolute features": leave_one_session_out(data, ABSOLUTE_FEATURES),
        "tree: relative features": rel_tree,
        "rule: stddev > 1.6": (data["stddev"] > 1.6).astype(int),
        "rel. tree OR fixed 590": ((rel_tree == 1) | (data["fixed_flag"] == 1)).astype(int),
    }

    print(f"\n{len(data)} rows used (first {WINDOW} rows of each session dropped), "
          f"{int(data['label'].sum())} anomaly rows\n")

    print("ALL SESSIONS TOGETHER")
    print(f"{'method':24s}{'recall':>9s}{'false alarm':>13s}{'balanced acc':>14s}")
    for name, pred in methods.items():
        r, f, b = metrics(data["label"], pred)
        print(f"{name:24s}{r:9.2f}{f:13.2f}{b:14.2f}")

    print("\nBALANCED ACCURACY PER SESSION")
    sessions = list(data["session"].unique())
    print(f"{'method':24s}" + "".join(f"{s:>19s}" for s in sessions))
    for name, pred in methods.items():
        row = f"{name:24s}"
        for s in sessions:
            mask = data["session"] == s
            _, _, b = metrics(data.loc[mask, "label"], pred[mask])
            row += f"{b:19.2f}"
        print(row)

    print("\nTREE RULES (trained on all four sessions)")
    for title, feats in [("absolute", ABSOLUTE_FEATURES),
                         ("relative", RELATIVE_FEATURES)]:
        tree = make_tree().fit(data[feats], data["label"])
        print(f"\n--- {title} features ---")
        print(export_text(tree, feature_names=feats))


if __name__ == "__main__":
    main()