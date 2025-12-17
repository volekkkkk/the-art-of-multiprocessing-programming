CC = gcc
CFLAGS = -pthread -O2 -Wall -Wextra -Werror -pedantic
BUILD_DIR = build
TARGET = $(BUILD_DIR)/benchmark

SRCS = benchmark.c peterson_lock.c
OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
