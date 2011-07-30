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

#include "sys/pmem.h"

#ifndef I386_PMEM_H
#define I386_PMEM_H

#define PAGESIZE			0x1000
#define PAGEMASK			0xfffff000
#define MAX_STACK			0x100

class i386_PageProvider : public PageProvider {
public:
	virtual physaddr_t GetPage();
	i386_PageProvider();
	virtual uintptr GetPageSize() { return PAGESIZE; };
	virtual uintptr GetPageMask() { return PAGEMASK; };

protected:
	physaddr_t nextpage;
};

class i386_PhysMemAllocator : public PhysMemAllocator {
public:
	virtual physaddr_t Alloc(physaddr_t fixed_address);
	virtual physaddr_t Alloc();
	virtual physaddr_t BeginAlloc();
	virtual physaddr_t EndAlloc(physaddr_t paddr);
	virtual void Free(physaddr_t paddr);
	virtual void Init(Multiboot *mb, int cpus, int flags);
	virtual physaddr_t AllocRange(uintptr length);

private:
	int next_cpu;
	PhysMemBitmap bmp;
	PhysMemStack stack[MAX_STACK];
};

#endif
