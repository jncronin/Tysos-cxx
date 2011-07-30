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

#ifndef I386_VMEM_H
#define I386_VMEM_H

#define KERNEL_START				0x00100000
#define KERNEL_END					0x3fbfffff
#define PAGETABLES					0x3fc00000
#define PAGEDIR						0x3fcff000
#define USER_START					0x40000000
#define USER_END					0xffffffff

#define P_PRESENT			0x1			/**< page is present */
#define P_WRITABLE			0x2			/**< page is writable */
#define P_USER				0x4			/**< page is accessible by CPL3 code */
#define P_WRITE_THROUGH		0x8			/**< write through enabled */
#define P_CACHE_DISABLED	0x10		/**< cache disabled */
#define P_ACCESSED			0x20		/**< page has been accessed */
#define P_RESERVED			0x40		/**< reserved bit */
#define P_DIRTY				0x40		/**< dirty page */
#define P_LARGEPAGE			0x80		/**< large page */
#define P_PAT				0x80		/**< pat */
#define P_GLOBAL			0x100		/**< global page */

#include "sys/vmem.h"
#include "i386/pmem.h"

class i386_AddressSpace : public AddressSpace {
public:
	virtual void Init();
};

class i386_VirtMemManager : public VirtMemManager {
public:
	virtual uintptr GetKernelStart() { return KERNEL_START; };
	virtual uintptr GetKernelEnd() { return KERNEL_END; };
	virtual uintptr GetProcessStart() { return USER_START; };
	virtual uintptr GetProcessEnd() { return USER_END; };
	virtual uintptr GetPageSize() { return PAGESIZE; };
	virtual uintptr GetPageMask() { return PAGEMASK; };

	virtual void Init(SectionList *KernelSections);
	virtual void SetAddressSpace(AddressSpace *tsi);
	virtual physaddr_t MapPage(uintptr virtaddr, AddressSpace *address_space = NULL, bool unmap = false, 
		uintptr length = 0, physaddr_t physaddr = 0, bool user = false, bool writable = true, 
		bool cache_disabled = false, bool write_through = false);
	virtual uintptr TempMap(physaddr_t paddr);
	virtual void TempUnmap();


	virtual void Enable();

private:
	static const unsigned int KernelPtsLength = 0x100000;

	Lockable temp_lock;
};

#endif
