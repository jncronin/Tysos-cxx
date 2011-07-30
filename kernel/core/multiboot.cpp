/* Copyright (c) 2007, John Cronin
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE PSSIBILITY OF SUCH DAMAGE. */

#include "sys/multiboot.h"
#include "string.h"
#include "sys/formatter.h"
#include "sys/arch.h"

extern Arch a;

// A class where we persistently keep the Multiboot information
Multiboot mboot;

bool Multiboot::Parse(mb_header *new_header)
{
	// Copy the multiboot header, and all its referenced tables, to a safe memory location

	memcpy(&this->header, new_header, sizeof(mb_header));

	uintptr i;

	if(this->header.flags & MULTIBOOT_ELF_AVAIL) {
		// copy elf headers

		uintptr elf_src = (uintptr)this->header.elf_addr;
		for(i = 0; i < (uintptr)this->header.elf_num; i++) {
			if(i >= ELF_MAX)
				break;
			memcpy(&this->elf[i], (void *)elf_src, sizeof(Elf32_Shdr));
			elf_src += (uintptr)this->header.elf_size;
		}

		this->header.elf_size = sizeof(Elf32_Shdr);
		this->header.elf_addr = (unsigned long int)this->elf;
		this->header.elf_num = (unsigned long int)i;

		// copy string table
		if(this->elf[this->header.elf_shndx].sh_size > ELFSTR_MAX)
			memcpy(this->elfstr, (void *)this->elf[this->header.elf_shndx].sh_addr, ELFSTR_MAX);
		else {
			memcpy(this->elfstr, (void *)this->elf[this->header.elf_shndx].sh_addr, 
				this->elf[this->header.elf_shndx].sh_size);
		}
		this->elf[this->header.elf_shndx].sh_addr = (Elf32_Off)this->elfstr;
	} else {
		this->header.elf_num = 0;
	}

	if(header.flags & MULTIBOOT_MODS_AVAIL) {
		/* Copy module sections */
		uintptr mod_src = (uintptr)header.mods_addr;
		for(i = 0; i < (uintptr)header.mods_count; i++) {
			if(i >= MOD_MAX)
				break;
			memcpy(&mods[i], (void *)mod_src, sizeof(mb_mod));
			mod_src += (uintptr)sizeof(mb_mod);
		}
		header.mods_addr = (unsigned long int)mods;
		header.mods_count = (unsigned long int)i;
	} else {
		header.mods_count = 0;
	}

	if(header.flags & MULTIBOOT_MMAP_AVAIL) {
		/* Copy memory map
		 Store number of entries */
		/* note that the memmap is now a properly iterable array, rather than some
		 bizarre-pseudo-linked-list */
		uintptr mem_src = (uintptr)header.mmap_addr;
		i = 0;
		while(mem_src < (uintptr)header.mmap_addr + (uintptr)header.mmap_length) {
			if(i >= MMAP_MAX)
				break;
			memcpy(&mmap[i], (void *)mem_src, (uintptr)sizeof(mb_mmap));
			mem_src += (uintptr)((mb_mmap *)mem_src)->size + 4;
			i++;
		}
		mmap_count = i;
		header.mmap_addr = (uintptr)mmap;
	} else {
		mmap_count = 0;
	}

	return true;
}

uintptr Multiboot::FindRange(uintptr length)
{
	// Searches for a free range, by checking the start of each free mmap region and the region
	// immediately after each elf section and module

	uintptr i;
	for(i = 0; i < (uintptr)mmap_count; i++) {
		if(mmap[i].type == MB_MMAP_AVAIL) {
			if(CheckRange(mmap[i].base_addr, length))
				return mmap[i].base_addr;
		}
	}

	for(i = 0; i < header.elf_num; i++) {
		if(CheckRange(elf[i].sh_addr + elf[i].sh_size, length))
			return elf[i].sh_addr + elf[i].sh_size;
	}

	for(i = 0; i < header.mods_count; i++) {
		if(CheckRange(mods[i].mod_end, length))
			return mods[i].mod_end;
	}

	return NULL;
}

bool Multiboot::CheckRange(uintptr base, uintptr length)
{
	// Check if a range is free, according to the multiboot header
	uintptr i;
	bool free = false;

#ifdef DEBUG_CHECKRANGE
	a.kconsole->Printf("CheckRange: base %x, length %x: ", base, length);
#endif

	// First, check the memory map
	if(header.flags & MULTIBOOT_MMAP_AVAIL) {
		for(i = 0; i < (uintptr)mmap_count; i++) {
			if(mmap[i].type == MB_MMAP_AVAIL) {
				if(Multiboot::Overlap(base, length, mmap[i].base_addr, mmap[i].length) & OVERLAP_WITHIN)
					free = true;
			} else {
				if(Multiboot::Overlap(base, length, mmap[i].base_addr, mmap[i].length) & OVERLAP_EQUAL) {
#ifdef DEBUG_CHECKRANGE
					a.kconsole->Printf("failed on mmap (used by %u)\n", i);
#endif
					return false;
				}
			}
		}
		if(!free) {
#ifdef DEBUG_CHECKRANGE
			a.kconsole->Printf("failed on mmap (not free)\n");
#endif
			return false;
		}
	} else if(header.flags & MULTIBOOT_MEM_AVAIL) {
		if((base + length) > (header.mem_upper + 0x100000)) {
#ifdef DEBUG_CHECKRANGE
			a.kconsole->Printf("failed on mem\n");
#endif
			return false;
		}
	}

	// Then check the elf headers
	if(header.flags & MULTIBOOT_ELF_AVAIL) {
		for(i = 0; i < header.elf_num; i++) {
			if(Multiboot::Overlap(base, length, elf[i].sh_addr, elf[i].sh_size) & OVERLAP_EQUAL) {
#ifdef DEBUG_CHECKRANGE
				a.kconsole->Printf("failed on elf (%u: %s)\n", i, elf[header.elf_shndx].sh_addr + elf[i].sh_name);
#endif
				return false;
			}
		}
	}

	// Finally check the modules
	if(header.flags & MULTIBOOT_MODS_AVAIL) {
		for(i = 0; i < header.mods_count; i++) {
			if(Multiboot::Overlap(base, length, mods[i].mod_start, mods[i].mod_end - mods[i].mod_start) & OVERLAP_EQUAL) {
#ifdef DEBUG_CHECKRANGE
				a.kconsole->Printf("failed on mods\n");
#endif
				return false;
			}
		}
	}

#ifdef DEBUG_CHECKRANGE
	a.kconsole->Printf("succeeded\n");
#endif
	return true;
}

int Multiboot::Overlap(uintptr base1, uintptr length1, uintptr base2, uintptr length2)
{
	// Compares region 1 with region 2, sets bits in return value according to result.
	// If part of region 1 is less than region 2, sets LESS
	// If part of region 1 overlaps region 2, sets EQUAL
	// If part of region 1 is greater than region 2, sets MORE
	// If EQUAL is set, without either LESS or MORE, then it also sets WITHIN (ie. region 1 is totally within 2)

	if(length1 == 0)
		return 0;
	if(length2 == 0)
		return 0;

	int ret = 0;
	uintptr end1 = base1 + (length1 - 1);
	uintptr end2 = base2 + (length2 - 1);

	if(base1 < base2)
		ret |= OVERLAP_LESS;
	if(end1 > end2)
		ret |= OVERLAP_MORE;
	if((base1 >= base2) && (base1 <= end2))
		ret |= OVERLAP_EQUAL;
	else if((end1 >= base2) && (end1 <= end2))
		ret |= OVERLAP_EQUAL;
	else if((ret & OVERLAP_LESS) && (ret & OVERLAP_MORE))
		ret |= OVERLAP_EQUAL;
	if(ret == OVERLAP_EQUAL)
		ret |= OVERLAP_WITHIN;

	return ret;
}

void Multiboot::Dump(IStream *s, unsigned long int flags)
{
	int i;
	Formatter::fPrintf(s, "Multiboot header:\n");
	if(flags & MULTIBOOT_MEM_AVAIL)
		Formatter::fPrintf(s, "  Lower memory: %u kiB, upper memory: %u kiB\n", header.mem_lower, header.mem_upper);
	if(flags & MULTIBOOT_MMAP_AVAIL) {
		Formatter::fPrintf(s, "  Memory map:\n");
		for(i = 0; i < mmap_count; i++)
			Formatter::fPrintf(s, "   %u %x - %x\n", mmap[i].type, mmap[i].base_addr, mmap[i].base_addr + mmap[i].length);
	}
	if(flags & MULTIBOOT_ELF_AVAIL) {
		Formatter::fPrintf(s, "  Elf sections:\n");
		for(i = 0; i < (int)header.elf_num; i++) {
			Formatter::fPrintf(s, "   %s %x - %x\n", 
				(((char *)(elf[header.elf_shndx].sh_addr + elf[i].sh_name))[0] == '\0') ?
				"Unnamed" : (const char *)(elf[header.elf_shndx].sh_addr + elf[i].sh_name),
				elf[i].sh_addr + elf[i].sh_size);
		}
	}
}
