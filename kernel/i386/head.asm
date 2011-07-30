; Copyright (c) 2007, John Cronin
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;     * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;     * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;     * Neither the name of the copyright holder nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY John Cronin ``AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL John Cronin BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

extern kstart
global loader
global idt
global tss

MODULEALIGN		equ		1<<0		; align modules on page boundaries
MEMINFO			equ		1<<1		; provide memory map
FLAGS			equ		MODULEALIGN | MEMINFO
MAGIC			equ		0x1BADB002	; MultiBoot magic
CHECKSUM		equ		-(MAGIC + FLAGS)

STACKSIZE		equ		0x8000		; 32kB of stack space

HEADERLEN		equ		90

section	.multiboot
align	4
mbheader:
	dd		MAGIC
	dd		FLAGS
	dd		CHECKSUM
	
section .text
loader:
; store multiboot info
	mov		[mb_addr], ebx
	mov		[mb_magic], eax
	
; use our own gdt
	lgdt	[gdtinfo]
	jmp		0x8:.owngdt
	
.owngdt:
	mov		eax,	0x10
	mov		ss,		eax
	mov		ds,		eax
	mov		es,		eax
	mov		fs,		eax
	mov		gs,		eax
	
	mov		esp,	stack + STACKSIZE		; top of stack in bss section
	
; set up an idt
	lidt	[idtinfo]
	
; clear the screen
	mov		edi, 0xb8000
	xor		eax, eax
	mov		ecx, 960
	rep		stosd
	
; set up the tss descriptor
; 1st dword
	mov		eax, [tsslimit]
	and		eax, 0xffff
	mov		ebx, tss
	and		ebx, 0xffff
	shl		ebx, 16
	or		eax, ebx
	mov	dword	[tss_ent + 0],	eax
	
; 2nd dword
	mov		eax, tss
	shr		eax, 16
	and		eax, 0xff
	
	mov		ebx, tss
	shr		ebx, 24
	and		ebx, 0xff
	shl		ebx, 24
	or		eax, ebx
	
	or word ax, 1000100100000000b
	
	mov		ebx, [tsslimit]
	shr		ebx, 16
	and		ebx, 0xff
	or word bx, 00000000b		; G00AVL flags for future use
	shl		ebx, 16
	or		eax, ebx
	
	mov dword	[tss_ent + 4], eax
	
	mov		eax, 0x28
loadtss:
	ltr		ax
	
disableints:
	mov		al, 0xff
	out		0x21, al
	out		0xa1, al
	
cwelcome:
	mov		eax, [mb_magic]
	mov		ebx, [mb_addr]
	push	eax								; multiboot magic
	push	ebx								; multiboot header address
	
	call	kstart

	cli
halt:	hlt
	jmp halt

section .data
align 32
idtinfo:
	dw		2047			; 256 entries of 8 bytes
	dd		idt			; linear address. idt at 1.5MB

gdtinfo:
        dw 		gdt_end - gdt - 1
        dd 		gdt		; linear address
               
section .tss
align 32
tss:
; entries marked (*) must be set before a task switch (from this task)
dw		0,				0		; previous task link,	reserved
dd		0						; esp0 (*)
dw		0x10,			0		; ss0 (*), reserved
dd		0						; esp1 (*) - not used
dw		0,				0		; ss1 (*) - not used, reserved
dd		0						; esp2 (*) - not used
dw		0,				0		; ss2 (*) - not used, reserved
dd		0						; cr3 (*)
dd		0, 0, 0, 0, 0, 0, 0, 0, 0, 0	; eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi
dd		0, 0, 0, 0, 0, 0		; es + reserved, cs + reserved, ss + reserved, ds + reserved
								; fs + reserved, gs + reserved
dw		0,				0		; ldt selector (*) - not used, reserved
dw		0,				0xffff	; debug flag + reserved, iopl map (*) - not used
tss_end:

; the limit of the tss for incorporation in a gdt
tsslimit	dd	tss_end - tss - 1

section .idt
align 32
idt:	times 2048 db 0
        
section .gdt
align 32
gdt    dd      0,0     ; null
code    db      0xff, 0xff, 0x00, 0x00, 0x00, 10011010b, 11001111b, 0x00
data    db      0xff, 0xff, 0x00, 0x00, 0x00, 10010010b, 11001111b, 0x00
code3	db		0xff, 0xff, 0x00, 0x00, 0x00, 11111110b, 11001111b, 0x00
data3	db		0xff, 0xff, 0x00, 0x00, 0x00, 11110010b, 11001111b, 0x00
tss_ent	dd		0, 0
.end

times (2048 - .end + gdt) db 0
gdt_end:

section .bss
align 32
stack: resb STACKSIZE
mb_magic: resd	1
mb_addr: resd	1


