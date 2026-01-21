export TOP_DIR=$(shell pwd)
export BUILD_DIR=$(TOP_DIR)/build

LIB_DIR := $(TOP_DIR)/lib
SRC_DIR := $(TOP_DIR)/src

INCDIRS := . $(TOP_DIR)/include
SRCDIRS=$(SRC_DIR)

SRC := $(wildcard $(addsuffix /*.c,$(SRCDIRS)))
OBJ := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))
DEPS := $(patsubst %.c,$(BUILD_DIR)/%.d,$(notdir $(SRC)))

BEAR_OPTS :=--output $(TOP_DIR)/compile_commands.json
BEAR := bear $(BEAR_OPTS) --

CC := clang

LDLIBS := 
LDFLAGS := -fuse-ld=mold

DEPFLAGS := -MP -MD
COMPILE_SANITY_OPTS := -Wall -Wextra
CPPFLAGS := $(foreach D,$(INCDIRS),-I$(D))
CFLAGS := -g $(DEPFLAGS)

FILE := $(TOP_DIR)/tmp
ARGS := $(FILE)

PROGRAM := $(TOP_DIR)/main

vpath %.c $(SRCDIRS)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(OBJ): $(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

run:
	$(PROGRAM) $(ARGS)

clean:
	rm -f -- $(BUILD_DIR)/*

bear: clean
	@$(BEAR) $(MAKE) -C .

.PHONY: all run clean bear

-include $(DEPS)
