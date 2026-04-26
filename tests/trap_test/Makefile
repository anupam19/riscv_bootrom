# Makefile for trap test payload
# Builds a payload that triggers an ECALL to verify BootROM trap handling

TOOLCHAIN_PREFIX ?= riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy

MARCH ?= rv64imac_zicsr
MABI  ?= lp64

CFLAGS := -ffreestanding -nostdlib -march=$(MARCH) -mabi=$(MABI) \
          -O2 -Wall -Wextra \
          -I../../include -I../../drivers/timer -I../../drivers/plic -I../../plat/generic

LDFLAGS := -T trap_payload.ld -nostdlib

TARGET := trap_payload
ELF := $(TARGET).elf
BIN := $(TARGET).bin

SRC := trap_payload.c
OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: $(ELF) $(BIN)

$(ELF): $(OBJ) trap_payload.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(ELF) $(BIN)
