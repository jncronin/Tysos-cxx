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

#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#define SYSCALL_COMMAND				"int $0x80;"
#define SYSCALL0(call)				__asm volatile ( SYSCALL_COMMAND : : "a" (call) )
#define SYSCALL1(call, a)			__asm volatile ( SYSCALL_COMMAND : : "a" (call), "c" (a) )
#define SYSCALL2(call, a, b)		__asm volatile ( SYSCALL_COMMAND : : "a" (call), "c" (a), "d" (b) )
#define SYSCALL3(call, a, b, c)		__asm volatile ( SYSCALL_COMMAND : : "a" (call), "c" (a), "d" (b), "S" (c) )
#define SYSCALL4(call, a, b, c, d)	__asm volatile ( SYSCALL_COMMAND : : "a" (call), "c" (a), "d" (b), "S" (c), "D" (d) )
#define SYSCALL0e(call)				__asm volatile ( SYSCALL_COMMAND : "=c" (errno) : "a" (call) )
#define SYSCALL1e(call, a)			__asm volatile ( SYSCALL_COMMAND : "=c" (errno) : "a" (call), "c" (a) )
#define SYSCALL2e(call, a, b)		__asm volatile ( SYSCALL_COMMAND : "=c" (errno) : "a" (call), "c" (a), "d" (b) )
#define SYSCALL3e(call, a, b, c)	__asm volatile ( SYSCALL_COMMAND : "=c" (errno) : "a" (call), "c" (a), "d" (b), "S" (c) )
#define SYSCALL4e(call, a, b, c, d)	__asm volatile ( SYSCALL_COMMAND : "=c" (errno) : "a" (call), "c" (a), "d" (b), "S" (c), "D" (d) )
#define SYSCALL0r(call, r)				__asm volatile ( SYSCALL_COMMAND : "=a" (r) : "a" (call) )
#define SYSCALL1r(call, r, a)			__asm volatile ( SYSCALL_COMMAND : "=a" (r) : "a" (call), "c" (a) )
#define SYSCALL2r(call, r, a, b)		__asm volatile ( SYSCALL_COMMAND : "=a" (r) : "a" (call), "c" (a), "d" (b) )
#define SYSCALL3r(call, r, a, b, c)		__asm volatile ( SYSCALL_COMMAND : "=a" (r) : "a" (call), "c" (a), "d" (b), "S" (c) )
#define SYSCALL4r(call, r, a, b, c, d)	__asm volatile ( SYSCALL_COMMAND : "=a" (r) : "a" (call), "c" (a), "d" (b), "S" (c), "D" (d) )
#define SYSCALL0re(call, r)				__asm volatile ( SYSCALL_COMMAND : "=a" (r), "=c" (errno) : "a" (call) )
#define SYSCALL1re(call, r, a)			__asm volatile ( SYSCALL_COMMAND : "=a" (r), "=c" (errno) : "a" (call), "c" (a) )
#define SYSCALL2re(call, r, a, b)		__asm volatile ( SYSCALL_COMMAND : "=a" (r), "=c" (errno) : "a" (call), "c" (a), "d" (b) )
#define SYSCALL3re(call, r, a, b, c)	__asm volatile ( SYSCALL_COMMAND : "=a" (r), "=c" (errno) : "a" (call), "c" (a), "d" (b), "S" (c) )
#define SYSCALL4re(call, r, a, b, c, d)	__asm volatile ( SYSCALL_COMMAND : "=a" (r), "=c" (errno) : "a" (call), "c" (a), "d" (b), "S" (c), "D" (d) )

#endif
