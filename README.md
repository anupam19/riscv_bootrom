# RISC-V BootROM

Minimal, single-stage RISC-V BootROM implementation (TF-A BL1 equivalent) for RV64 systems.

## Overview

This is a **bare-metal BootROM** that executes directly from reset on a RISC-V 64-bit core in Machine Mode. It performs minimal initialization and hands off to the next boot stage (e.g., BL2 or bootloader).

## Specification Compliance

- **ISA:** RV64I base (supports M and C extensions via `-march=rv64imac`)
- **Mode:** Machine Mode (M-mode) only
- **Memory:** Physical addressing, no virtual memory, no MMU
- **Traps:** No trap handling — all exceptions/interrupts are fatal
- **Execution:** Single hart (hart 0), single-stage
- **Constraints:** `-ffreestanding`, `-nostdlib`, no heap, no external dependencies

## Boot Flow

```
Reset (PC=0x1000)
    ↓
_start (assembly)
    → Setup stack pointer
    → Zero .bss section
    → Call C entry point
        ↓
    boot_main()
        → Optional UART debug output
        → Platform initialization hook (weak)
        → Jump to next stage at hardcoded address (default 0x80020000)
        → Infinite loop (wfi) if jump fails
```

## Project Structure

```
riscv_bootrom/
├── src/
│   ├── start.S        # Reset entry point, stack setup, bss zero
│   └── boot_main.c    # C entry point with minimal logic
├── include/
│   ├── boot.h         # Core BootROM types and linker symbols
│   └── uart.h         # UART driver API
├── plat/generic/
│   └── platform.c     # Weak platform_init() hook (override per-SoC)
├── drivers/uart/
│   └── uart.c         # Minimal 16550 memory-mapped UART driver
├── linker.ld          # Memory layout (ROM at 0x1000, RAM stack)
├── Makefile           # Build entry
├── config.mk          # Toolchain configuration
└── .github/workflows/ # CI/CD (build, quality, security, release)
```

## Building

### Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu make
```

### Build

```bash
make clean
make TOOLCHAIN_PREFIX=riscv64-linux-gnu-
# OR for unknown-elf toolchain:
# make TOOLCHAIN_PREFIX=riscv64-unknown-elf-
```

**Output:**
- `build/bootrom.elf` — ELF executable (for debugging)
- `build/bootrom.bin` — Raw binary image (to flash into ROM)

### Configuration

- `ENABLE_UART_DEBUG` in `include/boot.h` — set to 0 to disable UART output
- `UART_BASE_ADDR` in `drivers/uart/uart.c` — set MMIO base for UART (default `0x10000000`)
- Jump address in `boot_main.c` — modify `0x80020000` to your next stage load address

## Customizing for Your Platform

### Platform Initialization

Override `platform_init()` by editing `plat/generic/platform.c` or adding a new platform directory:

```c
/* In your platform file */
void platform_init(void)
{
    // Enable clocks, initialize DRAM, configure pinmux, etc.
}
```

### UART Driver

The included driver is a simple 16550-polled implementation. For other UART IPs, modify `drivers/uart/uart.c` or create new drivers.

### Memory Layout

Edit `linker.ld` if your system has different memory map:
- ROM origin and size
- RAM origin for stack
- Stack top pointer

## CI/CD

All commits and PRs trigger:

| Workflow | Purpose |
|---|---|
| Build | Compile with riscv64-linux-gnu, verify artifacts, size check (<8 KB) |
| Code Quality | clang-format linting, cppcheck static analysis, shellcheck |
| CodeQL | Security scanning (C/C++) |
| Cross-Compilation | Verify build with multiple toolchains |
| Multi-Platform Sanity Check | Ubuntu build only |
| Nightly | Daily builds with default & UART-disabled configs |
| Release | On tag push (`v*`), build and publish artifacts |
| Security Scan | GitLeaks, SBOM generation (SPDX + CycloneDX) |
| Static Analysis | Semgrep, enhanced cppcheck |
| Dependabot | Weekly GitHub Actions version updates |

Artifacts are uploaded per-commit; releases are created on tag push.

## Technical Specifications

| Parameter | Value |
|---|---|
| Code size (core) | ~200 SLOC |
| Instructions | RV64I only (no floating point, atomics in base) |
| Toolchain | riscv64-*-elf-gcc or riscv64-linux-gnu-gcc |
| C standard | C11 (freestanding) |
| Stack location | Top of RAM (configurable) |
| Reset vector | 0x1000 (RISC-V standard) |
| Entry point | `_start` (assembly) → `boot_main` (C) |

## Design Philosophy

**Minimalism:** Only essential boot code — no services, no multi-stage, no runtime.

**Extensibility:** Platform and driver layers abstract hardware differences without bloating core.

**Correctness over features:** No heap, no dynamic allocation, no undefined behavior. All memory layout explicit in linker script.

## License

MIT (or your chosen license — add LICENSE file)

## Contributing

PRs welcome. Keep changes minimal and aligned with BootROM constraints: no libc, no heap, no external deps.
