;/* Copyright (c) 2007, John Cronin
; * All rights reserved.
; *
; * Redistribution and use in source and binary forms, with or without
; * modification, are permitted provided that the following conditions are met:
; *     * Redistributions of source code must retain the above copyright
; *       notice, this list of conditions and the following disclaimer.
; *     * Redistributions in binary form must reproduce the above copyright
; *       notice, this list of conditions and the following disclaimer in the
; *       documentation and/or other materials provided with the distribution.
; *     * Neither the name of the copyright holder nor the
; *       names of its contributors may be used to endorse or promote products
; *       derived from this software without specific prior written permission.
; *
; * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
; * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
; * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

global do_switch
global c_switch
global perform_iret

perform_iret:
	iret

c_switch:
	; void c_switch(new_asp->address_space, *new_tsi->esp0, *old_tsi->esp0, Thread **CurrentThread, Thread *NewThread)
	mov			eax, [esp+4]
	mov			ecx, [esp+8]
	mov			edx, [esp+12]
	mov			edi, [esp+16]
	mov			esi, [esp+20]
	call		do_switch
	
	ret

do_switch:
	; eax = new_asp->address_space
	; ecx = &new_tsi->esp0
	; edx = &old_tsi->esp0
	; edi = Thread **CurrentThread
	; esi = Thread *NewThread
	
	cli
	
	push		gs
	push		fs
	push		es
	push		ds
	pushad
	
	test		eax, 0xffffffff
	jz			.otsi
	mov			cr3, eax
	
.otsi:
	test		edx, 0xffffffff
	jz			.ntsi
	mov			[edx], esp
	
.ntsi:
	test		ecx, 0xffffffff
	jz			.ct
	mov			esp, [ecx]
	
.ct:
	test		edi, 0xffffffff
	jz			.dopop
	mov			[edi], esi
	
.dopop
	popad
	pop			ds
	pop			es
	pop			fs
	pop			gs
	
	sti
	
	ret
