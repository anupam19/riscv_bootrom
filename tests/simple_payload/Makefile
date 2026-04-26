# Makefile for simple test payload
TOOLCHAIN_PREFIX ?= riscv64-unknown-elf-
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld
OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy

MARCH ?= rv64imac_zicsr
MABI  ?= lp64

CFLAGS := -ffreestanding -nostdlib -march=$(MARCH) -mabi=$(MABI) -O2 -Wall -Wextra

LDFLAGS := -T simple_payload.ld -nostdlib

TARGET := simple_payload
ELF := $(TARGET).elf
BIN := $(TARGET).bin

SRC := simple_payload.c
OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: $(ELF) $(BIN)

$(ELF): $(OBJ) simple_payload.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJ)

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(ELF) $(BIN)
