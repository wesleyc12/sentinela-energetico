# capture_serial.py
# Captures the Sentinela CSV log from the Arduino serial port and saves
# it to a file. Keys while running: A = label 1 (real anomaly),
# N = label 0 (normal), Q = quit.
# Close the Arduino IDE Serial Monitor first: only one program can use
# the port at a time. Windows only (uses msvcrt for key presses).

import csv
import msvcrt
import os
import time
from datetime import datetime

import serial
from serial.tools import list_ports

PORT = "COM5"
BAUD = 9600
HEADER = ["ms", "raw", "mean", "stddev", "fixed_flag", "adaptive_flag", "state", "label"]
DATA_DIR = os.path.join(os.path.dirname(__file__), "..", "data")


def open_port():
    try:
        return serial.Serial(PORT, BAUD, timeout=0.1)
    except serial.SerialException as error:
        print(f"Nao consegui abrir {PORT}: {error}")
        print("Portas encontradas:", [p.device for p in list_ports.comports()])
        print("Feche o Serial Monitor do Arduino IDE e confira o valor de PORT.")
        raise SystemExit(1)


def parse_data_line(line):
    # A valid data line has 8 comma-separated fields starting with a number.
    # Anything else (comments with '#', the header, boot noise) is skipped.
    parts = line.strip().split(",")
    if len(parts) == len(HEADER) and parts[0].isdigit():
        return parts
    return None


def main():
    description = input("Descricao da sessao (ex: ar_ligado, sem_ar): ")
    description = description.strip().replace(" ", "_")

    os.makedirs(DATA_DIR, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = os.path.join(DATA_DIR, f"sessao_{stamp}_{description}.csv")

    ser = open_port()
    print("Aguardando a placa reiniciar...")
    time.sleep(2)             # opening the port resets the Arduino
    ser.reset_input_buffer()  # drop boot messages
    ser.write(b"L")           # turn CSV logging on

    rows = 0
    print(f"Gravando em {filename}")
    print("Teclas: A = anomalia real | N = normal | Q = sair")

    with open(filename, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(HEADER)
        try:
            while True:
                if msvcrt.kbhit():
                    key = msvcrt.getwch().lower()
                    if key == "a":
                        ser.write(b"A")
                    elif key == "n":
                        ser.write(b"N")
                    elif key == "q":
                        break

                raw_line = ser.readline().decode("utf-8", errors="ignore")
                fields = parse_data_line(raw_line)
                if fields:
                    writer.writerow(fields)
                    f.flush()
                    rows += 1
                    print(f"\rlinhas: {rows} | leitura: {fields[1]} | rotulo: {fields[7]}   ",
                          end="", flush=True)
        finally:
            ser.write(b"L")   # turn logging off
            time.sleep(0.3)
            ser.close()

    print(f"\nPronto: {rows} linhas salvas em {filename}")


main()