CC = gcc
CFLAGS = -Wall

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

CONTROLLER_SRC = controller_system.c
SENSOR_SRC = sensor_system.c
SRCS = comm.c controller.c shm.c comm_utils.c device_utils.c
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
CLIENT_TARGET = $(BIN_DIR)/client
CONTROLLER_TARGET = $(BIN_DIR)/controller_system
SENSOR_TARGET = $(BIN_DIR)/sensor_system

.PHONY: all client driver clean

all: $(CONTROLLER_TARGET) $(SENSOR_TARGET)
client: $(CLIENT_TARGET)

$(CONTROLLER_TARGET): $(CONTROLLER_SRC) $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(CONTROLLER_SRC) $(OBJS) -o $@
	@echo "Built executable: $@"

$(SENSOR_TARGET): $(SENSOR_SRC) $(OBJ_DIR)/shm.o $(OBJ_DIR)/comm_utils.o $(OBJ_DIR)/device_utils.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SENSOR_SRC) $(OBJ_DIR)/shm.o $(OBJ_DIR)/comm_utils.o $(OBJ_DIR)/device_utils.o -o $@
	@echo "Built executable: $@"

$(CLIENT_TARGET): $(SRC_DIR)/client.c
	@mkdir -p $(BIN_DIR)
	$(MAKE) -C $(SRC_DIR) client

$(OBJS):
	$(MAKE) -C $(SRC_DIR)

clean:
	rm -f $(CONTROLLER_TARGET)
	rm -f $(SENSOR_TARGET)
	$(MAKE) -C $(SRC_DIR) clean
	@echo "Cleaned build artifacts in root and /obj/"