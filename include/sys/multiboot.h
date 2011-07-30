/*

* Copyright (c) 2007, John Cronin
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the copyright holder nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY John Cronin ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL John Cronin BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "sys/types.h"
#include "elf.h"
#include "stream.h"

#define MOD_MAX							40
#define MMAP_MAX						40
#define ELF_MAX							40
#define ELFSTR_MAX						1000

#define MB_MMAP_AVAIL					1

#define MULTIBOOT_MAGIC					0x2BADB002
#define MULTIBOOT_MEM_AVAIL				0x1
#define MULTIBOOT_BOOTDEVICE_AVAIL		0x2
#define MULTIBOOT_CMDLINE_AVAIL			0x4
#define MULTIBOOT_MODS_AVAIL			0x8
#define MULTIBOOT_AOUT_AVAIL			0x10
#define MULTIBOOT_ELF_AVAIL				0x20
#define MULTIBOOT_MMAP_AVAIL			0x40
#define MULTIBOOT_DRIVES_AVAIL			0x80
#define MULTIBOOT_CONFIG_AVAIL			0x100
#define MULTIBOOT_BLNAME_AVAIL			0x200
#define MULTIBOOT_APM_AVAIL				0x400
#define MULTIBOOT_VBE_AVAIL				0x800

struct mb_header {
	unsigned long int flags;
	unsigned long int mem_lower;
	unsigned long int mem_upper;
	unsigned long int boot_device;
	unsigned long int cmdline;
	unsigned long int mods_count;
	unsigned long int mods_addr;
	unsigned long int elf_num;
	unsigned long int elf_size;
	unsigned long int elf_addr;
	unsigned long int elf_shndx;
	unsigned long int mmap_length;
	unsigned long int mmap_addr;
	unsigned long int drives_length;
	unsigned long int drives_addr;
	unsigned long int config_table;
	unsigned long int boot_loader_name;
	unsigned long int apm_table;
	unsigned long int vbe_control_info;
	unsigned long int vbe_mode_info;
	unsigned long int vbe_mode;
	unsigned long int vbe_interface_seg;
	unsigned long int vbe_interface_off;
	unsigned long int vbe_interface_len;
} __attribute__((packed));

struct mb_mmap {
	unsigned long int size;
	union {
		uintptr base_addr;
		struct {
			unsigned long int base_addr_low;
			unsigned long int base_addr_high;
		} __attribute__((packed)) base_addr_struct;
	};
	union {
		uintptr length;
		struct {
			unsigned long int length_low;
			unsigned long int length_high;
		} __attribute__((packed)) length_struct;
	};
	unsigned long int type;
} __attribute__((packed));

struct mb_mod {
	unsigned long int mod_start;
	unsigned long int mod_end;
	unsigned long int string;
	unsigned long int reserved;
} __attribute__((packed));

#define OVERLAP_EQUAL				1
#define OVERLAP_LESS				2
#define OVERLAP_MORE				4
#define OVERLAP_WITHIN				8

class Multiboot {
public:
	mb_header header;

	bool CheckRange(uintptr base, uintptr length);
	uintptr FindRange(uintptr length);
	bool Parse(mb_header *new_header);
	void Dump(IStream *s, unsigned long int flags = 0xffffffff);

	static int Overlap(uintptr base1, uintptr length1, uintptr base2, uintptr length2);

	int mmap_count;
private:
	mb_mod mods[MOD_MAX];
	mb_mmap mmap[MMAP_MAX];
	Elf32_Shdr elf[ELF_MAX];
	char elfstr[ELFSTR_MAX];
};

#endif
