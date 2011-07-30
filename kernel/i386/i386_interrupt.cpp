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

#include "i386/interrupt.h"
#include "sys/vmem.h"
#include "sys/arch.h"
#include "sys/types.h"
#include "sys/formatter.h"

extern Arch a;
extern struct i386_Interrupt::idt_entry idt[256];

extern void (*int_0eh_handler)(void);
extern void (*int_0dh_handler)(void);

#define SELECTOR_CODE0_RPL0			0x8


void page_fault(unsigned long addr, unsigned long error)
{
	a.vmm->ResolvePageFault(addr, (error & 0x2) ? true : false, (error & 0x1) ? true : false,
		(error & 0x4) ? true : false);
}

void gp_fault(unsigned long cs, unsigned long eip, unsigned long error)
{
	ReversibleString<256> s;

	Formatter::fPrintf(&s, "General Protection Fault (%x) at %x:%x", error, cs, eip);
	a.Panic(s.GetString());
}

void i386_Interrupt::AddGate(int vector, void (**int_handler)(void), bool user_mode, bool trap_gate)
{
	__asm volatile ( "cli;" );
	::idt[vector].seg_selector = (unsigned short int)SELECTOR_CODE0_RPL0;
	::idt[vector].offset_low = (unsigned short int)((uintptr)int_handler & 0xffff);
	::idt[vector].offset_high = (unsigned short int)(((uintptr)int_handler >> 16) & 0xffff);

	unsigned short int flags = 0x8e00;
	if(user_mode)
		flags |= 0x6000;
	if(trap_gate)
		flags |= 0x100;
	::idt[vector].flags = flags;
	__asm volatile ( "sti;" );
}

void i386_Interrupt::Init()
{
	AddGate(0x0e, &int_0eh_handler);
	AddGate(0x0d, &int_0dh_handler, false, false);
}
