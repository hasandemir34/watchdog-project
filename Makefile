CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200809L -Iinclude

TARGET      := watchdog
WORKER_BIN  := workers/worker
SRC_DIR     := src
OBJ_DIR     := build
SRCS        := $(wildcard $(SRC_DIR)/*.c)
OBJS        := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(TARGET) $(WORKER_BIN)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(WORKER_BIN): workers/worker.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(WORKER_BIN) watchdog.log