# Makefile for test payload
# Uses same RISC-V cross-compilation toolchain as BootROM (override via TOOLCHAIN_PREFIX)

TOOLCHAIN_PREFIX ?= riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy

# ISA and ABI (match BootROM defaults)
MARCH ?= rv64imac_zicsr
MABI  ?= lp64

 CFLAGS := -ffreestanding -nostdlib -march=$(MARCH) -mabi=$(MABI) \
           -O2 -Wall -Wextra \
           -fno-pic -no-pie \
           -I../../include -I../../drivers/timer -I../../drivers/plic -I../../plat/generic

LDFLAGS := -T payload.ld -nostdlib

TARGET := payload
ELF := $(TARGET).elf
BIN := $(TARGET).bin

SRC := payload.S
OBJ := $(SRC:.S=.o)

.PHONY: all clean

all: $(ELF) $(BIN)

$(ELF): $(OBJ) payload.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(ELF) $(BIN)
