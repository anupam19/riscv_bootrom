#!/usr/bin/env python3
"""
QEMU integration test runner for riscv_bootrom.

Builds and runs QEMU with the BootROM and payload, verifies expected
serial output, and checks QEMU exit code.
"""

import subprocess
import sys
import os
import time

# Expected output fragments (in order, may be interleaved with other text)
EXPECTED = [
    b"BootROM: Starting",
    b"BootROM: Jumping to next stage",
    b"PAYLOAD: OK",
]

# QEMU configuration
QEMU_CMD = [
    "qemu-system-riscv64",
    "-machine", "virt",
     "-nographic",
     "-bios", "build/bootrom.bin",
     "-device", "loader,file=tests/payload/payload.bin,addr=0x80020000,force-raw=on",
 ]

TIMEOUT_SEC = 60


def run_qemu_test():
    """Run QEMU subprocess, capture output, and verify expectations."""
    print("[INFO] Starting QEMU integration test...")
    print("[DEBUG] QEMU command:", QEMU_CMD)

    # Ensure build artifacts exist
    if not os.path.exists("build/bootrom.bin"):
        print("[ERROR] build/bootrom.bin not found — did you run `make`?")
        sys.exit(1)
    if not os.path.exists("tests/payload/payload.bin"):
        print("[ERROR] tests/payload/payload.bin not found — did you run `make` in tests/payload?")
        sys.exit(1)

    # Launch QEMU
    start = time.time()
    try:
        proc = subprocess.Popen(
            QEMU_CMD,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        stdout, _ = proc.communicate(timeout=TIMEOUT_SEC)
        elapsed = time.time() - start
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, _ = proc.communicate()
        print(f"\n[FAIL] QEMU timed out after {TIMEOUT_SEC}s")
        write_log(stdout)
        sys.exit(1)
    except FileNotFoundError:
        print("[ERROR] qemu-system-riscv64 not found — install QEMU")
        sys.exit(1)

    # Save raw output for artifact
    write_log(stdout)

    # Check exit code
    if proc.returncode != 0:
        print(f"\n[FAIL] QEMU exited with code {proc.returncode}")
        print_output_summary(stdout)
        sys.exit(1)

    # Verify expected strings appear in order
    idx = 0
    for chunk in stdout.split(b'\n'):
        if idx < len(EXPECTED) and EXPECTED[idx] in chunk:
            print(f"[OK] Found: {EXPECTED[idx].decode()}")
            idx += 1

    if idx == len(EXPECTED):
        print(f"\n[PASS] QEMU integration test passed in {elapsed:.2f}s")
        sys.exit(0)
    else:
        print(f"\n[FAIL] Missing {len(EXPECTED) - idx} expected output(s)")
        print_output_summary(stdout)
        sys.exit(1)


def write_log(data: bytes):
    with open("qemu_output.log", "wb") as f:
        f.write(data)
    print("[INFO] QEMU output saved to qemu_output.log")


def print_output_summary(data: bytes):
    """Print last 40 lines of output (or all if fewer)."""
    lines = data.split(b'\n')
    start = max(0, len(lines) - 40)
    print("--- Last output lines ---")
    for line in lines[start:]:
        try:
            print(line.decode(errors="replace"))
        except Exception:
            print("<binary or undecodable>")


if __name__ == "__main__":
    run_qemu_test()
