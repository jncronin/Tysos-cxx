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
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "sys/kernel.h"
#include "sys/section.h"
#include "sys/multiboot.h"
#include "sys/arch.h"
#include "arraylist.h"
#include "sys/task.h"
#include "string.h"
extern Arch a;
#include "sys/sched.h"

extern Thread *CurrentThread;

static const char k_name[] = "Tysos Kernel";
static const char elfh_name[] = "Elf headers";
static const char kst_name[] = "Kernel Startup Thread";

Kernel::Kernel() : k_sects(&k_sections, &k_procsects)
{
	this->name = (char *)k_name;
	this->Sections = &k_sects;
	this->Threads = &k_threads;
	this->Heap = NULL;
	this->user_mode = false;
}

void Kernel::InitKernel(Multiboot *mb)
{
	// Find a place to store the Section classes for the elf sections
	this->elf_section_headers.Base = mb->FindRange(mb->header.elf_num * sizeof(Section));
	if(elf_section_headers.Base == 0) {
		a.kconsole->Puts("Kernel::InitKernel ERROR: did not find a range for elf headers section");
		return;
	}
	elf_section_headers.Length = mb->header.elf_num * sizeof(Section);
	elf_section_headers.ExpandDown = elf_section_headers.ExpandUp = elf_section_headers.LowerPadding =
		elf_section_headers.UpperPadding = 0;
	elf_section_headers.Flags = Section::Defined | Section::Bits | Section::Read | Section::Write | Section::KernelSection;
	elf_section_headers.MemSource = elf_section_headers.Base;
	elf_section_headers.Name = (char *)elfh_name;
	Sections->Add(elf_section_headers);

	// Add the elf sections
	Section *sects = (Section *)elf_section_headers.Base;
	Elf32_Shdr *e_str = (Elf32_Shdr *)(mb->header.elf_addr + mb->header.elf_shndx * mb->header.elf_size);
	unsigned int i;
	for(i = 0; i < mb->header.elf_num; i++) {
		Elf32_Shdr *e = (Elf32_Shdr *)(mb->header.elf_addr + i * mb->header.elf_size);
		if(e->sh_size == 0)
			continue;
		sects[i].Base = e->sh_addr;
		sects[i].Length = e->sh_size;
		sects[i].ExpandDown = sects[i].ExpandUp = sects[i].UpperPadding = sects[i].LowerPadding = 0;
		sects[i].Name = (char *)(e_str->sh_addr + e->sh_name);
		sects[i].Flags = Section::Defined | Section::Bits | Section::KernelSection | Section::Read;
		sects[i].MemSource = sects[i].Base;
		if(e->sh_flags & SHF_WRITE)
			sects[i].Flags |= Section::Write;
		if(e->sh_flags & SHF_EXECINSTR)
			sects[i].Flags |= Section::Execute;
		Sections->Add(sects[i]);

		// See if this is .bss, if it is then its the kernel stack
		if(!strcmp(".bss", sects[i].Name))
			startup_thread.Stack = &sects[i];
	}

	// Add the current thread
	startup_thread.proc = this;
	startup_thread.name = (char *)kst_name;
	startup_thread.ThreadLocal = NULL;
	startup_thread.location = LOC_CURRENT;
	startup_thread.priority = -1;
	Threads->Add(startup_thread);
	CurrentThread = &startup_thread;

	// Set up address space
	this->Address_Space = a.kst_asp;
}
