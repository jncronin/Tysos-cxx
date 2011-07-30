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

#include "sys/multiboot.h"
#include "sys/arch.h"
#include "sys/kernel.h"
Arch a;
#include "sys/sched.h"
#include "drv/drv.h"

#include "stdlib.h"
#include <typeinfo>
#include <exception>

extern Multiboot mboot;
Kernel k;
Thread *CurrentThread;

Scheduler<> *scheduler;

extern void register_eh_frames();
extern "C" int idle_loop(void *param);

Thread *ta, *tb;

void testa(void __attribute__((__unused__)) *param)
{
	uintptr i = 0;

	while(1) {
		uintptr esp;

		__asm volatile ( "movl %%esp, %0;" : "=r" (esp) );
		a.kconsole->Printf("a (%u): esp = %x\n", i, esp);
		i++;
		//scheduler->Sleep(ta, 100000000LL);
		//scheduler->ScheduleNext((unsigned long long int)-1, CurrentThread, a.tswitch);
		//a.tswitch->Switch(tb);
	}
}

void testb(void __attribute__((__unused__)) *param)
{
	while(1) {
		//uintptr esp;

		//__asm volatile ( "movl %%esp, %0;" : "=r" (esp) );
		//a.kconsole->Printf("b: esp = %x\n", esp);

		//*a.kconsole << 'b';
		//a.tswitch->Switch(ta);
	}
}

int kmain(mb_header *mb_addr, unsigned long int mb_magic)
{
	// Set the current thread to NULL, to allow spinlock to function correctly
	CurrentThread = NULL;

	// Initialise the particular architecture we're using
	a.Init();

	// Check we're loaded from a multiboot-compliant loader
	if(mb_magic != 0x2badb002) {
		a.kconsole->Puts("Error: not loaded from a multiboot-compliant loader");
		return -1;
	}

	// Parse the multiboot headers
	mboot.Parse(mb_addr);

	// Set up the physical memory allocator
	a.pma->Init(&mboot, 1, 0);

	// Configure the kernel process
	k.InitKernel(&mboot);

	// Ask the default console driver to allocate any memory its using
	IMemoryResourceUser *mru = dynamic_cast<IMemoryResourceUser *>(a.kconsole);
	if(mru != NULL)
		mru->RegisterMemory(k.Sections);

	// Have the physical memory allocator mark all used pages as used
	a.pma->UpdateUsed(k.Sections);

	// Initialise the virtual memory manager
	a.vmm->Init(k.Sections);

	// Enable paging
	a.kst_asp->Init();
	a.vmm->SetAddressSpace(a.kst_asp);
	a.vmm->Enable();

	// Register the exception handler frames
	register_eh_frames();

	// Convert the Kernel Section list to a LinkedList structure
	k.Sections->ConvertToLinkedList();

	// Create a few more devices now we have new() working
	a.InitDevices();

	// Create the scheduler
	scheduler = new Scheduler<>();

	// Dump the sections
	*a.kconsole << k.Sections;

	// Report success
	*a.kconsole << "System initialisation complete\n";

	// Create the idle loop
	Thread *idle = Thread::Create(&k, (uintptr)idle_loop, 0, "Idle task");
	idle->priority = 0;
	scheduler->Reschedule(idle);

	// Create some test threads
	scheduler->Reschedule(ta = Thread::Create(&k, (uintptr)testa, 0, "TestA"));
	scheduler->Reschedule(tb = Thread::Create(&k, (uintptr)testb, 0, "TestB"));
	
	// Start timer
	*a.kconsole << "Starting timer\n";
	a.irqs->EnableAll();
	//scheduler->ScheduleNext((unsigned long long int)-1, CurrentThread, a.tswitch);
	a.timer->Enable();

	while(1);

	return 0;
}
