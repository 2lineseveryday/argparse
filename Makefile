CC = gcc
CFLAGS = -Wall -Wextra -g -O1

BUILD_DIR = build
SRC_DIR = src
TARGET = $(BUILD_DIR)/main

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -o $@ -c $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	@echo \* > $(BUILD_DIR)/.gitignore

-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)


.PHONY: clean all
