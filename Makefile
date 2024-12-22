CC = gcc
CFLAGS = -Wall

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

MAIN_SRC = controller_system.c
OBJS = $(OBJ_DIR)/controller.o $(OBJ_DIR)/comm.o
TARGET = $(BIN_DIR)/controller_system

.PHONY: all clean

all: $(TARGET) $(CLIENT_TARGET)

$(TARGET): $(MAIN_SRC) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(MAIN_SRC) $(OBJS) -o $@
	@echo "Built executable: $@"

$(OBJS):
	$(MAKE) -C $(SRC_DIR)

clean:
	rm -f $(TARGET)
	$(MAKE) -C $(SRC_DIR) clean
	@echo "Cleaned build artifacts in root and /obj/"