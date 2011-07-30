KERNEL_DIR = kernel
LIBC_DIR = libc

.PHONY: kernel
.PHONY: clean-kernel
.PHONY: clean rebuild

kernel:
	$(MAKE) -C $(KERNEL_DIR) all

clean-kernel:
	$(MAKE) -C $(KERNEL_DIR) clean
	
clean: clean-kernel

all: kernel

rebuild: clean all
