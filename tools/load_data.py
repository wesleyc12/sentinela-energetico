# load_data.py
# Loads the four labeled CSV sessions from /data into one table.
# Other scripts import load_sessions() from here.

import glob
import os
import re

import pandas as pd

DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "data")


def session_name(path):
    # File names look like: sessao_20261024_153000_dia_ar_ligado.csv
    # The session name is everything after the date and time.
    base = os.path.basename(path)
    match = re.match(r"sessao_\d{8}_\d{6}_(.+)\.csv", base)
    return match.group(1) if match else base


def load_sessions():
    files = sorted(glob.glob(os.path.join(DATA_DIR, "sessao_*.csv")))
    frames = []
    for path in files:
        df = pd.read_csv(path)
        df["session"] = session_name(path)
        frames.append(df)
    return pd.concat(frames, ignore_index=True)


if __name__ == "__main__":
    data = load_sessions()
    print(f"Total rows: {len(data)}\n")
    for name, group in data.groupby("session"):
        rest = group[group["label"] == 0]["raw"].median()
        peak = group["raw"].max()
        print(f"{name}: {len(group)} rows, "
              f"{int(group['label'].sum())} anomaly rows, "
              f"rest median {rest:.0f}, max {peak}")