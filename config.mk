TOOLCHAIN_PREFIX ?= riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
AS := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
SIZE := $(TOOLCHAIN_PREFIX)size

# Default ISA & ABI (override on command line: make MARCH=rv32emac_zicsr MABI=ilp32)
MARCH ?= rv64imac_zicsr
MABI  ?= lp64

CFLAGS := -ffreestanding -nostdlib -march=$(MARCH) -mabi=$(MABI) \
          -O2 -Wall -Wextra \
          -Iinclude -I$(abspath plat/generic)

ASFLAGS := -march=$(MARCH) -mabi=$(MABI)

LDFLAGS := -T linker.ld -nostdlib

TARGET := bootrom
ELF := $(TARGET).elf
BIN := $(TARGET).bin

SRC_DIR := src
BUILD_DIR := build
