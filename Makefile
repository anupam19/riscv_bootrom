include config.mk

SRCS := $(wildcard src/*.c src/*.S plat/generic/*.c drivers/uart/*.c)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(filter %.c,$(SRCS)))
OBJS += $(patsubst %.S,$(BUILD_DIR)/%.o,$(filter %.S,$(SRCS)))

all: $(BUILD_DIR)/$(ELF) $(BUILD_DIR)/$(BIN)

$(BUILD_DIR)/$(ELF): $(OBJS) linker.ld
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/$(BIN): $(BUILD_DIR)/$(ELF)
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/src $(BUILD_DIR)/plat/generic $(BUILD_DIR)/drivers/uart

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
