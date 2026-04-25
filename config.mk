TOOLCHAIN_PREFIX ?= riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
AS := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
SIZE := $(TOOLCHAIN_PREFIX)size

CFLAGS := -ffreestanding -nostdlib -march=rv64imac_zicsr -mabi=lp64 \
          -O2 -Wall -Wextra \
          -Iinclude -I$(abspath plat/generic)

ASFLAGS := -march=rv64imac_zicsr -mabi=lp64

LDFLAGS := -T linker.ld -nostdlib

TARGET := bootrom
ELF := $(TARGET).elf
BIN := $(TARGET).bin

SRC_DIR := src
BUILD_DIR := build
