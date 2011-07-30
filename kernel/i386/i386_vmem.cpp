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

#include "i386/vmem.h"
#include "i386/task.h"
#include "sys/arch.h"
#include "i386/pmem.h"
#include "sys/concurrent.h"

#define invlpg(address) \
	__asm volatile ( "invlpg %0" : : "m" (*(char *)(address)) );

extern Arch a;

static const char tpd_str[] = "Temporary page directory";
static const char tpt_str[] = "Temporary page table";
static const char tp_str[] = "Temporary page";
static const char kpt_str[] = "Kernel page tables";
static Section temp_pd;
static Section temp_pt;
static Section temp_p;
static Section kernel_pts;

void i386_AddressSpace::Init()
{
	// First get a page table for the process
	unsigned long int *wr_pd;
	if(a.vmm->Enabled) {
		this->addr_space = a.vmm->MapPage(temp_pd.Base);
		wr_pd = (unsigned long int *)temp_pd.Base;
	} else {
		wr_pd = (unsigned long int *)a.pma->Alloc();
		this->addr_space = (physaddr_t)wr_pd;
	}

	if(wr_pd == NULL) {
		a.kconsole->Puts("i386_AddressSpace::Init ERROR: unable to allocate a page directory");
		return;
	}

	// Now map the first 255 entries to the kernel
	int i;
	unsigned long int entry;
	for(i = 0; i < 255; i++) {
		entry = kernel_pts.Base + i * PAGESIZE;
		entry &= PAGEMASK;
		entry |= P_PRESENT;
		entry |= P_WRITABLE;
		entry |= P_GLOBAL;

		wr_pd[i] = entry;
	}

	// Map the 256th entry back to itself
	entry = addr_space;
	entry |= P_PRESENT;
	entry |= P_WRITABLE;
	entry |= P_GLOBAL;
	wr_pd[255] = entry;

	// Unmap it if necessary
	if(a.vmm->Enabled)
		a.vmm->MapPage(temp_pd.Base, NULL, true);
}

void i386_VirtMemManager::SetAddressSpace(AddressSpace *asp)
{
	__asm volatile ( "movl %0, %%cr3" : : "r" (asp->addr_space) );
}

void i386_VirtMemManager::Enable()
{
	__asm volatile ( "movl %%cr0, %%eax; orl $0x80000000, %%eax; movl %%eax, %%cr0" : : : "eax" );
	Enabled = true;
}

void i386_VirtMemManager::Init(SectionList *KernelSections)
{
	// Allocate space for the kernel page tables
	kernel_pts.Base = a.pma->AllocRange(KernelPtsLength);
	if(kernel_pts.Base == 0) {
		a.kconsole->Puts("i386_VirtMemManager::Init ERROR: could not allocate physical pages for kernel page tables");
		return;
	}
	kernel_pts.Length = KernelPtsLength;
	kernel_pts.ExpandDown = kernel_pts.ExpandUp = kernel_pts.UpperPadding = kernel_pts.LowerPadding = 0;
	kernel_pts.Name = (char *)kpt_str;
	kernel_pts.MemSource = kernel_pts.Base;
	kernel_pts.Flags = Section::Defined | Section::Bits | Section::KernelSection | Section::Read | Section::Write;
	KernelSections->Add(kernel_pts);

	// Allocate space for temporary page directory
	temp_pd.Length = PAGESIZE;
	temp_pd.ExpandDown = temp_pd.ExpandUp = temp_pd.UpperPadding = temp_pd.LowerPadding = 0;
	temp_pd.Name = (char *)tpd_str;
	temp_pd.Flags = Section::Defined | Section::KernelSection | Section::Read | Section::Write;
	if(KernelSections->FindFree(this, &temp_pd) == NULL) {
		a.kconsole->Puts("i386_VirtMemManager::Init ERROR: could not allocate section for temp page directory");
		while(1);
		return;
	}
	KernelSections->Add(temp_pd);

	// Allocate space for temporary page table
	temp_pt.Length = PAGESIZE;
	temp_pt.ExpandDown = temp_pt.ExpandUp = temp_pt.UpperPadding = temp_pt.LowerPadding = 0;
	temp_pt.Name = (char *)tpt_str;
	temp_pt.Flags = Section::Defined | Section::KernelSection | Section::Read | Section::Write;
	if(KernelSections->FindFree(this, &temp_pt) == NULL) {
		a.kconsole->Puts("i386_VirtMemManager::Init ERROR: could not allocate section for temp page table");
		while(1);
		return;
	}
	KernelSections->Add(temp_pt);

	// Allocate space for a temporary page
	temp_p.Length = PAGESIZE;
	temp_p.ExpandDown = temp_p.ExpandUp = temp_p.UpperPadding = temp_p.LowerPadding = 0;
	temp_p.Name = (char *)tp_str;
	temp_p.Flags = Section::Defined | Section::KernelSection | Section::Read | Section::Write;
	if(KernelSections->FindFree(this, &temp_p) == NULL) {
		a.kconsole->Puts("i386_VirtMemManager::Init ERROR: could not allocate section for temp page");
		while(1);
		return;
	}
	KernelSections->Add(temp_p);

	// Now set up the kernel page tables
	int i, c;
	c = KernelSections->Count();
	unsigned long int *kpt = (unsigned long int *)kernel_pts.Base;
	for(i = 0; i < c; i++) {
		Section &s = KernelSections->GetItem(i);
		if(s.Flags & Section::Bits) {
			// The section has bits defined.  Map them
			physaddr_t cur_page, first_page;

			first_page = cur_page = (s.Base + PAGESIZE - 1) & PAGEMASK;
			while(cur_page < (s.Base + s.Length)) {
				unsigned long int entry;
				entry = (s.MemSource + cur_page - first_page + PAGESIZE - 1) & PAGEMASK;
				entry |= P_PRESENT;
				entry |= P_WRITABLE;
				entry |= P_GLOBAL;

				kpt[cur_page >> 12] = entry;

				cur_page += PAGESIZE;
			}
		}
	}
}

physaddr_t i386_VirtMemManager::MapPage(uintptr virtaddr, AddressSpace *asp, bool unmap, uintptr length, 
								  physaddr_t physaddr, bool user, bool writable, 
								  bool cache_disabled, bool write_through)
{
	if((length != 0) & (length != PAGESIZE)) {
		while(length > 0) {
			MapPage(virtaddr, asp, unmap, NULL, physaddr, user, true, cache_disabled, write_through);
			length -= PAGESIZE;
			virtaddr += PAGESIZE;
			if(physaddr != 0)
				physaddr += PAGESIZE;
		}

		return physaddr;
	}

	if(this->Enabled == false) {
		a.kconsole->Puts("i386_VirtMemManager::MapPage: ERROR cannot use MapPage function when paging is not enabled");
		return NULL;
	}

	// Get pointers for where to write to the page directory
	unsigned long int *wr_pd, *wr_pt;
	unsigned long int pd_ent = (virtaddr >> 22) & 0x3ff;
	unsigned long int pt_ent = (virtaddr >> 12) & 0x3ff;

	if(asp == NULL) {
		wr_pd = (unsigned long int *)PAGEDIR;
		wr_pt = (unsigned long int *)(PAGETABLES + pd_ent * 0x1000);
	} else {
		// Lock the temporary pages and use them to map page structs from another address space
		this->temp_lock.Lock();

		wr_pd = (unsigned long int *)temp_pd.Base;
		MapPage((uintptr)wr_pd, NULL, false, 0, asp->addr_space);

		wr_pt = (unsigned long int *)temp_pt.Base;
	}

	// Determine if the page table exists
	if((wr_pd[pd_ent] & P_PRESENT) == 0) {
		// It does not so we need to create it

		physaddr_t new_pt = a.pma->BeginAlloc();
		MapPage((uintptr)wr_pt, asp, false, NULL, new_pt, true);
		a.pma->EndAlloc((uintptr)wr_pt);

		wr_pd[pd_ent] = new_pt | P_PRESENT | P_USER | P_WRITABLE;
		invlpg((uintptr)wr_pt);

		// Map the page table temporarily if we have to
		if(asp != NULL)
			MapPage((uintptr)wr_pt, NULL, false, NULL, new_pt);
	}

	// Are we mapping or unmapping?
	if(unmap) {
		wr_pt[pt_ent] &= 0xfffffffe;
		invlpg(virtaddr);
	} else {
		bool created = false;
		// Create the page if we need to
		if(physaddr == 0) {
			physaddr = a.pma->BeginAlloc();

			// If we're mapping to a different address space, we need to map it here as well to sort out the pmem stack
			if(asp != NULL) {
				MapPage(temp_p.Base, NULL, false, NULL, physaddr);
			}

			created = true;
		}

		// Create the actual mapping
		wr_pt[pt_ent] = (physaddr & PAGEMASK) | P_PRESENT;
		if(user)
			wr_pt[pt_ent] |= P_USER;
		if(writable)
			wr_pt[pt_ent] |= P_WRITABLE;
		if(cache_disabled)
			wr_pt[pt_ent] |= P_CACHE_DISABLED;
		if(write_through)
			wr_pt[pt_ent] |= P_WRITE_THROUGH;

		// invalidate tlb
		invlpg(virtaddr);

		// Now complete the physical page allocation process, if necessary
		if(created) {
			if(asp == NULL)
				a.pma->EndAlloc(virtaddr & PAGEMASK);
			else {
				a.pma->EndAlloc(temp_p.Base);
				MapPage(temp_p.Base, NULL, true);
			}
		}
	}

	// Unmap temporary pages if necessary
	if(asp != NULL) {
		MapPage((uintptr)wr_pd, NULL, true);
		MapPage((uintptr)wr_pt, NULL, true);

		this->temp_lock.Unlock();
	}

	return NULL;
}

uintptr i386_VirtMemManager::TempMap(physaddr_t paddr)
{
	temp_lock.Lock();
	MapPage(temp_p.Base, NULL, false, NULL, paddr);
	return temp_p.Base;
}

void i386_VirtMemManager::TempUnmap()
{
	MapPage(temp_p.Base, NULL, true);
	temp_lock.Unlock();
}
