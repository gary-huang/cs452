XCC = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/gcc
AS  = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/as
LD  = /u/wbcowan/gnuarm-4.0.2/arm-elf/bin/ld
CP = /bin/cp

ARM_DIR ?= /u/cs452/tftp/ARM
BUILD_DIR ?= build
CONF_DIR  ?= conf
DOCS_DIR  ?= docs
TEST_DIR  ?= test

SRC_DIRS ?= src include
MD5_OUT ?= $(BUILD_DIR)/md5_hashes.txt

TARGET ?= a1
TARGET_ELF ?= $(TARGET).elf
TARGET_MAP ?= $(BUILD_DIR)/$(TARGET).map
TEST_EXEC ?= tests

LINKER_SCRIPT ?= $(CONF_DIR)/orex.ld

SRCS := $(shell find $(SRC_DIRS) -name *.c)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
TEST_SRCS := $(shell find $(TEST_DIR) -name *.c) $(SRCS)
TEST_OBJS := $(TEST_SRCS:%=$(BUILD_DIR)/%.o) $(OBJS)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CFLAGS = -c -fPIC -Wall -I. $(INC_FLAGS) -mcpu=arm920t -msoft-float -MMD -MP $(DFLAGS)
LDFLAGS = -init main -Map $(TARGET_MAP) -N -T $(LINKER_SCRIPT) -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2 -L./lib

all: $(BUILD_DIR)/$(TARGET_ELF)

test: $(BUILD_DIR)/$(TEST_EXEC)

$(BUILD_DIR)/$(TEST_EXEC): $(TEST_OBJS)
	$(XCC) $(INC_FLAGS) $(TEST_OBJS) -o $@

$(BUILD_DIR)/$(TARGET_ELF): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) -lgcc

$(BUILD_DIR)/%.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD_DIR)/%.c.s: %.c
	$(MKDIR_P) $(dir $@)
	$(XCC) -S $(CFLAGS) $< -o $@

.PRECIOUS: $(BUILD_DIR)/%.c.s

.PHONY: clean hashes docs test

hashes:
	$(MKDIR_P) $(BUILD_DIR)
	$(shell find -regex ".*\.[c|h]" -type f -exec md5sum '{}' \; > $(MD5_OUT))

docs: $(addsuffix .pdf, $(basename $(wildcard $(DOCS_DIR)/*.md)))

$(DOCS_DIR)/%.pdf: $(DOCS_DIR)/%.md
	$(PANDOC) $(PANDOC_FLAGS) $< -o $@

clean:
	$(RM) -r $(BUILD_DIR)

copy:
	$(MAKE) clean && $(MAKE) && $(CP) $(BUILD_DIR)/$(TARGET_ELF) $(ARM_DIR)/$(USER)

-include $(DEPS)

MKDIR_P ?= mkdir -p
PANDOC = pandoc
PANDOC_FLAGS=--metadata date="`date +'%b %d, %Y'`"
