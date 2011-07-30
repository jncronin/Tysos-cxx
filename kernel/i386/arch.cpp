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

#include "sys/arch.h"
#include "drv/vga.h"
#include "i386/pmem.h"
#include "i386/vmem.h"
#include "i386/task.h"
#include "i386/interrupt.h"
#include "drv/pic_8259a.h"
#include "drv/pit_8253.h"

static Vga vga;
static i386_PhysMemAllocator i386_pma;
static i386_VirtMemManager i386_vmm;
static i386_AddressSpace i386_kst_asp;
static i386_TaskSwitcher i386_tswitch;

void Arch::Init()
{
	kconsole = &vga;
	pma = &i386_pma;
	vmm = &i386_vmm;
	kst_asp = &i386_kst_asp;
	tswitch = &i386_tswitch;
	i386_Interrupt::Init();
}

void Arch::InitDevices()
{
	irqs = new Pic_8259a();
	timer = new Pit_8253();
}

void Arch::Halt()
{
	__asm volatile ( "halt: cli; hlt; jmp halt;" );
}

void Arch::Panic(const char *reason)
{
	if(reason == NULL)
		this->kconsole->Printf("Kernel Panic!\n\n");
	else
		this->kconsole->Printf("Kernel Panic: %s\n\n", reason);

	unsigned long int eax, ebx, ecx, edx, esp, cs, ds, es, fs, gs, ss, cr0, cr2, cr3, cr4, esi, edi, ebp, eip;

	__asm volatile ( "nop" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) );
	__asm volatile ( "movl %%esp, %%eax; movl %%cs, %%ecx; movl %%ds, %%edx" :
		"=a" (esp), "=c" (cs), "=d" (ds) );
	__asm volatile ( "movl %%es, %%eax; movl %%fs, %%ebx; movl %%gs, %%ecx; movl %%ss, %%edx" :
		"=a" (es), "=b" (fs), "=c" (gs), "=d" (ss) );
	__asm volatile ( "movl %%cr0, %%eax; movl %%cr2, %%ebx; movl %%cr3, %%ecx; movl %%cr4, %%edx" :
		"=a" (cr0), "=b" (cr2), "=c" (cr3), "=d" (cr4) );
	__asm volatile ( "movl %%ebp, %%eax; movl %%edi, %%ebx; movl %%esi, %%ecx; movl 4(%%ebp), %%edx" :
	"=a" (ebp), "=b" (edi), "=c" (esi), "=d" (eip) );

	this->kconsole->Printf("EAX: %x   EBX: %x   ECX: %x   EDX: %x\n", eax, ebx, ecx, edx);
	this->kconsole->Printf("ESP: %x   EBP: %x   EDI: %x   ESI: %x\n", esp, ebp, edi, esi);
	this->kconsole->Printf("CS:  %x   DS:  %x   ES:  %x   FS:  %x\n", cs, ds, es, fs);
	this->kconsole->Printf("GS:  %x   SS:  %x\n", gs, ss);
	this->kconsole->Printf("Return EIP: %x\n", eip);
	this->kconsole->Printf("CR0: %x   CR2: %x   CR3: %x   CR4: %x\n", cr0, cr2, cr3, cr4);

	Halt();
}

TaskSwitchInfo *Arch::CreateTaskSwitchInfo()
{
	return new i386_TaskSwitchInfo;
}
