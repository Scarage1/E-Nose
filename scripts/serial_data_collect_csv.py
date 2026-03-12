#!/usr/bin/env python3
"""
serial_data_collect_csv.py — E-Nose Serial Data Collection

Listens on a serial port for CSV-formatted sensor data from the Wio Terminal
data collection firmware and saves each sample as a labelled CSV file.

Protocol
--------
The Arduino sketch prints:
  1. A header row:  timestamp,temp,humd,pres,co2,voc1,voc2,no2,eth,co
  2. N data rows, each ending with \\r\\n
  3. An empty line (\\r\\n\\r\\n) to signal end-of-sample

Usage
-----
    python serial_data_collect_csv.py -p <PORT> -b 115200 -d datasets/raw -l coffee

    PORT examples:
      macOS / Linux : /dev/cu.usbmodem1101  or  /dev/ttyACM0
      Windows       : COM9

Dependencies
------------
    pip install pyserial

Author:  Shivam Kumar (Scarage1) — adapted from ShawnHymel / Edge Impulse
License: Apache-2.0 (https://www.apache.org/licenses/LICENSE-2.0)
"""

import argparse
import os
import sys
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("ERROR: pyserial not installed. Run:  pip install pyserial")
    sys.exit(1)

# ─── Defaults ─────────────────────────────────────────────────────────────────
DEFAULT_BAUD  = 115200
DEFAULT_LABEL = "_unknown"


def write_csv(data: str, directory: str, label: str) -> None:
    """Write *data* to a uniquely named CSV file inside *directory*.

    Filename format:  <label>.<epoch_ms>.csv
    """
    exists = True
    while exists:
        uid      = str(round(time.time() * 1000))
        filename = f"{label}.{uid}.csv"
        out_path = os.path.join(directory, filename)
        if not os.path.exists(out_path):
            exists = False
            try:
                with open(out_path, "w", newline="") as f:
                    f.write(data)
                print(f"  ✓ Saved: {out_path}")
            except IOError as exc:
                print(f"  ✗ Write error: {exc}")


def list_ports() -> None:
    """Print available serial ports to stdout."""
    print("\nAvailable serial ports:")
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("  (none found)")
    for port, desc, hwid in sorted(ports):
        print(f"  {port:20s}  {desc}  [{hwid}]")
    print()


def main() -> None:
    # ── Argument parser ────────────────────────────────────────────────────────
    parser = argparse.ArgumentParser(
        description="Collect CSV sensor data from Wio Terminal over serial.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "-p", "--port",
        required=True,
        help="Serial port (e.g. /dev/cu.usbmodem1101 or COM9)",
    )
    parser.add_argument(
        "-b", "--baud",
        type=int,
        default=DEFAULT_BAUD,
        help="Baud rate",
    )
    parser.add_argument(
        "-d", "--directory",
        default=".",
        help="Output directory for CSV files",
    )
    parser.add_argument(
        "-l", "--label",
        default=DEFAULT_LABEL,
        help="Odor label prepended to file names",
    )

    list_ports()
    args = parser.parse_args()

    # ── Validate / create output directory ────────────────────────────────────
    try:
        os.makedirs(args.directory, exist_ok=True)
    except OSError as exc:
        print(f"ERROR: Cannot create directory '{args.directory}': {exc}")
        sys.exit(1)

    # ── Open serial port ───────────────────────────────────────────────────────
    ser = serial.Serial()
    ser.port     = args.port
    ser.baudrate = args.baud
    ser.timeout  = 0.1

    try:
        ser.open()
    except serial.SerialException as exc:
        print(f"ERROR: Cannot open port '{args.port}': {exc}")
        sys.exit(1)

    print(f"Connected to {args.port} @ {args.baud} baud")
    print(f"Saving to   : {os.path.abspath(args.directory)}/")
    print(f"Label       : {args.label}")
    print("Press Ctrl+C to stop.\n")

    # ── Receive loop ───────────────────────────────────────────────────────────
    rx_buf = b""

    try:
        while True:
            if ser.in_waiting > 0:
                while ser.in_waiting:
                    rx_buf += ser.read()

                    # End-of-sample marker: blank line (\r\n\r\n)
                    if rx_buf[-4:] == b"\r\n\r\n":
                        # Normalise line endings
                        sample_str = rx_buf.decode("utf-8", errors="replace").strip()
                        sample_str = sample_str.replace("\r\n", "\n").replace("\r", "\n")

                        if sample_str:
                            print(f"Sample received ({len(sample_str.splitlines())} lines):")
                            print(f"  {sample_str.splitlines()[0]}")  # header preview
                            write_csv(sample_str, args.directory, args.label)

                        rx_buf = b""

    except KeyboardInterrupt:
        print("\nInterrupted by user.")

    finally:
        ser.close()
        print("Serial port closed.")


if __name__ == "__main__":
    main()
