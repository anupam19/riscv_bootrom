#!/usr/bin/env python3
"""
Simple test runner: runs QEMU with BootROM and simple_payload, expects
QEMU to exit with code 0 (test finisher pass). Does not check serial output.
"""

import subprocess
import sys
import os

QEMU_CMD = [
    "qemu-system-riscv64",
    "-machine", "virt",
    "-nographic",
    "-bios", "build/bootrom.bin",
    "-device", "loader,file=tests/simple_payload/simple_payload.bin,addr=0x80020000,force-raw=on,cpu-num=0",
]

TIMEOUT_SEC = 30

def run_simple_test():
    print("[INFO] Starting simple test (test finisher only)...")

    if not os.path.exists("build/bootrom.bin"):
        print("[ERROR] build/bootrom.bin not found")
        sys.exit(1)
    if not os.path.exists("tests/simple_payload/simple_payload.bin"):
        print("[ERROR] tests/simple_payload/simple_payload.bin not found")
        sys.exit(1)

    try:
        proc = subprocess.Popen(
            QEMU_CMD,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        stdout, _ = proc.communicate(timeout=TIMEOUT_SEC)
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, _ = proc.communicate()
        print(f"[FAIL] QEMU timed out after {TIMEOUT_SEC}s")
        print_output(stdout)
        sys.exit(1)
    except FileNotFoundError:
        print("[ERROR] qemu-system-riscv64 not found")
        sys.exit(1)

    # Save output
    with open("simple_test.log", "wb") as f:
        f.write(stdout)

    # Check exit code: 0 means pass (test finisher triggered shutdown)
    if proc.returncode == 0:
        print("[PASS] Simple test passed: QEMU exited with code 0")
        sys.exit(0)
    else:
        print(f"[FAIL] QEMU exited with code {proc.returncode}")
        print_output(stdout)
        sys.exit(1)

def print_output(data: bytes):
    print("--- QEMU output ---")
    try:
        print(data.decode(errors="replace"))
    except Exception:
        print("<binary>")
    print("--- End output ---")

if __name__ == "__main__":
    run_simple_test()
