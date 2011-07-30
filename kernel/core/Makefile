IDIR = ../../include
ODIR = obj
OUTPUT = core.a

CPP = i586-pc-tysos-g++
AR = i586-pc-tysos-ar

CFLAGS = -I$(IDIR) -nostartfiles -Wall -Wextra -pedantic -Wno-long-long -DI386 -g -DKERNEL

_DEPS = sys/types.h i386/pmem.h sys/multiboot.h sys/pmem.h sys/sched.h sys/syscall.h sys/video.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = main.o multiboot.o pmem.o video.o cppsupport.o stdclib.o formatter.o kernel.o malloc.o sbrk.o \
  section.o vmem.o task.o idle.o syscall.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CPP) -c -o $@ $< $(CFLAGS)
	
all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(AR) rcs $@ $^

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(OUTPUT)
	
