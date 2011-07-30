IDIR = ../../include
ODIR = obj
OUTPUT = i386.a

CPP = i586-pc-tysos-g++
AR = i586-pc-tysos-ar
AS = i586-pc-tysos-as
YASM = yasm

CFLAGS = -I$(IDIR) -nostartfiles -Wall -Wextra -Wno-long-long -pedantic -g -DI386 -DKERNEL
YASMFLAGS = -f elf -g stabs
ASFLAGS = -I$(IDIR) -g

_DEPS = sys/types.h i386/pmem.h sys/multiboot.h sys/pmem.h sys/sched.h sys/syscall.h i386/io.h sys/video.h i386/concurrent.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = head.o i386_pmem.o vga.o arch.o i386_vmem.o i386_interrupt.o int_stub.o i386_task.o i386_switcher.o pic_8259a.o pit_8253.o pit_8253_asm.o concurrent.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CFLAGS)
	
$(ODIR)/%.o: %.s $(DEPS)
	$(AS) -o $@ $< $(ASFLAGS)
	
$(ODIR)/%.o: %.asm $(DEPS)
	$(YASM) $(YASMFLAGS) -o $@ $<
	
all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(AR) rcs $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(OUTPUT)
