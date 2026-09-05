#!/usr/bin/env python3
"""
serial_send.py

Send a text file to a COM/serial port line by line, with a 100 ms delay
between each line, while echoing any data received back from the port
to the console.

Library used: pyserial (import serial)
Install with:  pip install pyserial

Usage:
    python serial_send.py --port COM3 --file myfile.txt --baud 9600

On Linux/Mac the port might look like /dev/ttyUSB0 or /dev/ttyACM0
instead of COM3.
"""

import argparse
import sys
import threading
import time

import serial  # pyserial


def reader_thread(ser: serial.Serial, stop_event: threading.Event, rx_file) -> None:
    """Continuously read from the serial port, print to stdout, and log to rx_file."""
    while not stop_event.is_set():
        try:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                if data:
                    text = data.decode("utf-8", errors="replace")

                    # Console output (best-effort; never let a display issue kill the thread)
                    try:
                        print(f"[RX] {text}", end="", flush=True)
                    except UnicodeEncodeError:
                        # Console codepage can't display it; fall back to raw bytes.
                        print(f"[RX] {data!r}", end="", flush=True)

                    # File log (always written, regardless of console encoding issues)
                    if rx_file is not None:
                        rx_file.write(text)
                        rx_file.flush()
            else:
                time.sleep(0.02)
        except serial.SerialException:
            # Port likely closed/disconnected; stop quietly.
            break


def send_file(ser: serial.Serial, filepath: str, delay_s: float, line_ending: bytes) -> None:
    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    print(f"[INFO] Sending {len(lines)} line(s) from '{filepath}' "
          f"with {delay_s * 1000:.0f} ms between writes...\n")

    for i, line in enumerate(lines, start=1):
        payload = line.rstrip("\n").rstrip("\r").encode("utf-8") + line_ending
        ser.write(payload)
        ser.flush()
        print(f"[TX {i}/{len(lines)}] {line.rstrip()}")
        time.sleep(delay_s)

    print("\n[INFO] Done sending file.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Send a text file to a serial/COM port, line by line, "
                    "with a delay between lines, while echoing RX data."
    )
    parser.add_argument("--port", required=True,
                         help="COM port to open, e.g. COM3 or /dev/ttyUSB0")
    parser.add_argument("--file", required=True,
                         help="Path to the text file to send")
    parser.add_argument("--baud", type=int, default=9600,
                         help="Baud rate (default: 9600)")
    parser.add_argument("--delay", type=float, default=0.1,
                         help="Delay in seconds between lines (default: 0.1 = 100 ms)")
    parser.add_argument("--line-ending", choices=["lf", "crlf", "cr", "none"],
                         default="lf",
                         help="Line ending appended to each line when sent (default: lf)")
    parser.add_argument("--timeout", type=float, default=1.0,
                         help="Serial read timeout in seconds (default: 1.0)")
    parser.add_argument("--rx-file", default="rx_fc.txt",
                         help="Path to write received (RX) data to (default: rx_fc.txt). "
                              "Pass an empty string ('') to disable RX logging to file.")

    args = parser.parse_args()

    # Force UTF-8 on stdout so RX bytes never crash on a legacy console
    # codepage (e.g. cp1252 on Windows).
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

    line_ending_map = {
        "lf": b"\n",
        "crlf": b"\r\n",
        "cr": b"\r",
        "none": b"",
    }
    line_ending = line_ending_map[args.line_ending]

    rx_file = None
    if args.rx_file:
        try:
            rx_file = open(args.rx_file, "w", encoding="utf-8")
            print(f"[INFO] Logging RX data to '{args.rx_file}'.")
        except OSError as e:
            print(f"[ERROR] Could not open RX log file '{args.rx_file}': {e}", file=sys.stderr)
            sys.exit(1)

    try:
        ser = serial.Serial(port=args.port, baudrate=args.baud, timeout=args.timeout)
    except serial.SerialException as e:
        print(f"[ERROR] Could not open port '{args.port}': {e}", file=sys.stderr)
        if rx_file is not None:
            rx_file.close()
        sys.exit(1)

    print(f"[INFO] Opened {args.port} at {args.baud} baud.")

    stop_event = threading.Event()
    rx_thread = threading.Thread(target=reader_thread, args=(ser, stop_event, rx_file), daemon=True)
    rx_thread.start()

    try:
        send_file(ser, args.file, args.delay, line_ending)
        # Give a moment to catch any trailing responses from the device.
        time.sleep(0.5)
    except FileNotFoundError:
        print(f"[ERROR] File not found: {args.file}", file=sys.stderr)
    except KeyboardInterrupt:
        print("\n[INFO] Interrupted by user.")
    finally:
        stop_event.set()
        rx_thread.join(timeout=1)
        ser.close()
        if rx_file is not None:
            rx_file.close()
        print("[INFO] Port closed.")


if __name__ == "__main__":
    main()