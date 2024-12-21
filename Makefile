# Cross-Compile Toolchain Prefix
CROSS_COMPILE=aarch64-linux-gnu-

# Target binary name
TARGET=sensor_subsystem

# Source file
SRC=$(TARGET).c

# Compilation Flags
CFLAGS=-Wall -O2

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(SRC)
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $(TARGET) $(SRC) -lrt

# Clean target
clean:
	rm -f $(TARGET)

.PHONY: all clean

