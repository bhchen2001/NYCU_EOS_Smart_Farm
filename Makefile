CC = gcc
CFLAGS = -Wall

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CONTROLLER_SRC = controller_system.c
SENSOR_SRC = sensor_system.c
OBJS = $(OBJ_DIR)/controller.o $(OBJ_DIR)/comm.o $(OBJ_DIR)/shm.o
CONTROLLER_TARGET = $(BIN_DIR)/controller_system
SENSOR_TARGET = $(BIN_DIR)/sensor_system

.PHONY: all clean

all: $(CONTROLLER_TARGET) $(CLIENT_TARGET) $(SENSOR_TARGET)

$(CONTROLLER_TARGET): $(CONTROLLER_SRC) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CONTROLLER_SRC) $(OBJS) -o $@
	@echo "Built executable: $@"

$(SENSOR_TARGET): $(SENSOR_SRC) $(OBJ_DIR)/shm.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SENSOR_SRC) $(OBJ_DIR)/shm.o -o $@
	@echo "Built executable: $@"

$(OBJS):
	$(MAKE) -C $(SRC_DIR)

clean:
	rm -f $(CONTROLLER_TARGET)
	$(MAKE) -C $(SRC_DIR) clean
	@echo "Cleaned build artifacts in root and /obj/"