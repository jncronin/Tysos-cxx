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

#include "i386/task.h"
#include "sys/arch.h"
#include "sys/vmem.h"
#include "sys/pmem.h"

extern Arch a;
extern "C" void c_switch(physaddr_t address_space, uintptr *new_esp0, uintptr *old_esp0, Thread **curr_thread,
						 Thread *new_thread);
extern "C" void perform_iret();
extern Thread *CurrentThread;

struct tss
{
	unsigned short int previous_task;
	unsigned short int r1;
	unsigned long int esp0;
	unsigned short int ss0;
	unsigned short int r2;
	unsigned long int esp1;
	unsigned short int ss1;
	unsigned short int r3;
	unsigned long int esp2;
	unsigned short int ss2;
	unsigned short int r4;
	unsigned long int cr3;
	unsigned long int eip;
	unsigned long int eflags;
	unsigned long int eax;
	unsigned long int ecx;
	unsigned long int edx;
	unsigned long int ebx;
	unsigned long int esp;
	unsigned long int ebp;
	unsigned long int esi;
	unsigned long int edi;
	unsigned short int es;
	unsigned short int r5;
	unsigned short int cs;
	unsigned short int r6;
	unsigned short int ss;
	unsigned short int r7;
	unsigned short int ds;
	unsigned short int r8;
	unsigned short int fs;
	unsigned short int r9;
	unsigned short int gs;
	unsigned short int r10;
	unsigned short int ldt;
	unsigned short int r11;
	unsigned short int t;
	unsigned short int iomap;
} __attribute__((packed));

extern struct tss tss;

void i386_TaskSwitchInfo::Init(uintptr entry_address, Section *Stack, void *param, Section *KernelStack)
{
	// Init should:
	// - If kernel mode, push _exit address to kernel stack, then param, else push these to user stack
	// - Set up kernel mode stack, with all registers + eflags + return address pushed
	// - Point esp0 to bottom of kernel stack, esp0_top to highest address of kernel stack

	unsigned long int *p_st = (unsigned long int *)(Stack->Base + Stack->Length);
	*--p_st = 0; // value passed to __exit
	*--p_st = (unsigned long int)param;
	*--p_st = (unsigned long int)__exit;

	unsigned long int d_seg, c_seg;

	unsigned long int *k_st;
	if(KernelStack != NULL) {
		d_seg = 0x23;
		c_seg = 0x1b;

		k_st = (unsigned long int *)(KernelStack->Base + KernelStack->Length);
		this->esp0_top = (uintptr)k_st;
		*--k_st = d_seg;
		*--k_st = (unsigned long int)p_st;
	} else {
		d_seg = 0x10;
		c_seg = 0x08;
		k_st = p_st;
	}

	/* For the first switch to a task, we pretend it was interrupted by an irq,
	    which called the isr 'perform_iret', which in turn called do_switch.  
		Thus, the isr stuff will be pushed, followed by the return address in
		'perform_iret', which is basically just the address of an 'iret' instruction.

		Thus, the ret at the end of do_switch will return to perform_iret, which
		will return to the entry point */

	*--k_st = i386_TaskSwitchInfo::IF;		// EFLAGS
	*--k_st = c_seg;
	*--k_st = entry_address;

	*--k_st = (unsigned long int)perform_iret;

	*--k_st = d_seg;	//gs
	*--k_st	= d_seg;	//fs
	*--k_st = d_seg;	//es
	*--k_st = d_seg;	//ds

	*--k_st = 0x0;	//edi
	*--k_st = 0x0;	//esi
	*--k_st = 0x0;  //ebp
	*--k_st = 0x0;	//esp (ignored)
	*--k_st = 0x0;  //ebx
	*--k_st = 0x0;  //edx
	*--k_st = 0x0;  //ecx
	*--k_st = 0x0;	//eax

	this->esp0 = (uintptr)k_st;
}

void i386_TaskSwitcher::Switch(Thread *new_thread)
{
	// Task switch should:
	//  - Push all regs
	//  - Save old cr3 if necessary
	//  - Update cr3 if necessary
	//  - Save esp to old_tsi->cr3
	//  - Update esp from new_tsi->cr3
	//  - Pop all regs
	//  - Update the esp0 value in tss with the highest address of the new kernel mode stack (new_tsi->esp0_top)

	__asm volatile ( "cli; ");

	tss.esp0 = ((i386_TaskSwitchInfo *)new_thread->tsi)->esp0_top;
	/*c_switch(new_thread->tsi,
		(new_tsi != NULL) ? &(((i386_TaskSwitchInfo *)new_tsi)->esp0) : NULL,
		(old_tsi != NULL) ? &(((i386_TaskSwitchInfo *)old_tsi)->esp0) : NULL);*/

	c_switch((new_thread == CurrentThread) ? NULL : (new_thread->proc->Address_Space->addr_space),
		(((i386_TaskSwitchInfo *)new_thread->tsi) == NULL) ? NULL :
		&(((i386_TaskSwitchInfo *)new_thread->tsi)->esp0),
		(((i386_TaskSwitchInfo *)CurrentThread->tsi) == NULL) ? NULL :
		&(((i386_TaskSwitchInfo *)CurrentThread->tsi)->esp0),
		&CurrentThread,
		new_thread);

/*	__asm volatile (
		"pushl %gs;"
		"pushl %fs;"
		"pushl %es;"
		"pushl %ds;"
		"pushal;" );

	if(old_asp != NULL) {
		__asm volatile (
			"movl %%cr3, %%eax;"
			"movl %%eax, %0;" : "=m" (old_asp->addr_space) );
	}
	if(new_asp != NULL) {
		__asm volatile (
			"movl %0, %%eax;"
			"movl %%eax, %%cr3;" : : "m" (new_asp->addr_space) );
	}

	if(old_tsi != NULL) {
		__asm volatile (
			"movl %%esp, %%eax;"
			"movl %%eax, %0;": "=m" (((i386_TaskSwitchInfo *)old_tsi)->esp0) );
	}

	__asm volatile (
		"movl %0, %%eax;"
		"movl %%eax, %%esp;"  : : "m" (((i386_TaskSwitchInfo *)new_tsi)->esp0) );
	
	__asm volatile (
		"popal;"
		"pop %ds;"
		"pop %es;"
		"pop %fs;"
		"pop %gs;" ); */

}
