CC = gcc
CFLAGS = -Wall -I../

OBJ_DIR = ../obj
BIN_DIR = ../bin

SRCS = comm.c controller.c shm.c
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

CLIENT_SRC = client.c
CLIENT_TARGET = $(BIN_DIR)/client

.PHONY: all clean

all: $(OBJS) $(CLIENT_TARGET)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled: $< -> $@"

$(CLIENT_TARGET): $(CLIENT_SRC)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $@
	@echo "Built executable: $@"

clean:
	rm -f $(OBJS)
	rm -f $(CLIENT_TARGET)
	@echo "Cleaned object files in /obj/"