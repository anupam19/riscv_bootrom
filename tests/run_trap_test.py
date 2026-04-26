#!/usr/bin/env python3
"""
Trap test runner for riscv_bootrom.

Loads the trap_payload at 0x80020000, runs QEMU, and verifies that
the BootROM trap handler prints the expected diagnostic message for
an ECALL from M-mode.

Because the trap handler loops forever on WFI, we use a short timeout
and simply grep the captured output — QEMU will not exit cleanly.
"""

import subprocess
import sys
import os
import time

# Expected trap diagnostic substring
EXPECTED_TRAP_MSG = b"ECALL from M-mode"

# QEMU command template - using timeout command to ensure we don't hang
QEMU_CMD = [
    "timeout",  # safety kill switch
    "10s",
    "qemu-system-riscv64",
    "-machine", "virt",
     "-nographic",
     "-bios", "build/bootrom.bin",
     "-device", "loader,file=tests/trap_test/trap_payload.bin,addr=0x80020000,force-raw=on",
 ]

TIMEOUT_SEC = 15


def run_trap_test():
    """Run trap test via QEMU, capture output, verify trap message."""
    print("[INFO] Starting trap test...")

    # Check build artifacts
    if not os.path.exists("build/bootrom.bin"):
        print("[ERROR] build/bootrom.bin not found — run `make` first")
        sys.exit(1)
    if not os.path.exists("tests/trap_test/trap_payload.bin"):
        print("[ERROR] tests/trap_test/trap_payload.bin not found — run `make` in tests/trap_test")
        sys.exit(1)

    # Launch QEMU with timeout
    start = time.time()
    try:
        # Note: using shell=True for the 'timeout' prefix approach
        proc = subprocess.Popen(
            QEMU_CMD,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            shell=False,
        )
        stdout, _ = proc.communicate(timeout=TIMEOUT_SEC)
        elapsed = time.time() - start
    except subprocess.TimeoutExpired:
        proc.kill()
        stdout, _ = proc.communicate()
        print(f"\n[INFO] QEMU terminated after timeout (expected for trap test)")
    except FileNotFoundError:
        print("[ERROR] qemu-system-riscv64 not found — install QEMU")
        sys.exit(1)

    # Save output for debugging / artifact
    with open("tests/trap_test_trap_output.log", "wb") as f:
        f.write(stdout)
    print("[INFO] Trap test output saved to tests/trap_test_trap_output.log")

    # Print tail of output for visibility
    print_output_tail(stdout)

    # Verify expected trap message is present
    if EXPECTED_TRAP_MSG in stdout:
        print(f"\n[PASS] Trap handler diagnostic verified: '{EXPECTED_TRAP_MSG.decode()}'")
        sys.exit(0)
    else:
        print(f"\n[FAIL] Expected trap message '{EXPECTED_TRAP_MSG.decode()}' not found in output")
        sys.exit(1)


def print_output_tail(data: bytes, lines: int = 30):
    lines_list = data.split(b'\n')
    tail = lines_list[-lines:] if len(lines_list) > lines else lines_list
    print("--- QEMU output (tail) ---")
    for line in tail:
        try:
            print(line.decode(errors="replace"))
        except Exception:
            print("<binary or undecodable>")


if __name__ == "__main__":
    run_trap_test()
