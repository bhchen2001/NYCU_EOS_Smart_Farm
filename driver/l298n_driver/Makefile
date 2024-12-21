# Cross-compile toolchain prefix
CROSS_COMPILE=aarch64-linux-gnu-

# Kernel module
obj-m := l298n_driver.o

# Kernel source directory (adjust path as needed)
KDIR := ../../linux

# User-space program source
USER_PROGRAMS := l298n_writer

# Default target: build both kernel module and user program
all: RPI_build user_programs

# Build kernel module
RPI_build:
	sudo make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) modules

# Build user-space program
user_programs: $(USER_PROGRAMS)

l298n_writer: l298n_writer.c
	$(CROSS_COMPILE)gcc -o l298n_writer l298n_writer.c

# Clean build artifacts
clean: RPI_clean user_clean

# Clean kernel module build
RPI_clean:
	sudo make ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) -C $(KDIR) M=$(PWD) clean

# Clean user-space programs
user_clean:
	rm -f $(USER_PROGRAMS)

.PHONY: all clean RPI_build user_programs

