CC       ?= gcc
AR       ?= ar
STD      ?= c11

CFLAGS   ?= -g -O1
CFLAGS   += -std=$(STD) -Wall -Wextra -MMD -MP
CPPFLAGS += -I$(SRC_DIR)
LDFLAGS  ?=
LDLIBS   ?=

ifdef SANITIZE
CFLAGS  += -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
endif

BUILD_DIR := build
SRC_DIR   := src
TEST_DIR  := tests

LIB       := $(BUILD_DIR)/libargparse.a
LIB_SRCS  := $(SRC_DIR)/argparse.c
LIB_OBJS  := $(LIB_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

DEMO      := $(BUILD_DIR)/main
DEMO_SRCS := $(SRC_DIR)/main.c
DEMO_OBJS := $(DEMO_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

TEST_BIN  := $(BUILD_DIR)/test_argparse
TEST_SRCS := $(TEST_DIR)/test_argparse.c
TEST_OBJS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_DIR)/%.o)

DEPS := $(LIB_OBJS:.o=.d) $(DEMO_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

all: $(LIB) $(DEMO)

$(LIB): $(LIB_OBJS) | $(BUILD_DIR)
	$(AR) rcs $@ $^

$(DEMO): $(DEMO_OBJS) $(LIB) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $(DEMO_OBJS) $(LIB) $(LDLIBS)

$(TEST_BIN): $(TEST_OBJS) $(LIB) | $(BUILD_DIR)
	$(CC) $(LDFLAGS) -o $@ $(TEST_OBJS) $(LIB) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	@echo '*' > $(BUILD_DIR)/.gitignore

test: $(TEST_BIN)
	./$(TEST_BIN)

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all test clean
