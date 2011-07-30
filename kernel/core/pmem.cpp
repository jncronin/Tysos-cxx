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
#include "string.h"
#include "sys/concurrent.h"
#include "sys/arch.h"
#include "sys/section.h"
#include "list.h"

extern Arch a;

physaddr_t PhysMemStack::PopPage()
{
	return EndPop(BeginPop());
}

physaddr_t PhysMemStack::BeginPop()
{
	if(this->top == 0)
		return NULL;

	Lock();
	return this->top;
}

physaddr_t PhysMemStack::EndPop(physaddr_t paddr)
{
	this->top = *((physaddr_t *)paddr);
	Unlock();
	return paddr;
}

void PhysMemStack::PushPage(physaddr_t paddr)
{
	if(paddr == 0)
		return;

	obtain_spinlock(&lock, 1);
	*((physaddr_t *)paddr) = this->top;
	this->top = paddr;
	release_spinlock(&lock, 1);
}

PhysMemStack::PhysMemStack()
{
	this->top = NULL;
}

void PhysMemStack::Init()
{
	lock = 0;
}

void PhysMemBitmap::Init(void *bitmap_memory, unsigned long int bitmap_base, unsigned long int bitmap_end,
						 unsigned long int page_size)
{
	this->bmp = bitmap_memory;
	this->base = bitmap_base;
	this->end = bitmap_end;
	this->ps = page_size;
	this->bmp_size = (bitmap_end - bitmap_base) / (page_size * 8);

	// Mark all pages as used
	memset(bmp, 0x0, bmp_size);
}

physaddr_t PhysMemBitmap::Alloc(physaddr_t paddr)
{
	if(paddr < base)
		return NULL;
	if(paddr >= end)
		return NULL;

	int bitno = (paddr - base) / ps;
	int bit = bitno % 32;
	int dw = bitno / 32;

	if(!lock_btr((unsigned long int *)(base + (dw * 4)), bit))
		return NULL;
	return paddr;
}

physaddr_t PhysMemBitmap::Alloc()
{
	uintptr off = (uintptr)bmp;
	while(off <= ((uintptr)bmp + bmp_size)) {
		unsigned long int *b = (unsigned long int *)off;
		if(*b != 0) {
			int i;
			for(i = 0; i < 32; i++) {
				if((*b >> i) & 1) {
					if(lock_btr(b, i)) {
						// we've found a match, return it
						return (physaddr_t)(((off - (uintptr)bmp) * 32 + i) * ps + base);
					}
				}
			}
		}
	}
	return NULL;
}

void PhysMemBitmap::Free(physaddr_t paddr)
{
	if(paddr < base)
		return;
	if(paddr >= end)
		return;

	int bitno = (paddr - base) / ps;
	int bit = bitno % 32;
	int dw = bitno / 32;

	lock_bts((unsigned long int *)((unsigned long int)bmp + (dw * 4)), bit);
}

void PhysMemAllocator::UpdateUsed(SectionList *sections)
{
	// Set all pages defined in a section as used

	int i;
	int c = sections->Count();

	for(i = 0; i < c; i++) {
		Section &s = sections->GetItem(i);

		if((s.Flags & Section::Defined) & (s.Flags & Section::Bits)) {
			physaddr_t p = s.Base & pp->GetPageMask();
			while(p < (s.Base + s.Length)) {
				this->Alloc(p);
				p += pp->GetPageSize();
			}
		}
	}
}

physaddr_t PhysMemBitmap::AllocRange(uintptr length)
{
	// Allocate a contiguous range of pages
	unsigned int pages = (length + ps - 1) / ps;

	unsigned int i;
	unsigned long int *arr = (unsigned long int *)bmp;
	for(i = 0; i < (bmp_size * 8 - pages); i++) {
		// Now we have to check the 'pages' contiguous bits, starting at i
		unsigned int j;
		bool found = true;

		for(j = i; j < (i + pages); j++) {
			int bit, dw;
			dw = j / 32;
			bit = j % 32;

			if(((arr[dw] >> bit) & 0x1) == 0x0) {
				found = false;
				i = j;
				break;
			}
		}

		if(found == true) {
			// We have found space starting at bit i
			return base + i * ps;
		}
	}

	return NULL;
}
