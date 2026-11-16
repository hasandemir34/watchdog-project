CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS =

SRC_DIR = src
WORKER_DIR = workers
OBJ_DIR = obj

# Watchdog kaynak ve nesne dosyaları
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

WORKER_SRCS = $(wildcard $(WORKER_DIR)/*.c)
WORKER_TARGETS = $(patsubst $(WORKER_DIR)/%.c, %, $(WORKER_SRCS))

TARGET = watchdog

.PHONY: all clean dirs

all: dirs $(TARGET) $(WORKER_TARGETS)

dirs:
	@mkdir -p $(OBJ_DIR) logs

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

%: $(WORKER_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(WORKER_TARGETS) logs/*.log