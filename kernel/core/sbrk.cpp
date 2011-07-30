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

#include "stdlib.h"
#include "sys/kernel.h"
#include "errno.h"
#include "sys/section.h"
#include "sys/arch.h"
#include "list.h"

extern Arch a;
extern Kernel k;
static Section kheap;
static const char kheap_str[] = "Kernel heap";

void *sbrk(intptr d)
{
	if(k.Heap == NULL) {
		// Determine size of kernel space
		uintptr ksize = a.vmm->GetKernelEnd() - a.vmm->GetKernelStart();

		// Ask for half of it as a heap
		kheap.Name = (char *)kheap_str;
		kheap.Base = 0;
		kheap.Length = 0;
		kheap.ExpandDown = 0;
		kheap.ExpandUp = (ksize / 2) & a.vmm->GetPageMask();
		kheap.UpperPadding = kheap.LowerPadding = a.vmm->GetPageSize();
		kheap.Flags = Section::Defined | Section::Read | Section::Write | Section::KernelSection | Section::Heap;

		Section *ks = NULL;

		while((ks == NULL) && (kheap.ExpandUp >= a.vmm->GetPageSize())) {
			ks = k.Sections->FindFree(a.vmm, &kheap);

			if(ks == NULL)
				kheap.ExpandUp /= 2;
		}

		if(ks == NULL) {
			a.kconsole->Puts("sbrk ERROR: couldn't find space to allocate kernel heap");
			errno = ENOMEM;
			return (void *)-1;
		}

		// Add the new kernel heap
		k.Sections->Add(*ks);
		k.Heap = ks;

		if(k.Heap == NULL) {
			a.kconsole->Puts("sbrk ERROR: couldn't add kernel heap to kernel section list");
			errno = ENOMEM;
			return (void *)-1;
		}

		a.kconsole->Printf("Kernel heap created at %x\n", ks->Base);
	}

	// k.Heap points to our new heap
	void *ret;
	if(d >= 0) {
		if((k.Heap->ExpandUp - k.Heap->Length) < (uintptr)d) {
			// Request for more than is available
			errno = ENOMEM;
			return (void *)-1;
		}
		ret = (void *)(k.Heap->Base + k.Heap->Length);
		k.Heap->Length += (uintptr)d;
	} else {
		uintptr shr_d = (uintptr)(-d);
		if(k.Heap->Length < shr_d) {
			// Request to free more than we can
			errno = ENOMEM;
			return (void *)-1;
		}
		ret = (void *)(k.Heap->Base + k.Heap->Length);
		k.Heap->Length -= shr_d;
	}

	return ret;
}
