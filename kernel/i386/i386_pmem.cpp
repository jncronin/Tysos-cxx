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

#include "i386/pmem.h"
#include "i386/concurrent.h"
#include "sys/arch.h"
#include "string.h"
#include "sys/sectionlist.h"

extern Arch a;
static i386_PageProvider pprov;

physaddr_t i386_PageProvider::GetPage()
{
	nextpage += PAGESIZE;
	if(nextpage > (mb->header.mem_upper * 1024 + 0x100000))		// mem_upper is in kiB
		return NULL;
	return nextpage - PAGESIZE;
}

i386_PageProvider::i386_PageProvider()
{
	this->nextpage = 0x100000;
}

physaddr_t i386_PhysMemAllocator::Alloc(physaddr_t fixed_address)
{
	if(fixed_address >= 0x1000000)
		return NULL;					// Can't allocate fixed address above 16MB
	return bmp.Alloc(fixed_address);
}

physaddr_t i386_PhysMemAllocator::Alloc()
{
	physaddr_t ret = stack[0].PopPage();
	if(ret == 0)
		ret = bmp.Alloc();
	return ret;
}

physaddr_t i386_PhysMemAllocator::BeginAlloc()
{
	physaddr_t ret = stack[0].BeginPop();
	if(ret == 0)
		ret = bmp.Alloc();
	return ret;
}

physaddr_t i386_PhysMemAllocator::EndAlloc(physaddr_t paddr)
{
	stack[0].EndPop(paddr);
	memset((void *)paddr, 0, PAGESIZE);
	return NULL;
}

void i386_PhysMemAllocator::Free(physaddr_t paddr)
{
	if(paddr >= 0x1000000)
		stack[0].PushPage(paddr);
	else
		bmp.Free(paddr);
}

void i386_PhysMemAllocator::Init(Multiboot *mb, int cpus, int flags)
{
	if(cpus != 1)
		a.kconsole->Puts("Error: currently only uniprocessor systems supported");
	if(flags != 0)
		a.kconsole->Puts("Error: flags must currently be set to 0");

	if(mb->header.mem_upper < (31 * 1024))
		a.kconsole->Puts("Error: 32 MB of memory is required");

	a.kconsole->Puts("i386_PhysMemAllocator::Init");

	// Mark the first 1MB as unavailable
	mb_mmap *new_mmap = (mb_mmap *)(mb->header.mmap_addr + mb->mmap_count * sizeof(mb_mmap));
	new_mmap->base_addr = 0x0;
	new_mmap->length = 0x100000;
	new_mmap->type = 2;
	mb->mmap_count++;

	// Dump the memory map
	//mb->Dump(a.kconsole);

	// Size of a 16MB bitmap is 512 bytes
	physaddr_t bitmap = mb->FindRange(512);
	if(bitmap != 0) {
		// Mark the region as used
		Elf32_Shdr *new_region = (Elf32_Shdr *)(mb->header.elf_addr + mb->header.elf_num * mb->header.elf_size);
		new_region->sh_addr = bitmap;
		new_region->sh_size = 512;
		new_region->sh_name = 0;
		mb->header.elf_num++;

		bmp.Init((void *)bitmap, 0x0, 0x1000000, PAGESIZE);

		a.kconsole->Printf("Bitmap allocated at %x\n", bitmap);
	}

	pp = &pprov;
	pp->SetMultiboot(mb);

	// Set all free pages as free
	physaddr_t paddr;
	physaddr_t freepages = 0;
	while((paddr = pp->GetPage())) {
		if(mb->CheckRange(paddr, pp->GetPageSize())) {
			freepages++;
			this->Free(paddr);
		}
	}
}

physaddr_t i386_PhysMemAllocator::AllocRange(uintptr length)
{
	if(length <= PAGESIZE)
		return Alloc();
	return bmp.AllocRange(length);
}
