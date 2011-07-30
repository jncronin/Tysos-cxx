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

#ifndef VMEM_H
#define VMEM_H

#include "sys/types.h"
#include "sys/section.h"
#include "sys/concurrent.h"

class AddressSpace {
public:
	physaddr_t addr_space;

	virtual void Init() = 0;

	virtual ~AddressSpace() {};
};

#include "sys/multiboot.h"
#include "sys/task.h"

class SectionList;

class VirtMemManager {
public:
	virtual uintptr GetProcessStart() = 0;
	virtual uintptr GetProcessEnd() = 0;
	virtual uintptr GetKernelStart() = 0;
	virtual uintptr GetKernelEnd() = 0;
	virtual uintptr GetPageSize() = 0;
	virtual uintptr GetPageMask() = 0;

	virtual void Init(SectionList *KernelSections) = 0;

	virtual void SetAddressSpace(AddressSpace *asp) = 0;
	virtual AddressSpace *GetAddressSpace() { return aspace; };

	virtual physaddr_t MapPage(uintptr virtaddr, AddressSpace *address_space = NULL, bool unmap = false, 
		uintptr length = NULL, physaddr_t physaddr = NULL, bool user = false, bool writable = true,
		bool cache_disabled = false, bool write_through = false) = 0;
	virtual uintptr TempMap(physaddr_t paddr) = 0;
	virtual void TempUnmap() = 0;

	virtual void ResolvePageFault(uintptr fault_address, bool write_fault, bool protection_error, bool user_mode);

	virtual ~VirtMemManager() {};
	VirtMemManager() { Enabled = false; };

	virtual void Enable() = 0;

	bool Enabled;

protected:
	AddressSpace *aspace;
};

#endif
