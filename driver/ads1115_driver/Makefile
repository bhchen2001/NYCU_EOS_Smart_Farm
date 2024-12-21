# Cross-compile toolchain prefix
CROSS_COMPILE=aarch64-linux-gnu-

# Kernel module
obj-m := ads1115_driver.o

# Kernel source directory (adjust path as needed)
KDIR := ../../linux

# User program source
USER_PROGRAMS := ads1115_test ads1115_reader

# Default target: build both kernel module and user program
all: RPI_build user_programs

# Build kernel module
RPI_build:
	sudo make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

# Build user program
user_programs: $(USER_PROGRAMS)

ads1115_test: ads1115_test.c
	$(CROSS_COMPILE)gcc -o ads1115_test ads1115_test.c

ads1115_reader: ads1115_reader.c
	$(CROSS_COMPILE)gcc -o ads1115_reader ads1115_reader.c

# Clean build artifacts
clean: RPI_clean user_clean

# Clean kernel module build
RPI_clean:
	sudo make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean

# Clean user-space programs
user_clean:
	rm -f $(USER_PROGRAMS)

.PHONY: all clean RPI_build user_programs

