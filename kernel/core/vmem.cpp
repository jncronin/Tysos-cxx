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

#include "sys/vmem.h"
#include "sys/task.h"
#include "sys/arch.h"
#include "sys/section.h"
extern Arch a;
#include "sys/sched.h"
#include "sys/concurrent.h"

extern Thread *CurrentThread;

void VirtMemManager::ResolvePageFault(uintptr fault_address, bool write_error, bool protection_error, bool user_mode)
{
	// If its a page not present fault, check for a matching section
	if(!protection_error) {
		int i, c;
		c = CurrentThread->proc->Sections->Count();
		Section::Extent ae;
		ae.Lower = fault_address;
		ae.Upper = fault_address;

		protection_error = true; // Set this back to false if we find a valid section

		for(i = 0; i < c; i++) {
			Section::Extent se;
			Section &s = CurrentThread->proc->Sections->GetItem(i);
			if(s.CheckExtents(&se)) {
				if(ae == se) {
					// Its part of this section.  Allow expanding sections
					if(s.Flags & Section::AutoExpand) {
						if((fault_address < s.Base) && ((s.Base - fault_address) <= s.ExpandDown)) {
							s.ExpandDown -= (s.Base - fault_address);
							s.Base = fault_address;
							protection_error = false;
						}
						if((fault_address >= (s.Base + s.Length)) && ((fault_address - (s.Base + s.Length)) <= s.ExpandUp)) {
							s.ExpandUp -= (fault_address - (s.Base + s.Length));
							s.Length = fault_address - s.Base + 1;
							protection_error = false;
						}
					} 
					if((fault_address >= s.Base) && (fault_address < (s.Base + s.Length)))
						protection_error = false;

					if(!protection_error) {
						// Map the page
						physaddr_t paddr = NULL;
						if(s.Flags & Section::Bits)
							paddr = (fault_address - s.Base + s.MemSource) & GetPageMask();
		
						MapPage(fault_address & GetPageMask(), NULL, false, NULL,
							(fault_address - s.Base + s.MemSource) & GetPageMask(),
							(s.Flags & Section::KernelSection) ? false : true,
							(s.Flags & Section::Write) ? true : false);

						break;
					}
				}
			}
		}
	}
			
	// Check for errors
	if(protection_error) {
		// A bad error
		a.kconsole->Printf("PAGE FAULT! Invalid attempt by ");
		if(user_mode)
			a.kconsole->Printf("process ");
		else
			a.kconsole->Printf("kernel ");
		a.kconsole->Printf("to ");
		if(write_error)
			a.kconsole->Printf("write to ");
		else
			a.kconsole->Printf("read from ");
		a.kconsole->Printf("address %x whilst executing %s\n", fault_address, CurrentThread->proc->name);
		a.Panic();		
	}
}
