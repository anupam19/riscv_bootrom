# TF-A Repository Deep Analysis

**Repository:** Trusted Firmware-A (TF-A)  
**Location Analyzed:** `/home/anupam/Dev/trusted-firmware-a`  
**Version:** 2.14.0  
**Analysis Date:** 2026-04-25  

---

## Table of Contents

1. [Repository Overview](#1-repository-overview)
2. [Main Bootloader Stages](#2-main-bootloader-stages)
3. [Platform Support Structure](#3-platform-support-structure)
4. [Drivers Framework](#4-drivers-framework)
5. [Services Architecture](#5-services-architecture)
6. [Common Code & Libraries](#6-common-code--libraries)
7. [Build System](#7-build-system)
8. [Tools & Utilities](#8-tools--utilities)
9. [Documentation](#9-documentation)
10. [Device Tree & Configuration](#10-device-tree--configuration)
11. [Code Quality & CI/CD](#11-code-quality--cicd)
12. [Architectural Patterns](#12-architectural-patterns)
13. [Quick Reference](#13-quick-reference)

---

## 1. Repository Overview

**Trusted Firmware-A (TF-A)** is the reference implementation of secure world software for Arm A-Profile architectures (Armv8-A and Armv7-A), providing:

- Exception Level 3 (EL3) Secure Monitor
- Power State Coordination Interface (PSCI)
- Trusted Board Boot Requirements - CLIENT (TBBR-CLIENT)
- System Control and Management Interface (SCMI)
- Software Delegated Exception Interface (SDEI)
- Secure Partition Manager (SPM)
- Firmware Update (FWU) support
- Measured Boot & Attestation

### Repository Statistics

| Metric | Count | Details |
|--------|-------|---------|
| **Total Source Files** | 4,373 | `.c`, `.S`, `.h` files |
| **Assembly Files** | 413 | `.S` (ARM assembly) |
| **Header Files** | 659 | In `include/` directory |
| **Makefiles** | 402 | Build configuration files |
| **Platform Makefiles** | 110 | One per platform |
| **Documentation Files** | 212 | `.md`, `.rst` formats |
| **DT Source Files** | 186 | `.dts`, `.dtsi` files |
| **Python Tools** | 20 | Development & build utilities |
| **Library Directories** | 79 | Reusable components |
| **CPU Extensions** | 22 | Optional ARMv8/9 features |
| **Supported Platforms** | 110+ | Vendor/SoC/board combinations |
| **Vendor Support** | 33+ | ARM, NXP, ST, Qualcomm, etc. |

---

## 2. Main Bootloader Stages

TF-A uses a **multi-stage bootloader** architecture with clear separation of concerns:

```
BL1 (ROM/ext) → BL2 (FSBL) → BL31 (EL3 Runtime) + BL32 (Secure Payload) → BL33 (Non-secure)
```

### 2.1 BL1 - First Stage Bootloader

**Path:** `bl1/`

**Purpose:** Initial boot stage, typically from ROM or on-chip SRAM. Initializes basic platform, handles secure firmware update (FWU) metadata.

**Structure:**
```
bl1/
├── aarch32/        # 32-bit implementation
│   ├── bl1_entrypoint.S
│   ├── bl1_arch_setup.c
│   ├── bl1_context_mgmt.c
│   └── bl1_exceptions.S
├── aarch64/        # 64-bit implementation (similar structure)
│   ├── bl1_entrypoint.S
│   ├── bl1_arch_setup.c
│   ├── bl1_context_mgmt.c
│   └── bl1_exceptions.S
├── tbbr/           # Trusted Board Boot support
│   └── tbbr_img_desc.c
├── bl1_main.c      # Main entry point (primary init)
├── bl1_fwu.c       # Firmware Update support (major file: 21KB)
├── bl1.ld.S        # Linker script
├── bl1.mk          # Build makefile
└── bl1_private.h   # Private declarations
```

**Key Responsibilities:**
- Minimal platform initialization
- BL2 image authentication (TBBR)
- FWU metadata handling & anti-rollback
- Jump to BL2 entry point

**Total Files:** 13

---

### 2.2 BL2 - Second Stage Bootloader

**Path:** `bl2/`

**Purpose:** Loads and authenticates subsequent images (BL31, BL32, BL33). Handles secure boot chain, loads images from storage (MMC, SPI NOR, etc.), performs certificate chain validation.

**Structure:**
```
bl2/
├── aarch32/        # 32-bit BL2
│   ├── bl2_entrypoint.S
│   ├── bl2_el3_entrypoint.S
│   ├── bl2_arch_setup.c
│   ├── bl2_el3_exceptions.S
│   └── bl2_run_next_image.S
├── aarch64/        # 64-bit BL2
│   ├── (similar files)
├── bl2_main.c      # Main logic: image loading, auth, dispatch
├── bl2_image_load_v2.c  # Image loading (v2 format support)
├── bl2.ld.S        # Linker script (BL2 image)
├── bl2_el3.ld.S    # EL3-specific linker script
├── bl2.mk          # Build makefile
└── bl2_private.h   # Internal headers
```

**Key Responsibilities:**
- Storage device initialization (MMC, SPI, etc.)
- Image loading from filesystems (FAT, GPT)
- Authentication via certificate chain (TBBR)
- Firmware Update (FWU) image handling
- BL2U support for firmware update mode
- Dynamic configuration (fconf)
- Handoff to BL31/BL32 via `bl31_platform_setup()`

**Total Files:** 15

---

### 2.3 BL2U - BL2 Unprivileged (Firmware Update)

**Path:** `bl2u/`

**Purpose:** Special BL2 variant for authenticated firmware update scenarios. Runs in unprivileged mode for security isolation.

**Structure:**
```
bl2u/
├── aarch32/        # 32-bit
├── aarch64/        # 64-bit
├── bl2u_main.c     # Entry point
├── bl2u.ld.S       # Linker script
└── bl2u.mk         # Build config
```

**Total Files:** 10

---

### 2.4 BL31 - Bootloader Stage 31 (EL3 Runtime)

**Path:** `bl31/`

**Purpose:** The **core EL3 runtime** providing secure monitor functions, power management (PSCI), interrupt handling, service dispatch, and runtime services. This is the largest and most complex stage.

**Structure:**
```
bl31/
├── aarch64/        # BL31 is AArch64-only
│   ├── bl31_entrypoint.S      # Entry from BL2
│   ├── crash_reporting.S      # Crash dump generation
│   ├── ea_delegate.S          # External abort delegation
│   ├── runtime_exceptions.S   # Exception vectors
│   └── bl31_arch_setup.c      # Architecture init
├── bl31_main.c                # Main service loop (~11KB)
├── bl31.mk                    # Build configuration
├── bl31.ld.S                  # Main linker script (~6KB)
├── bl31_context_mgmt.c        # World context switching (~2.5KB)
├── bl31_traps.c               # Trap handling (~7KB)
├── ehf.c                      # External Hierarchical Fault (RME) (~16KB)
├── interrupt_mgmt.c           # Interrupt management (~8KB)
├── psci.c                     # PSCI implementation integration
├── pwr_state.c                # Power state management
└── [other service integrations]
```

**Integrated Services:**
- **PSCI** (Power State Coordination Interface)
- **SDEI** (Software Delegated Exception Interface)
- **SPM/SPMD** (Secure Partition Manager/Dispatcher)
- **RMMD** (Realm Management Monitor Driver - for RME)
- **TRNG** (True Random Number Generator)
- **Standard Power Management**
- **Interrupt routing & delegation**

**Total Files:** 19 core files + integrations

**Artifacts:** `bl31.bin`, `bl31.elf`

---

### 2.5 BL32 - Bootloader Stage 32 (Secure Payload)

**Path:** `bl32/`

**Purpose:** Secure world runtime environment. Three supported payload types:

#### **A. SP-MIN (Secure Partition Manager Minimal)**

**Path:** `bl32/sp_min/`

**Structure:**
```
bl32/sp_min/
├── aarch32/        # 32-bit only
│   ├── sp_min_main.c
│   ├── sp_min_entrypoint.S
│   └── sp_min_helpers.S
├── sp_min_main.c   # Minimal SPM implementation (~9KB)
├── sp_min.ld.S     # Linker script
├── sp_min.mk       # Build config
└── [CVE-2017-5715 workarounds]
```

**Purpose:** Minimal secure payload for simple use cases, no FF-A, basic context management.

**Total Files:** ~7

---

#### **B. TSP (Trusted Services Payload)**

**Path:** `bl32/tsp/`

**Structure:**
```
bl32/tsp/
├── aarch64/        # 64-bit TSP
│   ├── tsp_main.c           # Core TSP logic (~9KB)
│   ├── tsp_ffa_main.c       # FF-A interface (~20KB) - main FF-A handling
│   ├── tsp_context.c         # Context management
│   ├── tsp_interrupt.c       # Interrupt handling
│   ├── tsp_timer.c           # Timer support
│   ├── tsp_entrypoint.S      # Assembly entry
│   └── tsp_helpers.S         # Helper routines
├── tsp.mk          # Build config
├── tsp.ld.S        # Linker script
├── ffa_helpers.c   # FF-A helper functions
├── ffa_helpers.h
└── tsp_private.h
```

**Purpose:** Full-featured secure service runtime using **FF-A (Firmware Framework for A-profile)** specification. Supports multiple secure partitions, message passing, and resource management.

**Total Files:** ~12

---

#### **C. OP-TEE (Open Portable TEE)**

**Path:** `bl32/optee/`

**Purpose:** Integration layer for OP-TEE OS as a secure payload. Provides dispatcher for OP-TEE.

**Note:** External project integration, not part of core TF-A build by default.

---

## 3. Platform Support Structure

**Path:** `plat/`

TF-A supports **110+ platforms** across **33+ vendors**. Platform code is **completely isolated** from generic code, enabling easy porting.

### 3.1 Platform Directory Pattern

```
plat/<vendor>/<soc>/<board>/       # Common pattern
plat/<vendor>/<board>/              # Alternative (simpler)
plat/<vendor>/common/               # Shared code across boards
plat/<vendor>/soc/                  # SoC-specific code
```

Each platform directory contains:

**Required:**
- `platform.mk` - Main build configuration (defines `PLATFORM`, sources, includes)
- Platform-specific source files in `bl1/`, `bl2/`, `bl31/`, `bl32/` subdirs (if overrides needed)

**Optional:**
- `platform_defaults.mk` - Default variable values
- Helper `.mk` files in subdirectories
- `include/` - Platform-specific headers
- `drivers/` - Platform-specific drivers (or use generic ones)

---

### 3.2 Major Platform Vendors

#### **ARM (Reference Platforms)**

**Path:** `plat/arm/`

Most comprehensive platform support, includes reference models:

```
plat/arm/
├── board/           # Development boards
│   ├── fvp/                 # Fixed Virtual Platform (primary dev platform)
│   ├── juno/                # Juno ARM Development Platform
│   ├── morello/             # Morello (experimental CHERI)
│   ├── n1sdp/              # Neoverse N1 System Development Platform
│   ├── neoverse_rd/        # Neoverse Reference Design (rdv3, rdn2)
│   ├── tc/                 # Total Compute platforms
│   ├── corstone1000/       # Corstone-1000 (CPU+NPU)
│   ├── corstone700/        # Corstone-700 (IoT)
│   ├── fvp_ve/             # FVP Virtual Express
│   ├── arm_fpga/           # FPGA-based platforms
│   └── automotive_rd/      # Automotive reference (rd1ae, rdaspen)
├── soc/             # SoC abstraction
│   └── common/             # Common SoC drivers
│       ├── css/            # CoreSight SoC-400/600
│       └── scmi/           # SCMI protocol
└── common/          # Shared ARM platform code
    ├── aarch32/            # 32-bit common code
    ├── aarch64/            # 64-bit common code
    ├── fconf/              # Firmware configuration
    ├── sp_min/             # SP-MIN support
    ├── trp/                # Test and Debug (TRP)
    ├── tsp/                # TSP support
    ├── arm_bl1_setup.c     # BL1 platform setup
    ├── arm_bl2_setup.c     # BL2 setup (~13KB)
    ├── arm_bl31_setup.c    # BL31 setup (~17KB)
    ├── arm_bl2u_setup.c    # BL2U setup
    ├── arm_common.c/h      # Common utilities
    ├── arm_dyn_cfg.c/h     # Dynamic config
    ├── arm_io_storage.c    # Storage abstraction
    ├── arm_pm.c            # Power management
    ├── arm_image_load.c    # Image loading
    ├── plat_gicv2.c        # GICv2 support
    ├── plat_gicv3.c        # GICv3 support
    ├── arm_cci.c           # CCI-400/500 interconnect
    ├── arm_ccn.c           # CCN interconnect
    └── arm_tzc*.c          # TrustZone controller
```

**Key Files:**
- `arm_common.h/c` - Main platform utility API
- `arm_dyn_cfg.h/c` - Dynamic config structures (fconf)
- `arm_io_storage.h/c` - Storage device abstraction (MMC, SPI, etc.)

---

#### **NXP (i.MX & Layerscape)**

**Path:** `plat/imx/`

Extensive support for i.MX and Layerscape families:

```
plat/imx/
├── common/                    # Shared code across all i.MX
│   ├── aarch32/              # 32-bit common
│   ├── aarch64/              # 64-bit common
│   ├── sci/                  # System Controller Interface (SCMI-like)
│   ├── ddr/                  # DDR initialization & training
│   ├── scmi/                 # SCMI protocol support
│   ├── imx8/
│   ├── imx93/
│   ├── imx95/
│   ├── drivers/              # Common i.MX drivers
│   │   ├── console/
│   │   ├── crypto/
│   │   ├── gpc/
│   │   ├── i2c/
│   │   ├── src/
│   │   └── ...
│   ├── include/
│   │   └── plat/
│   │       └── imx/
│   │           ├── common/
│   │           ├── drivers/
│   │           └── platform_info.h
│   └── lib/
│       └── [common libs]
├── imx7/                     # i.MX 7 series
├── imx8m/                    # i.MX 8M family
│   ├── imx8mm/              # 8 Mini
│   ├── imx8mn/              # 8 Nano
│   ├── imx8mp/              # 8 Plus
│   └── imx8mq/              # 8 Quad
├── imx8qm/                   # i.MX 8QM (quad-core)
├── imx8qx/                   # i.MX 8QX (quad + cortex-A)
├── imx8ulp/                  # i.MX 8ULP
├── imx9/                     # i.MX 9 series
│   ├── imx93/
│   ├── imx95/
│   └── imx94/
├── soc-ls1028a/              # Layerscape LS1028A
│   ├── ls1028ardb/
│   ├── ls1028aqds/
│   └── ...
└── [more Layerscape: ls1043a, ls1046a, ls1088a, lx2160a, etc.]
```

**Platform Files:**
- Each platform dir contains `platform.mk`, `bl31/`, `common/`, `include/`, drivers

---

#### **STMicroelectronics (STM32)**

**Path:** `plat/st/`

**Structure:**
```
plat/st/
├── stm32mp1/                 # STM32MP1 series (Cortex-A7 + M4)
│   ├── common/              # Shared across boards
│   │   ├── drivers/         # Extensive: bsec, clk, crypto, ddr, etc.
│   │   ├── include/
│   │   │   └── plat/
│   │   │       └── st/
│   │   │           └── stm32mp1/
│   │   └── stm32mp1.c
│   ├── boards/
│   │   ├── stm32mp157c-dk2/
│   │   ├── stm32mp157c-ev1/
│   │   ├── stm32mp135f-dk/
│   │   └── ...
│   └── services/            # Platform services
├── stm32mp2/                 # STM32MP2 series (Cortex-A35/A53)
│   └── [similar structure]
├── drivers/                  # ST-specific drivers (symlinked or separate)
└── common/                   # Common ST code
```

**Key Drivers:**
- **BSEC** - One-Time Programmable (OTP) memory
- **CLK** - Clock management
- **CRYPTO** - Hardware crypto accelerators
- **DDR** - DDR controller initialization
- **ETZPC** - TrustZone protection
- **FMC** - Flash memory controller
- **GPIO, I2C, IWDG, MCE, MMC, PMIC, RCC, RESET, RIF, SPI, USART, USB**

---

#### **Qualcomm (qti)**

**Path:** `plat/qti/`

```
plat/qti/
├── common/                   # Shared Qualcomm code
│   ├── inc/                 # Headers
│   └── src/                 # Sources
├── kodiak/                  # Snapdragon 8 Gen platforms
│   ├── rb3gen2/
│   └── sc7280_chrome/
├── lemans/                  # SC7180 family
│   └── lemans_evk/
├── qcs615/                  # QCS615 platform
├── msm8916/                 # Legacy: Snapdragon 410
├── msm8939/                 # Legacy: Snapdragon 430
├── msm8909/                 # Legacy: Snapdragon 210
├── mdm9607/                 # Legacy: IoT modem
├── [sp_min/, tsp/ subdirs per platform]
└── qtiseclib/               # Secure library
```

---

#### **MediaTek**

**Extensive support for 14+ SoCs:**

```
plat/mediatek/
├── common/                  # Common Mediatek code
├── mt8173/
├── mt8183/
├── mt8186/
├── mt8188/
├── mt8189/
├── mt8192/
├── mt8195/
├── mt8196/
├── qcs615/                  # Shared with Qualcomm?
├── sc7180/                  # Shared with Qualcomm?
└── [each has: platform.mk, bl31/, common/, drivers/, include/, lib/]
```

---

#### **Renesas**

**Path:** `plat/renesas/`

```
plat/renesas/
├── common/                  # Common Renesas code
├── rcar/                    # R-Car family
│   ├── common/
│   ├── gen3/               # R-Car H3/M3/M3N
│   ├── gen4/               # R-Car E3/VERSAILITY
│   └── gen5/               # R-Car V4H/U
├── rza/                     # RZ/A family
│   ├── common/
│   ├── soc/rza3m/
│   └── board/rza3m_ek_nor/
├── rzg/                     # RZ/G family
├── synquacer/              # Synquacer SC2A11
└── [each platform has drivers/, include/, lib/]
```

---

#### **Rockchip**

**Path:** `plat/rockchip/`

11+ SoCs supported:

```
plat/rockchip/
├── common/                 # Common Rockchip code
│   ├── include/
│   ├── drivers/
│   └── ...
├── px30/                   # RK3308/PX30
├── rk3288/                 # RK3288
├── rk3328/                 # RK3328
├── rk3368/                 # RK3368
├── rk3399/                 # RK3399 (most common)
├── rk3568/                 # RK3568
├── rk3576/                 # RK3576
├── rk3588/                 # RK3588
├── [more]
└── [each: platform.mk, bl31/, common/, drivers/]
```

**Common Features:** SCMI PMU, SRAM, DRAM, UART, etc.

---

#### **Raspberry Pi**

**Path:** `plat/raspberrypi/`

```
plat/raspberrypi/
├── rpi3/                   # Raspberry Pi 3 (BCM2837)
├── rpi4/                   # Raspberry Pi 4 (BCM2711)
├── rpi5/                   # Raspberry Pi 5 (BCM2712)
└── common/                 # Shared aarch64 code
```

---

#### **Other Notable Vendors**

| Vendor | Path | Platforms | Notes |
|--------|------|-----------|-------|
| **AMD** | `plat/amd/` | versal2, versal | Includes PM service, TSP |
| **Intel** | `plat/intel/` | agilex, agilex5, n5x, stratix10 | FPGA SoCs |
| **Marvell** | `plat/marvell/` | armada, octeontx | COMPHY, MC TrustZone |
| **Broadcom** | `plat/brcm/` | stingray | eMMC, I2C, MDIO, SPI |
| **Hisilicon** | `plat/hisilicon/` | hikey, hikey960, poplar | HiKey boards |
| **NVIDIA** | `plat/nvidia/` | tegra | Tegra SoC family |
| **Nuvoton** | `plat/nuvoton/` | npcm845x | NPCM8xx |
| **Texas Instruments** | `plat/ti/` | k3, k3low | K3 multicore |
| **Xilinx** | `plat/xilinx/` | zynqmp, versal, versal_net | Zynq UltraScale+, Versal |
| **AllWinner** | `plat/allwinner/` | sun50i_a64, sun50i_h6, etc. | A64, H6, H616, R329 |
| **Amlogic** | `plat/amlogic/` | axg, g12a, gxbb, gxl | S905, S912 |
| **Cadence** | `plat/cadence/` | [platforms] | USB, eMMC drivers |
| **Aspeed** | `plat/aspeed/` | ast2700 | BMC SoCs |
| **QEMU** | `plat/qemu/` | qemu, qemu_sbsa | Emulation platforms |

---

### 3.3 Platform Makefile Convention

Every platform **must** define a `platform.mk` with:

```makefile
# Required variables:
PLATFORM            := <platform_name>
PLATFORM_FLAVOR     := <flavor> (optional)
PLATFORM_VERSION    := <version> (optional)

# Source files:
BL1_SOURCES         := (if BL1 overrides)
BL2_SOURCES         := (if BL2 overrides)
BL31_SOURCES        := (BL31 sources)
BL32_SOURCES        := (if BL32 is built-in)
BL33_SOURCES        := (if BL33 is built-in)

# Include paths:
INCLUDES            := -Ipath/to/plat/include

# Platform-specific compile flags:
PLAT_CPPFLAGS       := -D<defines>

# Dependencies:
$(eval $(call add_platform_dependency,<subdir>))

# Optional: include platform_defaults.mk
include plat/$(PLATFORM)/platform_defaults.mk
```

**Platform Discovery:** Build system recursively finds all `plat/*/*/platform.mk` files via `rwildcard`.

**Selection:** `make PLAT=<name> [other options]`

---

## 4. Drivers Framework

**Path:** `drivers/`

**Total Driver Categories:** 33 top-level directories  
**Driver Philosophy:** Vendor-specific code isolated; common APIs defined in headers.

### 4.1 Driver Organization

```
drivers/
├── arm/                   # ARM IP drivers (most extensive)
│   ├── cci/              # CCI-400, CCI-500 interconnects
│   ├── ccn/              # CCN interconnect
│   ├── css/              # CoreSight SoC-400/600
│   ├── dcc/              # Data Coherency Controller
│   ├── dsu/              # DynamIQ Shared Unit
│   ├── ethosn/           # NPU (Ethos-N)
│   ├── fvp/              # FVP-specific drivers
│   ├── gic/              # Generic Interrupt Controller v2/v3
│   │   ├── gicv3.c
│   │   ├── gicv3_setup.c
│   │   ├── gicv3_its.c   # Interrupt Translation Service
│   │   ├── gicv3_redist.c
│   │   └── gicv4.c       # GICv4 support
│   ├── gicv5/
│   ├── mhu/              # Message Handling Unit
│   ├── pl011/            # PrimeCell UART (16550-compatible)
│   ├── pl061/            # PrimeCell GPIO
│   ├── rse/              # Root of Trust for Measurement (RSE)
│   ├── sbsa/             # Server Base System Architecture
│   ├── sfcp/             # SCF/SBMC Flash Controller
│   ├── smmu/             # System MMU (IOMMU)
│   ├── sp804/            # SP804 timer
│   ├── sp805/            # SP805 watchdog
│   └── tzc/              # TrustZone Controller
├── auth/                  # Authentication/crypto
│   ├── cca/              # CCA certificate chain
│   ├── dualroot/         # Dual-root certificate support
│   ├── mbedtls/          # mbedTLS integration
│   └── tbbr/             # TBBR authentication
├── console/               # Console abstraction
│   ├── aarch32/
│   └── aarch64/
├── delay_timer/           # Timer/delay utilities
├── fwu/                   # Firmware Update
│   └── fwu.c (8,831 bytes)
├── gpio/                  # GPIO controllers
├── io/                    # I/O abstraction (block devices)
├── mmc/                   # MMC/SD card support
├── mtd/                   # Memory Technology Devices
│   ├── nand/             # NAND flash
│   ├── nor/              # NOR flash
│   └── spi-mem/          # SPI NOR/NAND
├── partition/             # Partition management (GPT, MBR)
├── ufs/                   # Universal Flash Storage
├── usb/                   # USB support
├── amlogic/               # Amlogic-specific
│   ├── console/
│   └── crypto/
├── brcm/                  # Broadcom
│   ├── emmc/
│   ├── i2c/
│   ├── mdio/
│   └── spi/
├── cadence/               # Cadence IP
│   ├── combo_phy/
│   ├── emmc/
│   ├── nand/
│   └── uart/
├── cfi/                   # CFI flash (v2m platform)
├── coreboot/              # Coreboot integration
│   └── cbmem_console/
├── imx/                   # NXP i.MX
│   ├── timer/
│   ├── uart/
│   └── usdhc/            # MMC host
├── intel/                 # Intel SoC drivers
├── marvell/               # Marvell
│   ├── comphy/           # PHY configuration
│   ├── mc_trustzone/     # MC TrustZone
│   ├── mg_conf_cm3/      # Management controller
│   ├── mochi/            # Mochi interconnect
│   ├── secure_dfx_access/
│   └── uart/
├── measured_boot/         # Measured boot infrastructure
│   ├── event_log/        # Event log formatting
│   └── rse/              # Root of Trust for Measurement
├── mentor/               # Mentor I2C
├── nxp/                  # Additional NXP (non-i.MX)
│   ├── auth/
│   ├── clk/
│   ├── console/
│   ├── crypto/
│   ├── csu/              # Central Security Unit
│   ├── dcfg/
│   ├── ddr/
│   ├── flexspi/
│   ├── gic/
│   ├── gpio/
│   ├── i2c/
│   ├── ifc/              # Interface
│   ├── interconnect/
│   ├── pmu/
│   ├── qspi/
│   ├── scmi/
│   ├── sd/
│   ├── sec_mon/          # Security Monitor
│   ├── sfp/
│   ├── timer/
│   ├── trdc/             # Trusted Read-Write Dispatcher
│   └── tzc/
├── qti/                  # Qualcomm
│   ├── accesscontrol/
│   ├── crypto/
│   ├── qtimer/
│   ├── sec_core/
│   └── watchdog/
├── renesas/              # Renesas
│   ├── common/
│   ├── rcar/
│   ├── rcar_gen4/
│   ├── rcar_gen5/
│   ├── rza/
│   └── rzg/
├── rpi3/                 # Raspberry Pi 3
│   ├── gpio/
│   ├── mailbox/
│   ├── rng/
│   └── sdhost/
├── scmi-msg/             # SCMI message protocol driver
├── st/                   # STMicroelectronics (very comprehensive)
│   ├── bsec/             # OTP memory controller
│   ├── clk/              # Clock tree
│   ├── crypto/           # Crypto accelerators
│   ├── ddr/              # DDR PHY & controller
│   ├── etzpc/            # TrustZone protection
│   ├── fmc/              # Flash memory
│   ├── gpio/
│   ├── i2c/
│   ├── iwdg/             # Independent watchdog
│   ├── mce/              # Memory Control & Encryption
│   ├── mmc/
│   ├── pmic/
│   ├── regulator/
│   ├── reset/
│   ├── rif/              # Resource isolation
│   ├── spi/
│   ├── uart/
│   ├── usb/
│   └── usb_dwc3/
├── synopsys/             # Synopsys IP
│   ├── emmc/
│   └── ufs/
├── ti/                   # Texas Instruments
│   ├── ipc/              # Inter-processor communication
│   ├── ti_sci/           # System Control Interface
│   └── uart/
└── tpm/                  # Trusted Platform Module
    └── tpm2_slb9670/
```

---

### 4.2 Driver Pattern

Each driver typically includes:

```
drivers/<vendor>/<driver>/
├── <driver>.c           # Implementation
├── <driver>.h           # Internal header (if needed)
├── <driver>_data.c      # Data structures (optional)
├── <driver>.mk          # Build file
└── [platform_*.c]       # Platform variations (if needed)
```

Corresponding headers in: `include/drivers/<vendor>/<driver>/<driver>.h`

**Registration Pattern:**
- Drivers register with framework via init functions
- Platform provides driver instances in `platform.mk` via `BL2_SOURCES`, `BL31_SOURCES`, etc.
- Driver may have `driver_<name>_init()` called at appropriate boot stage

---

### 4.3 Key Driver Types

| Driver Type | Purpose | Examples |
|-------------|---------|----------|
| **Interrupt Controller** | Handle interrupts at EL3 | GICv2, GICv3, GICv5 |
| **Timer** | System timers, delays | SP804, SP805, generic |
| **Console** | Debug output | PL011 UART, USB CDC, SMCCC |
| **Storage** | Block device I/O | MMC, SPI NOR, USB mass storage |
| **Crypto** | Hardware acceleration | CryptoCell, CryptoCell-713, ST CRYP |
| **Power Management** | Power state transitions | SCMI, PSCI, vendor PMUs |
| **MMU/IOMMU** | Address translation | SMMU, MPU |
| **DDR** | Memory initialization | DDR PHY, training |
| **Security** | TrustZone, isolation | TZC, DDC, RIF |

---

## 5. Services Architecture

**Path:** `services/`

TF-A provides **runtime services** that can be invoked from lower ELs (EL1/EL2) via SMC calls. Services are registered with the **Runtime Service Framework**.

### 5.1 Runtime Service Framework

**Core Files:**
- `runtime_svc.h` - Service registration API
- `runtime_svc.c` - Dispatcher logic

**Service Registration Pattern:**

```c
svc_desc_t svc_desc = {
    .svc_uid = UUID,           // Service UUID
    .svc_version_major = 1,
    .svc_version_minor = 0,
    .svc_ops = &svc_ops,       // Function pointers
};

void svc_init(void) {
    /* Register service */
}
```

Services are called via `SMC` (Secure Monitor Call) instructions.

---

### 5.2 Standard Services (`std_svc/`)

**Path:** `services/std_svc/`

ARM-defined standard services:

```
services/std_svc/
├── drtm/           # Dynamic Root of Trust for Measurement
│   └── drtm_main.c
├── errata_abi/     # CPU errata ABI handling
├── firme/          # Firmware Measurement Environment
├── lfa/            # Low Firmware Attestation
├── rmmd/           # Realm Management Monitor Driver (RME)
│   └── trp/        # Transitional Realm Participant
│       └── trp_main.c
├── sdei/           # Software Delegated Exception Interface
│   ├── sdei_main.c        (~27KB - core)
│   ├── sdei_intr_mgmt.c   (~20KB - interrupt handling)
│   ├── sdei_event.c
│   └── sdei_private.h
├── spm/            # Secure Partition Manager
│   ├── common/            # SPM common code
│   │   ├── spm.c          # SPM core logic
│   │   ├── spm_def.h
│   │   └── spm_private.h
│   ├── el3_spmc/          # EL3 SPM (SPMD at EL3)
│   │   └── el3_spmc.c
│   └── spm_mm/            # SPM Memory Management
│       └── spm_mm.c
├── spmd/           # SPM Dispatcher (main routing logic)
│   ├── spmd_main.c       (~45KB - main dispatcher)
│   ├── spmd_logical_sp.c (~29KB - logical SP management)
│   ├── spmd_helpers.c
│   ├── spmd_private.h
│   └── spd.h             # SPD interface definitions
└── trng/           # True Random Number Generator
    └── trng.c
```

**Service Summary:**
| Service | Description | Key Files |
|---------|-------------|-----------|
| **PSCI** | Power management (CPU on/off, suspend) | Integrated via `psci.c` in BL31 |
| **SDEI** | Event-driven interrupts from NS world | `sdei_main.c`, `sdei_intr_mgmt.c` |
| **SPMD** | Secure Partition Dispatcher (FF-A compliant) | `spmd_main.c`, `spmd_logical_sp.c` |
| **RMMD** | Realm Management (RME feature) | `rmmd_main.c` |
| **TRNG** | Hardware random number service | `trng.c` |
| **DRTM** | Dynamic Root of Trust for Measurement | `drtm_main.c` |

---

### 5.3 Secure Partition Dispatchers (SPD)

**Path:** `services/spd/`

SPDs are **secure world OS dispatchers** - each manages a different secure OS (TEE).

```
services/spd/
├── opteed/        # OP-TEE Dispatcher (most common)
│   ├── opteed_main.c    (~25KB)
│   ├── opteed_common.c
│   ├── opteed_pm.c      # Power management
│   ├── teesmc_opteed.h  # SMC interface
│   ├── opteed_private.h
│   └── [arch/]
├── tlkd/          # Trusted Logic Kernel Dispatcher
│   ├── tlkd_main.c      (~15KB)
│   ├── tlkd_common.c
│   ├── tlkd_pm.c
│   └── [headers]
├── tspd/          # TSP Dispatcher (for TSP payload)
│   ├── tspd_main.c      (~25KB)
│   ├── tspd_common.c
│   ├── tspd_pm.c
│   ├── tspd_helpers.S
│   └── [headers]
└── trusty/        # Google's Trusty TEE
    └── trusty.c
```

**SPD Selection:**

Built via `SPD` make variable:
- `SPD=opteed` - OP-TEE (recommended, most tested)
- `SPD=tlkd` - Trusted Logic
- `SPD=tspd` - TSP (deprecated in favor of SP-MIN)
- `SPD=trusty` - Trusty
- `SPD=pncd` - PM Config Node (for ST platforms)
- `SPD=none` - No secure payload

**SPD Loading:** SPMD loads `.sp` image (BL32) and dispatches to appropriate SPD.

---

### 5.4 Architecture Service (`arm_arch_svc/`)

**Path:** `services/arm_arch_svc/`

Registers ARM architecture-standard services:
- PSCI
- SDEI
- SMCCC (SMC Calling Convention)

**Files:**
- `arm_arch_svc_setup.c` - Service registration

---

### 5.5 Vendor EL3 Services (`el3/`)

**Path:** `services/el3/`

- `ven_el3_svc.c` - Vendor-specific EL3 services

---

### 5.6 OEM Services (`oem/`)

**Path:** `services/oem/`

Platform/OEM-specific services:
- `chromeos/` - ChromeOS-specific services

---

## 6. Common Code & Libraries

### 6.1 Common Code (`common/`)

**Path:** `common/`

Architecture-agnostic code shared across bootloader stages.

```
common/
├── aarch32/                    # AArch32-specific common
│   ├── debug.S                 # Debug assembly helpers
│   └── early_exceptions.S      # Early exception handling
├── aarch64/                    # AArch64-specific common
│   ├── debug.S
│   └── early_exceptions.S
├── backtrace/                  # Stack unwinding
│   ├── backtrace.c
│   └── backtrace.mk
├── bl_common.h                 # Common bootloader definitions
├── bl_aux_params.h/c           # Auxiliary parameters passed between stages
├── context_mgmt.h/c            # World context management (EL1/EL3)
├── debug.h/c                   # Debug macros (LOG macros)
├── assert.h                    # Assertion macros
├── dyn_cfg.h/c                 # Dynamic configuration framework
├── feat_detect.h/c             # Feature detection (CPU features)
├── fwu.h/c                     # Firmware Update metadata
├── hob.h/c                     # Handoff Block (HOB) structures
├── runtime_svc.h/c             # Service registration framework
├── tbbr.h/c                    # TBBR definitions
├── gpt_rme.h                   # GPT-RME (Granule Protection Tables)
└── [misc utility headers]
```

**Purpose:** Avoid code duplication across BL1/BL2/BL31/BL32. Shared types, macros, helper functions.

---

### 6.2 Libraries (`lib/`)

**Path:** `lib/`

Reusable, modular libraries. Can be used across stages and platforms.

```
lib/
├── aarch32/                    # AArch32 runtime support
├── aarch64/                    # AArch64 runtime support
├── cpus/                       # CPU-specific operations
│   ├── cpu-ops.mk              # CPU errata workarounds (67KB)
│   ├── errata_common.c
│   ├── errata_report.c
│   ├── aarch32/
│   └── aarch64/
├── el3_runtime/                # EL3 runtime support
│   ├── aarch32/
│   └── aarch64/
├── xlat_tables/                # Translation table management (v1)
│   ├── xlat_tables_common.c
│   ├── aarch32/
│   └── aarch64/
├── xlat_tables_v2/             # Newer translation table API
│   ├── xlat_tables_v2.c
│   └── [arch-specific]
├── utils/                      # General utilities
├── libc/                       # Minimal C library
│   ├── aarch32/
│   └── aarch64/
│       ├── string.c            # memcpy, memset, strcmp, etc.
│       └── ...
├── libfdt/                     # Flattened Device Tree library
│   └── [from upstream libfdt]
├── zlib/                       # Compression (zlib)
│
├── psci/                       # PSCI implementation
│   ├── psci_common.c          (~51KB - core logic)
│   ├── psci_main.c
│   ├── psci_on.c
│   ├── psci_off.c
│   ├── psci_suspend.c
│   ├── psci_setup.c
│   ├── psci_stat.c
│   ├── aarch32/
│   └── aarch64/
│
├── fconf/                      # Firmware Configuration
│   ├── fconf.c
│   ├── fconf_common.c
│   ├── fconf_dyn_cfg.c
│   └── [arch-specific]
│
├── hob/                        # Handoff Block
├── transfer_list/              # Transfer list data structure
│
├── dice/                       # DICE - Device Identifier Composition Engine
├── measured_boot/               # Measured boot event log
│
├── psa/                        # PSA APIs (Platform Security Architecture)
│
├── extensions/                 # Optional CPU feature extensions (22 subdirs!)
│   ├── amu/                    # Activity Monitors Unit
│   ├── brbe/                   # Branch Record Buffer Extension
│   ├── cpa2/                   # Checked Pointer Authentication (v2)
│   ├── debug/                  # Debug extensions
│   ├── fgt/                    # Fine-Grained Traps
│   ├── idte/                   # Enhanced Translation Tables
│   ├── mpam/                   # Memory Partitioning & Monitoring
│   ├── pauth/                  # Pointer Authentication (v1)
│   ├── pmuv3/                  # Performance Monitors v3
│   ├── ras/                    # Reliability, Availability, Serviceability
│   ├── sme/                    # Scalable Matrix Extension
│   ├── spe/                    # Statistical Profiling Extension
│   ├── sve/                    # Scalable Vector Extension
│   ├── sys_reg_trace/          # System register tracing
│   ├── sysreg128/              # 128-bit system registers
│   ├── tcr/                    # TCR updates for extensions
│   ├── trbe/                   # Trace Buffer Extension
│   └── trf/                    # Trace Filtering
│
├── semihosting/                # Semihosting support (debug)
│   ├── aarch32/
│   └── aarch64/
│
├── stack_protector/            # Stack canary protection
│   ├── aarch32/
│   └── aarch64/
│
├── debugfs/                    # Debug filesystem
├── per_cpu/                    # Per-CPU data structures (aarch64)
├── pmf/                        # Power Management Framework
│
├── romlib/                     # ROM library support
│   └── templates/              # Code generation templates
│
├── gpt_rme/                    # Granule Protection Tables (RME)
├── locks/                      # Locking primitives
│   ├── bakery/                 # Bakery locks
│   └── exclusive/              # Exclusive monitors
│
├── bl_aux_params/              # Auxiliary boot parameters
├── coreboot/                   # Coreboot integration
│
├── compiler-rt/                # LLVM compiler runtime
│   └── [built-in functions]
│
└── [others]
```

---

### 6.3 Third-Party Libraries

External codebases integrated into TF-A:
- **libfdt** - Device Tree manipulation (from Linux kernel)
- **zlib** - Compression (from zlib.net)
- **mbedTLS** - Crypto library (in `contrib/` or git submodule)
- **compiler-rt** - LLVM runtime support

---

## 7. Build System

**Primary Build Files:**
- `Makefile` - Main entry point (1,324 lines)
- `make_helpers/` - Build helper modules

### 7.1 Makefile Structure

**Top-Level Makefile:**

```makefile
# Version
VERSION_MAJOR = 2
VERSION_MINOR = 14

# Include core make helpers
include make_helpers/common.mk
include make_helpers/defaults.mk
include make_helpers/arch_features.mk
include make_helpers/march.mk
include make_helpers/cflags.mk
include make_helpers/build-rules.mk
include make_helpers/plat_helpers.mk
include make_helpers/toolchain.mk
include make_helpers/utilities.mk
include make_helpers/constraints.mk
include make_helpers/tbbr/tbbr_tools.mk

# Platform discovery
ALL_PLATFORM_MK_FILES := $(call rwildcard,plat/,platform.mk)
ALL_PLATFORMS := $(sort $(notdir $(dir $(ALL_PLATFORM_MK_FILES))))

# Main targets:
#   all         - Build everything (default)
#   fip         - Build Firmware Image Package
#   sp          - Build Secure Partition
#   fwu_fip     - Build FWU image
#   certtool    - Build certificate tool
#   fiptool     - Build FIP manipulation tool
#   sptool      - Build SP packaging tool
#   doc         - Build documentation
#   clean       - Clean build artifacts
```

---

### 7.2 Make Helpers (`make_helpers/`)

| File | Purpose |
|------|---------|
| **`common.mk`** | Common definitions, path setup |
| **`defaults.mk`** | Default build values (DEBUG=0, etc.) |
| **`arch_features.mk`** | Architecture feature flags detection |
| **`march.mk`** | Architecture (AArch32/AArch64) selection |
| **`cflags.mk`** | Compiler flags per architecture/platform |
| **`build-rules.mk`** | Pattern rules for building objects |
| **`plat_helpers.mk`** | Platform discovery & inclusion logic |
| **`toolchain.mk`** | Toolchain configuration (GCC/Clang/ARM) |
| **`utilities.mk`** | Utility functions (wildcard, filtering) |
| **`constraints.mk`** | Build-time constraints checking |
| **`tbbr/tbbr_tools.mk`** | TBBR-specific build helpers |
| **`toolchains/`** | Toolchain-specific configs |
| - `aarch32.mk` | AArch32 toolchain (arm-none-eabi-) |
| - `aarch64.mk` | AArch64 toolchain (aarch64-linux-gnu-) |
| - `host.mk` | Host tools (for fiptool, cert_create) |
| - `rk3399-m0.mk` | Special M0 firmware for Rockchip |

---

### 7.3 Build Variables

**Platform Selection:**
```bash
make PLAT=fvp            # Build for FVP platform
make PLAT=stm32mp257    # Build for STM32MP257
```

**Architecture:**
```bash
make ARCH=aarch64        # 64-bit (default for most platforms)
make ARCH=aarch32        # 32-bit
```

**Build Type:**
```bash
make DEBUG=1            # Debug build (with symbols, no optimization)
make DEBUG=0            # Release build (default, optimized)
make BUILD=production   # Production (no assertions)
```

**Secure Payload:**
```bash
make SPD=opteed         # OP-TEE (default for many platforms)
make SPD=tspd           # Trusted Services Payload
make SPD=tlkd           # Trusted Logic
make SPD=spmin          # SP-MIN
make SPD=none           # No BL32
```

** Firmware Configuration:**
```bash
make FIRMWARE_CONFIG=fw-conf-img   # Dynamic config via fconf
```

**Other Key Variables:**
- `BL33` - Path to BL33 image (U-Boot/Linux)
- `GENERATE_COT` - Generate Chain of Trust (TBBR)
- `ENABLE_FEAT_<FEATURE>` - Enable architecture features (e.g., `ENABLE_FEAT_RME`)
- `MBEDTLS_*` - mbedTLS configuration flags
- `TRUSTED_BOARD_BOOT` - Enable TBBR
- `GUIDED_BOOT` - Guided boot flow

---

### 7.4 Build Outputs

**Output Directory:** `build/<platform>/`

```
build/fvp/
├── bl1/
│   ├── bl1.elf           # ELF image
│   └── bl1.bin           # Raw binary
├── bl2/
│   ├── bl2.elf
│   └── bl2.bin
├── bl31/
│   ├── bl31.elf
│   └── bl31.bin
├── bl32/
│   ├── sp_min/
│   │   ├── sp_min.elf
│   │   └── sp_min.bin
│   └── tsp/
│       ├── tsp.elf
│       └── tsp.bin
├── fip/                   # Firmware Image Package
│   ├── fip.bin           # Full FIP with all images
│   ├── fip_unsigned.bin  # Without signatures
│   └── [individual .bin files]
├── tools/                 # Built host tools
│   ├── fiptool
│   ├── cert_create
│   └── sptool
├── .d/                    # Dependency files
└── .map/                  # Linker map files
```

**FIP (Firmware Image Package):**
- Container format for all bootloader images
- Contains: BL2, BL31, BL32, BL33 (optional), certificates, FWU metadata
- Created by `fiptool`
- Flashed as single image to storage

---

### 7.5 Platform Makefile Inclusion

Platform makefiles are **automatically discovered** and loaded:

```makefile
# In top-level Makefile:
PLAT_MK_FILE := $(call select_platform_mk, $(PLAT))
ifeq ($(PLAT_MK_FILE),)
    $(error Unknown platform $(PLAT))
endif
include $(PLAT_MK_FILE)

# Platform makefile can include:
# - Platform defaults
# - Platform-specific sources
# - Feature detection
# - Toolchain adjustments
```

---

### 7.6 Configuration System

TF-A uses **multiple configuration mechanisms**:

1. **Make Variables** - Direct control from command line
2. **Dynamic Configuration (fconf)** - Runtime-configurable parameters via FDT
3. **Platform Defaults** - `platform_defaults.mk` per platform
4. **Header Configuration** - `#define` in platform-specific headers
5. **Feature Detection** - Compile-time CPU feature detection

---

## 8. Tools & Utilities

**Path:** `tools/`

Host-side utilities for building, signing, packaging, and analyzing TF-A images.

### 8.1 Core Build Tools

#### **FIPTool**

**Path:** `tools/fiptool/`

**Purpose:** Create, inspect, modify FIP (Firmware Image Package) files.

```
tools/fiptool/
├── fiptool.c           (32,152 bytes - main)
├── fiptool.h
├── tbbr_config.c       # TBBR config parsing
├── win_posix.c/h       # Cross-platform I/O
└── plat_fiptool/       # Platform-specific fiptool extensions
    ├── nxp/
    │   ├── s32/
    │   │   └── s32g274ardb2/
    │   └── plat_fiptool.mk
    ├── arm/
    │   ├── board/
    │   │   ├── tc/
    │   │   ├── juno/
    │   │   └── ...
    ├── st/
    └── ...
```

**Usage:**
```bash
fiptool create fip.bin \
    --tb-fw=bl2.bin \
    --fw-config=fw_config.img \
    --bl31=bl31.bin \
    --bl32=bl32.bin \
    --bl33=uboot.fit \
    --cert=chain.pem

fiptool info fip.bin          # Inspect
fiptool unpack fip.bin out/   # Extract contents
fiptool update fip.bin --bl31=new_bl31.bin
```

**Build:** `make fiptool` (host tool, built with system compiler)

---

#### **Certificate Creation Tool**

**Path:** `tools/cert_create/`

**Purpose:** Generate X.509 certificates for TBBR secure boot chain.

```
tools/cert_create/
├── src/
│   ├── main.c          (14,042 bytes - entry point)
│   ├── cert.c          # Certificate creation
│   ├── key.c           # Key handling
│   ├── ext.c           # X.509 extensions
│   ├── sha.c           # SHA hashing
│   ├── cmd_opt.c       # Command-line options
│   ├── c
│   └── ...
├── include/
│   └── tbbr/           # TBBR certificate structures
│       └── tbbr_cert.h
└── Makefile
```

**Usage:**
```bash
cert_create --key key.pem --cert cert.pem \
    --fw-config fw_config.img \
    --hw-config hw_config.der
```

**Chain of Trust:** Codemon key → OEM key → Key certificate → Content certificate

**Build:** `make certtool`

---

#### **SPTool**

**Path:** `tools/sptool/`

**Purpose:** Package Secure Partition (SP) images for SPM.

```
tools/sptool/
├── sptool.py           # Main executable
├── sp_mk_generator.py  # Makefile generator
├── hob.py              # HOB (Handoff Block) handling
├── spactions.py        # SP actions
├── pyproject.toml
└── poetry.lock
```

**Usage:**
```bash
sptool package --sp sp.elf --out sp.pkg
```

**Build:** Python tool (Poetry-managed)

---

### 8.2 Platform-Specific Tools

#### **Memory Analyzer**

**Path:** `tools/memory/`

**Purpose:** Parse linker scripts and ELF files to generate memory map summary.

```
tools/memory/
├── src/memory/
│   ├── mapparser.py    # Parse linker map files
│   ├── summary.py      # Generate summary
│   ├── image.py        # Image analysis
│   ├── printer.py      # Output formatting
│   ├── elfparser.py    # ELF parsing
│   └── memmap.py
├── pyproject.toml
└── poetry.lock
```

**Usage:** `python3 -m memory [options]`

---

#### **Other Platform Tools**

```
tools/
├── amlogic/           # Amlogic platform utilities
├── marvell/           # Marvell image tools
├── nxp/               # NXP utilities
│   ├── create_pbl/    # PBL (Primary Boot Loader) generation
│   └── [other tools]
├── qti/               # Qualcomm tools
├── renesas/           # Renesas utilities
│   ├── cert/
│   ├── rcar_defconfig/
│   └── rcar_signedimage.py
├── stm32image/        # STM32 image packaging
└── tlc/               # TLC linker config tool
```

---

### 8.3 Development Tools

#### **DT to C Converter**

**Path:** `tools/cot_dt2c/`

Convert Device Tree blobs to C structures for inclusion.

```
tools/cot_dt2c/
├── __main__.py
├── cot_dt2c.py
├── tests/
│   └── test_util.py
├── pyproject.toml
└── poetry.lock
```

---

#### **Changelog Generator**

**Path:** `tools/conventional-changelog-tf-a/`

Generates changelog from conventional commit messages.

---

#### **Libraries (contrib/)**

```
contrib/
├── libeventlog/       # Event logging library
├── libtl/             # TL library
├── libtpm/            # TPM support
└── mbed-tls/          # mbedTLS (git submodule - may be empty if not initialized)
```

---

### 8.4 Toolchain Requirements

**Host Tools:**
- Python 3.x + Poetry (for Python tools)
- Node.js (for changelog tools, optional)
- GCC/Clang (for C host tools: fiptool, cert_create)

**Target Toolchains:**
- `arm-none-eabi-` (GCC for AArch32)
- `aarch64-none-elf-` (GCC for AArch64 bare-metal)
- `clang` (LLVM/Clang)
- `armclang` (ARM Compiler 6 - optional)

**Toolchain Selection:**
- `CROSS_COMPILE` environment variable
- Or auto-detected by build system

---

## 9. Documentation

**Path:** `docs/`

**Format:** reStructuredText (`.rst`) + Sphinx

**Total Files:** 659+ documentation files

### 9.1 Documentation Structure

```
docs/
├── _static/
│   └── css/                      # Custom stylesheets
├── about/                        # About TF-A
├── build/                        # Build documentation
│   ├── doctrees/                  # Sphinx intermediate
│   └── html/                     # HTML output
├── components/                   # Component docs
│   ├── fconf/
│   ├── measured_boot/
│   └── spd/                      # Secure Partition docs
├── design/                       # Design documents
├── design_documents/             # Architecture designs
│   ├── bl1-design.rst
│   ├── bl2-design.rst
│   ├── bl31-design.rst
│   ├── bl32-design.rst
│   ├── firmware-update-design.rst
│   └── ...
├── getting_started/              # Getting started guides
├── perf/                        # Performance docs
├── plat/                         # Platform-specific docs
│   ├── arm/
│   ├── marvell/
│   ├── nxp/
│   ├── qti/
│   ├── st/
│   └── ...
├── process/                     # Development process
│   ├── contributing.rst
│   ├── code-review.rst
│   ├── commit-log.rst
│   └── release-process.rst
├── resources/                   # Additional resources
│   └── diagrams/               # Architecture diagrams
├── security_advisories/
├── threat_model/
│   └── firmware_threat_model/
└── tools/                       # Tool documentation
```

---

### 9.2 Key Documentation Files

**Root Level:**

| File | Description |
|------|-------------|
| `index.rst` | Documentation homepage |
| `glossary.rst` | Glossary of TF-A terms (BL1, BL2, SPD, SPM, etc.) |
| `architecture_features.rst` | **Comprehensive feature guide** (99KB) |
| `porting-guide.rst` | **Porting guide** (162KB) - essential for new platforms |
| `change-log.md` | **Full changelog** (991KB - massive!) |
| `security-policy.rst` | Security policy |
| `license.rst` | License information (BSD-3-Clause) |

**Design Documents** (`design_documents/`):

| Document | Purpose |
|----------|---------|
| `bl1-design.rst` | BL1 architecture |
| `bl2-design.rst` | BL2 architecture, image loading, auth |
| `bl31-design.rst` | EL3 runtime, services |
| `bl32-design.rst` | Secure payload design (SP-MIN, TSP) |
| `firmware-update-design.rst` | FWU mechanism |
| `measured_boot-design.rst` | Measured boot |
| `dynamic_configuration.rst` | fconf framework |
| `psci-design.rst` | PSCI design |
| `spm-design.rst` | SPM/SPMD architecture |
| `tbbr-design.rst` | TBBR trusted boot |
| `rme-design.rst` | Realm Management Extension |

---

### 9.3 Building Documentation

**Requirements:**
- Sphinx
- Doxygen (for API docs, optional)
- Sphinx theme (custom TF-A theme)

**Build:**
```bash
make doc           # Build HTML docs
make doc-clean     # Clean docs
```

**Output:** `docs/build/html/` (local web server at `http://localhost:8000`)

**ReadTheDocs:** `.readthedocs.yaml` config auto-builds on rtfd.io

---

### 9.4 API Documentation

- **Public API:** Exported via `include/export/`
- **Internal API:** Stage-specific `*_private.h` headers
- **Doxygen** config possibly in `docs/` (check `Doxyfile` or Sphinx extensions)

---

## 10. Device Tree & Configuration

### 10.1 Flattened Device Tree (`fdts/`)

**Path:** `fdts/`

**Total DT Files:** 186 (`.dts`, `.dtsi`)

**Purpose:** Device Tree sources for TF-A platforms. Used for:
- Platform description (CPU, memory, peripherals)
- Firmware configuration (fconf)
- SoC-level descriptions

**Layout:**
```
fdts/
├── fvp-base-*.dts                 # ARM FVP platforms
├── fvp-ve-Cortex-A5x1.dts
├── juno-*.dts                     # Juno boards
├── morello-*.dts                  # Morello
├── stm32mp157c-dk2.dts            # STM32MP157 Discovery Kit
├── stm32mp235f-dk-fw-config.dts   # STM32MP2 fw-config
├── n1sdp-multi-chip.dts           # Neoverse N1SDP
├── tc4.dts                        # Total Compute
├── rdaspen.dts                    # Qualcomm automotive
├── imx8mp-*.dts                   # NXP i.MX
├── rk3399-*.dts                   # Rockchip
└── [many more platform DT files]
```

**DT Include Hierarchy:**
```
platform.dts
├── include "soc.dtsi"             # SoC-level definition
├── include "board.dtsi"           # Board-level
├── include "fw-config.dtsi"       # Firmware config
└── [TF-A specific nodes]
```

**DT Bindings:** Standard bindings in `include/dt-bindings/`

---

### 10.2 Firmware Configuration (fconf)

**Path:** `lib/fconf/`

**Framework:** `fconf = Firmware CONFiguration`

**Purpose:** Dynamic, runtime-configurable parameters without recompilation.

**Mechanism:**
1. Configuration defined in Device Tree (`.dts`) under `/firmware` node
2. `fconf` library parses DT at runtime (via `libfdt`)
3. Populates configuration structures accessible by firmware

**Configuration Types:**
- **BL31 config:** Power states, CPU topology, interrupt routing
- **BL32 config:** Memory regions, partition definitions
- **Platform config:** Hardware addresses, clock frequencies
- **Security config:** Key IDs, certificate chain

**Example DT Node:**
```dts
/firmware {
    bl31 {
        warmboot_entrypoint = <0x...>;
        cpu_on = <...>;
    };
    bl32 {
        memory_region = <...>;
    };
};
```

**Implementation:** `lib/fconf/fconf_dyn_cfg.c` parses DT and fills config structs.

---

## 11. Code Quality & CI/CD

### 11.1 Code Style & Checks

**Configuration Files:**
```
.checkpatch.conf   # Linux kernel coding style enforcement
.clang-format      # Clang-format code style
.editorconfig      # EditorConfig for consistent formatting
```

**Tools:**
- `checkpatch.pl` - Kernel style checker (run before commit)
- `clang-format` - Auto-formatting

---

### 11.2 Commit Standards

**Commitizen:** Conventional Commits

```json
// .cz.json
{
  "types": ["feat", "fix", "docs", "style", "refactor", "perf", "test", "chore", "revert"]
}
```

**Examples:**
```
feat(bl31): add support for GICv4.1
fix(drivers): correct UART baud rate calculation
docs(porting): update memory map example
```

**Commit Linting:** `.commitlintrc.js` validates commit messages.

**DCO (Developer's Certificate of Origin):** `dco.txt` - sign-off required.

---

### 11.3 Version Management

**Tools:**
- `.versionrc.cjs` - Versions configuration (for commitizen/release)
- Standard Version or similar for automated version bumps

**Version Scheme:** `MAJOR.MINOR.PATCH` (e.g., 2.14.0)

---

### 11.4 GitHub CI/CD

**Path:** `.github/`

```
.github/
├── CODEOWNERS       # Code ownership for review
└── dependabot.yml   # Dependency updates (Python, Node)
```

**Git Hooks:** `.husky/`
- `pre-commit` - Run lint/format checks
- `commit-msg` - Validate commit message format

**Git Review:** `.gitreview` - Gerrit integration (legacy, for Arm internal)

---

### 11.5 License

**Primary License:** BSD-3-Clause (see license headers in source files)

**Third-Party Licenses:** `licenses/` directory
- `LICENSE-APACHE-2.0.txt`
- `LICENSE.MIT`

**Compliance:** Each source file has SPDX-like header with copyright and license.

---

## 12. Architectural Patterns

TF-A demonstrates several **well-known firmware patterns**:

---

### 12.1 Multi-Stage Bootloader

**Pattern:** Divide boot process into stages with increasing capability.

**TF-A Implementation:**
```
Stage 0: ROM (chip vendor)
    ↓
Stage 1: BL1 (minimal init, FWU metadata)
    ↓
Stage 2: BL2 (load & authenticate all images)
    ↓
Stage 3: BL31 (EL3 runtime, power, services)
    ↓
Stage 4: BL32 (secure OS/TEE) ← optional
    ↓
Stage 5: BL33 (non-secure bootloader: U-Boot)
    ↓
Stage 6: Linux/Android
```

**Rationale:**
- Each stage fits in limited on-chip SRAM
- Smaller attack surface per stage
- Separation of concerns (auth vs runtime vs secure OS)

---

### 12.2 Platform Abstraction Layer

**Pattern:** Isolate hardware-specific code behind defined interfaces.

**TF-A Implementation:**
```
Platform Interface Contracts:
- bl1_platform_setup()
- bl2_platform_setup()
- bl31_platform_setup()
- bl32_platform_setup()
- plat_*() APIs for drivers, IO, etc.

Platform Directory Structure:
plat/<vendor>/<soc>/<board>/
  ├── platform.mk           # Build
  └── [sources, headers]    # Implementation
```

**Benefit:** Generic code (`bl31/`, `services/`) calls platform APIs without knowing hardware.

---

### 12.3 Driver Model

**Pattern:** Vendor drivers isolated; common APIs defined in headers.

```
Driver Registration:
1. Driver implements defined API (init, read, write, etc.)
2. Platform includes driver .mk file in platform.mk
3. Driver init called at appropriate boot stage
```

**Example (UART):**
```c
// include/drivers/arm/pl011.h
typedef struct {
    unsigned int base;
    unsigned int baud_rate;
} pl011_data_t;

int pl011_initialize(pl011_data_t *data);
int pl011_putc(int c);
```

---

### 12.4 Service Dispatcher

**Pattern:** Runtime services registered via function table, dispatched by SMC handler.

```
SMC Call → SMC Dispatcher → Service Lookup (UUID) → Service Handler
                                   ↓
                          Services: PSCI, SDEI, SPM, etc.
```

**Advantages:**
- Services can be added dynamically
- Standardized interface (SMC calling convention)
- Multiple services coexisting

---

### 12.5 Configuration via Device Tree

**Pattern:** Hardware description external to binary.

**Usage:**
- Platform DT files in `fdts/`
- `libfdt` parses DT at runtime
- `fconf` extracts firmware configuration
- Platform queries config structures

**Advantage:** Same binary works across board variants with different DT.

---

### 12.6 Optional Feature Flags

**Pattern:** Compile-time feature selection via make variables and CPU feature detection.

**Example:**
```makefile
ifeq ($(ENABLE_FEAT_RME),1)
    include lib/gpt_rme/gpt_rme.mk
    BL31_SOURCES += lib/gpt_rme/gpt_rme.c
endif
```

**Features:** PAuth, SVE, SME, RAS, FGT, MPAM, etc. (detected at runtime but built optionally)

---

### 12.7 Firmware Image Package (FIP)

**Pattern:** Monolithic container for multiple firmware images.

**Purpose:**
- Single image to flash to storage
- Signed as unit (TBBR)
- Contains all bootloader stages and config

**Format:**
```
+---------------------+
| FIP Header          |  (uuid, name, offset, size)
+---------------------+
| Certificate Chain   |
+---------------------+
| BL2 image           |
+---------------------+
| FW Config           |
+---------------------+
| BL31 image          |
+---------------------+
| BL32 image (SP-MIN) |
+---------------------+
| BL33 (U-Boot)       |
+---------------------+
```

---

### 12.8 Trusted Boot Chain

**Pattern:** Cryptographically signed images, certificate chain validation.

**TBBR Flow:**
```
1. BL1 verifies BL2 using embedded keys (ROM)
2. BL2 verifies BL31, BL32 using certificate chain
3. Certificates signed by OEM key (in BL2)
4. Anti-rollback via version numbers
5. Measurements logged (measured boot optional)
```

**Keys:** 
- HW_KEY (manufacturer, in ROM)
- ROTPK (RoT public key, in cert)
- OEM key (signs certificates)

---

## 13. Quick Reference

### 13.1 Directory Purpose Summary

| Directory | Purpose | Key Contents |
|-----------|---------|--------------|
| `bl1/` | First stage bootloader | Entry point, FWU metadata, TBBR image desc |
| `bl2/` | Second stage bootloader | Image loading, auth, dispatch |
| `bl2u/` | FWU-specific BL2 | Unprivileged update mode |
| `bl31/` | EL3 runtime | PSCI, SDEI, SPM, traps, interrupts |
| `bl32/` | Secure payload | SP-MIN, TSP, OP-TEE |
| `plat/` | Platform code | Vendor/SoC/board directories, `platform.mk` |
| `drivers/` | Device drivers | Vendor-specific, common drivers |
| `services/` | Runtime services | SPM, SDEI, PSCI, SPDs |
| `common/` | Shared code | Architecture-agnostic utilities |
| `lib/` | Reusable libs | PSCI, xlat_tables, extensions |
| `include/` | Public headers | API for consumers |
| `tools/` | Host utilities | fiptool, cert_create, sptool |
| `docs/` | Documentation | Sphinx RST docs |
| `fdts/` | Device trees | Platform .dts files |
| `make_helpers/` | Build modules | Makefile includes |
| `contrib/` | Third-party | External libraries (mbedTLS, etc.) |
| `licenses/` | License files | Apache, MIT, BSD |

---

### 13.2 File Extensions & Their Meaning

| Extension | Meaning |
|-----------|---------|
| `.c` | C source file |
| `.S` | ARM assembly (preprocessed) |
| `.h` | C header |
| `.ld.S` | Linker script (preprocessed) |
| `.mk` | Makefile fragment |
| `.dts` | Device Tree source |
| `.dtsi` | Device Tree include |
| `.rst` | reStructuredText (Sphinx) |
| `.md` | Markdown |
| `.py` | Python script |
| `.json` | JSON config (commitizen, package deps) |
| `.yaml`/`.yml` | YAML config (changelog, CI/CD) |

---

### 13.3 Common Make Variables

**Build Control:**
| Variable | Values | Default | Purpose |
|----------|--------|---------|---------|
| `PLAT` | platform name | `fvp` | Target platform |
| `ARCH` | `aarch64`, `aarch32` | platform | Target architecture |
| `DEBUG` | `0`, `1` | `0` | Debug vs release |
| `BUILD` | `debug`, `release`, `production` | `release` | Build variant |
| `SPD` | `spmin`, `opteed`, `tspd`, `tlkd`, `trusty`, `none` | per-platform | Secure payload |
| `BL33` | path to image | (optional) | Non-secure bootloader |

**TBBR/Boot:**
| Variable | Values | Purpose |
|----------|--------|---------|
| `GUIDED_BOOT` | `0`/`1` | Use guided boot flow (TBBR) |
| `TRUSTED_BOARD_BOOT` | `0`/`1` | Enable trusted boot |
| `GENERATE_COT` | `0`/`1` | Generate chain of trust |

**Crypto:**
| Variable | Values | Purpose |
|----------|--------|---------|
| `PSA_CRYPTO` | `0`/`1` | PSA Crypto API |
| `CRYPTOGRAPHY` | `0`/`1` | Generic crypto (legacy) |
| `MBEDTLS_*` | varies | mbedTLS configuration |

**Architecture Features:**
| Variable | Values | Purpose |
|----------|--------|---------|
| `ENABLE_FEAT_PAUTH` | `0`/`1` | Pointer Authentication |
| `ENABLE_FEAT_SVE` | `0`/`1` | Scalable Vector Extension |
| `ENABLE_FEAT_SME` | `0`/`1` | Scalable Matrix Extension |
| `ENABLE_FEAT_RME` | `0`/`1` | Realm Management Extension |
| `ENABLE_FEAT_RAS` | `0`/`1` | Reliability & Serviceability |

---

### 13.4 Common Build Commands

```bash
# Build everything for platform (default: FVP)
make PLAT=fvp

# Build specific stage
make BL31

# Clean build artifacts
make clean

# Clean everything including tools
make distclean

# Build documentation
make doc

# Build FIP only (assumes binaries exist)
make fip

# Build firmware update FIP
make fwu_fip

# Build SP package (Secure Partition)
make sp

# Build host tools
make fiptool
make certtool
make sptool

# Run tests (if platform supports)
make check

# Print configuration
make print-config

# Show all available platforms
make list-plats
```

---

### 13.5 Platform Selection Examples

```bash
# ARM FVP (reference)
make PLAT=fvp

# ARM Juno
make PLAT=juno

# STM32MP157C Discovery Kit
make PLAT=stm32mp257_dk

# NXP i.MX8MP EVK
make PLAT=imx8mp_evk

# Raspberry Pi 4
make PLAT=rpi4

# NXP Layerscape LS1048A RDB
make PLAT=ls1048ardb

# Qualcomm RB3Gen2
make PLAT=qualcomm_ rb3gen2
```

---

### 13.6 File Key Patterns

| Pattern | Meaning | Example |
|---------|---------|---------|
| `bl*` | Bootloader stage | `bl1_main.c`, `bl2_image_load.c` |
| `plat_*` | Platform-specific | `plat_gicv3.c` |
| `arm_*` | ARM common code | `arm_bl31_setup.c` |
| `*_private.h` | Internal header (not exported) | `bl31_private.h` |
| `*_ld.S` | Linker script | `bl31.ld.S` |
| `*_mk` | Build fragment | `imx8mm.mk` |
| `.dts` | Device Tree source | `fvp-base-aemv8r.dts` |
| `.dtsi` | Device Tree include | `fvp-common.dtsi` |
| `Makefile` | Build entry | Top-level or platform |
| `platform.mk` | Platform definition (required) | `plat/arm/board/fvp/platform.mk` |

---

### 13.7 Common Error Patterns

| Error | Likely Cause | Fix |
|-------|--------------|-----|
| `Unknown platform` | `PLAT=` undefined or wrong | `make list-plats` to see available |
| `No rule to make target` | Missing dependencies (e.g., `libfdt`) | `make prepare` or init submodules |
| `undefined reference` | Linking error, missing object | Check `platform.mk` includes all sources |
| `fatal error: xxx.h: No such file` | Missing include path | Check `INCLUDES` in `platform.mk` |
| `incompatible pointer type` | 32/64-bit mismatch | Ensure ARCH matches platform expectation |
| `missing required symbol` | Linker script issue | Check `bl*_ld.S` memory regions |

---

## 14. Getting Started for New Repository

If creating a **new firmware repository** inspired by TF-A:

1. **Analyze** your platform requirements (single-stage vs multi-stage)
2. **Adopt** directory conventions: `bl1/`, `bl2/`, `bl31/`, `bl32/`, `plat/`, `drivers/`, `lib/`, `include/`
3. **Port** platform code: create `plat/<your_vendor>/<soc>/<board>/platform.mk`
4. **Integrate** necessary drivers from TF-A (or write new ones)
5. **Configure** build system (Makefile + make_helpers)
6. **Set up** CI/CD with linting, commit hooks, automated builds
7. **Document** design and porting guide for maintainers

---

## Appendix

### A. Supported Platforms Summary

**110+ platforms across 33+ vendors** including:

- **ARM**: FVP, Juno, Morello, N1SDP, Neoverse RD, TC, Corstone
- **NXP**: i.MX7/8M/8Q/8ULP/9, Layerscape LS1028A/LS1043A/LS1046A/LS1088A/LX2160A
- **ST**: STM32MP1, STM32MP2
- **Qualcomm**: RB3Gen2, SC7280, Lemans, QCS615, MSM8916/39/8909, MDM9607
- **MediaTek**: MT8173/MT8183/MT8186/MT8188/MT8192/MT8195
- **Rockchip**: RK3288/RK3328/RK3368/RK3399/RK3568/RK3588
- **Raspberry Pi**: RPi3, RPi4, RPi5
- **Renesas**: R-Car Gen3/4/5, RZ/A, RZ/G
- **Xilinx**: ZynqMP, Versal, Versal Net
- **TI**: K3, K3Low
- **Marvell**: Armada, OcteonTX
- **AMD**: Versal2
- **Intel**: Agilex, Agilex5, N5X, Stratix10
- **Broadcom**: Stingray
- **Hisilicon**: HiKey, HiKey960, Poplar
- **AllWinner**: A64, H6, H616, R329
- **Amlogic**: AXG, G12A, GXBB, GXL
- **NVIDIA**: Tegra
- **Nuvoton**: NPCM845X
- **Aspeed**: AST2700
- **QEMU**: Qemu, Qemu SBSA

---

### B. License & Contribution

**License:** BSD-3-Clause  
**Contributing:** Follow `docs/process/contributing.rst`  
**Code Review:** Use Gerrit or GitHub PRs  
**DCO:** Developer's Certificate of Origin required  
**Coding Style:** Linux kernel style enforced by `checkpatch.pl`

---

## Conclusion

TF-A is a **mature, production-grade** secure firmware implementation with:

- **Clear separation** between bootloader stages
- **Excellent abstraction** via platform layer
- **Extensive vendor support** (110+ platforms)
- **Comprehensive security** (TBBR, measured boot, FWU, RME)
- **Active development** with CI/CD, code review, strict standards
- **Well-documented** design, porting guides, API docs
- **Modular libraries** enabling reuse across stages
- **Flexible build system** supporting myriad configurations

**Best Practices Demonstrated:**
1. Separation of concerns (stages, drivers, services)
2. Platform abstraction with minimal impact on generic code
3. Header file organization (public API vs internal)
4. Configuration via Make + DT + fconf
5. Security-first design (trusted boot, attestation)
6. Comprehensive testing & CI infrastructure
7. Professional documentation & porting guides

**Use This Analysis** to inform architecture of new firmware projects, understand industry best practices for bootloader design, or evaluate open-source firmware quality.

---

**End of Analysis**

*Generated from TF-A repository at commit analysis on 2026-04-25*
