# depth_sweep.py
# Compares decision trees of different depths (relative features) using the
# same leave-one-session-out test, to pick the simplest tree that works.

import pandas as pd
from sklearn.tree import DecisionTreeClassifier

from load_data import load_sessions
from train_eval import RELATIVE_FEATURES, add_features, metrics


def leave_one_session_out(data, features, depth):
    preds = pd.Series(0, index=data.index)
    for held_out in data["session"].unique():
        train = data[data["session"] != held_out]
        test = data[data["session"] == held_out]
        tree = DecisionTreeClassifier(max_depth=depth, min_samples_leaf=20,
                                      class_weight="balanced", random_state=0)
        tree.fit(train[features], train["label"])
        preds.loc[test.index] = tree.predict(test[features])
    return preds


def main():
    data = add_features(load_sessions())
    print("\nDEPTH SWEEP (relative features, leave one session out)")
    print(f"{'depth':>6s}{'recall':>9s}{'false alarm':>13s}{'balanced acc':>14s}")
    for depth in [1, 2, 3, 4]:
        pred = leave_one_session_out(data, RELATIVE_FEATURES, depth)
        r, f, b = metrics(data["label"], pred)
        print(f"{depth:6d}{r:9.2f}{f:13.2f}{b:14.2f}")


if __name__ == "__main__":
    main()