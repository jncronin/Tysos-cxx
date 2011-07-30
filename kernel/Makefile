CORE_DIR = core
I386_DIR = i386

CORE_LIB = $(CORE_DIR)/core.a
I386_LIB = $(I386_DIR)/i386.a

LINK = i586-pc-tysos-gcc
LDFLAGS = -Wl,-Tlinker.ld,--whole-archive -nostdlib -lgcc -lsupc++
I386_LDFLAGS = -Wl,-Map=i386-map.txt

MCOPY = mcopy
IMAGE = image

i386-kernel: $(I386_LIB) $(CORE_LIB)
	$(LINK) -o $@ $(LDFLAGS) $(I386_LDFLAGS) $^ 
	
$(CORE_LIB):
	$(MAKE) -C $(CORE_DIR) all
	
$(I386_LIB):
	$(MAKE) -C $(I386_DIR) all
	
all: image

$(IMAGE): i386-kernel
	$(MCOPY) -o -i $@ $^ ::/boot/

.PHONY: $(CORE_LIB) $(I386_LIB) clean
.PHONY: clean-core clean-i386

clean-core:
	$(MAKE) -C $(CORE_DIR) clean
	
clean-i386:
	$(MAKE) -C $(I386_DIR) clean

clean: clean-core clean-i386
