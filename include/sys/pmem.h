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

#ifndef PMEM_H
#define PMEM_H

#include "sys/types.h"
#include "sys/multiboot.h"
#include "sys/section.h"
#include "sys/concurrent.h"
#include "list.h"

// A page provider class that is architecture dependent
class PageProvider {
public:
	virtual uintptr GetPageSize() = 0;
	virtual physaddr_t GetPage() = 0;
	virtual uintptr GetPageMask() = 0;
	virtual void SetMultiboot(Multiboot *mb_addr) { mb = mb_addr; };
	virtual ~PageProvider() {};

protected:
	Multiboot *mb;
};

// A physical memory stack
class PhysMemStack : public Lockable {
public:
	physaddr_t PopPage();
	physaddr_t BeginPop();
	physaddr_t EndPop(physaddr_t paddr);
	void PushPage(physaddr_t paddr);
	PhysMemStack();
	void Init();

protected:
	physaddr_t top;
};

// A physical memory bitmap
class PhysMemBitmap {
public:
	physaddr_t Alloc();
	physaddr_t Alloc(physaddr_t paddr);
	physaddr_t AllocRange(uintptr length);
	void Free(physaddr_t paddr);
	void Init(void *bitmap_memory, unsigned long int bitmap_base, unsigned long int bitmap_end,
		unsigned long int page_size);

protected:
	void *bmp;
	unsigned long int base, end;
	unsigned long int ps;
	unsigned long int bmp_size;
};

// The physical memory allocator
class PhysMemAllocator {
public:
	virtual physaddr_t Alloc() { return EndAlloc(BeginAlloc()); }
	virtual physaddr_t BeginAlloc() = 0;
	virtual physaddr_t EndAlloc(physaddr_t paddr) = 0;
	virtual physaddr_t Alloc(physaddr_t fixed_address) = 0;
	virtual void Free(physaddr_t paddr) = 0
	virtual void Init(Multiboot *mb, int cpus, int flags) = 0;
	virtual void UpdateUsed(SectionList *sections);

	virtual physaddr_t AllocRange(uintptr length) = 0;

	virtual ~PhysMemAllocator() {};

protected:
	PageProvider *pp;
};

#endif
