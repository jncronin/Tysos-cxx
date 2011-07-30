ENTRY (loader)

SECTIONS {
	. = 0x00100000;
	.multiboot : {
		*(.multiboot)
	}
	
	. = 0x00160000;
	.gdt : {
		*(.gdt)
	}
	
	. = 0x00180000;
	.idt : {
		*(.idt)
	}
	
	. = 0x00200000;
	.text : {
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t*)
		*(.const*)
	}
	.eh_frame : {
		start_eh_frame = .;
		*(.eh_frame)
		*(.eh_frame.*)
		QUAD(0)
	}
	.rodata ALIGN (0x1000) : {
		*(.rodata)
		*(.rodata.*)
		*(.rdata)
		*(.gnu.linkonce.r*)
	}
	.data ALIGN (0x1000) : {
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		*(.tss)
		start_ctors = .;
		*(.ctor*)
		end_ctors = .;
		start_dtors = .;
		*(.dtor*)
		end_dtors = .;
	}
	.bss ALIGN (0x1000) : {
		_sbss = .;
		*(COMMON)
		*(.bss)
		*(.bss.*)
		*(.gnu.linkonce.b*)
		_ebss = .;
	}
}
