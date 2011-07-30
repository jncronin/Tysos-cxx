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
 

extern page_fault
extern gp_fault
global int_0eh_handler
global int_0dh_handler

int_0eh_handler:
	push	ebp
	mov		ebp, esp
	pushad
	
	push dword	[ebp+4]
	mov		eax, cr2
	push	eax
	call	page_fault
	add		esp, 8
	
	popad
	pop		ebp
	add		esp, 4
	iretd

int_0dh_handler:
	push	ebp
	mov		ebp, esp
	pushad
	
	push dword	[ebp+4]
	push dword	[ebp+8]
	push dword	[ebp+12]
	call	gp_fault
	add		esp, 12
	
	popad
	pop		ebp
	add		esp, 4
	iretd
	