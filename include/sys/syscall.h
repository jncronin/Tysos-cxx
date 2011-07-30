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

#ifndef SYSCALL_H

#ifdef KERNEL
#include "sys/types.h"
int do_syscall(int scall, uintptr a, uintptr b, uintptr c, uintptr d);
#endif

#define SYSCALL__exit				0
#define SYSCALL_close				1
#define SYSCALL_environ				2
#define SYSCALL_execve				3
#define SYSCALL_fork				4
#define SYSCALL_fstat				5
#define SYSCALL_getpid				6
#define SYSCALL_isatty				7
#define SYSCALL_kill				8
#define SYSCALL_link				9
#define SYSCALL_lseek				10
#define SYSCALL_open				11
#define SYSCALL_read				12
#define SYSCALL_sbrk				13
#define SYSCALL_stat				14
#define SYSCALL_times				15
#define SYSCALL_unlink				16
#define SYSCALL_wait				17
#define SYSCALL_write				18
#define SYSCALL_fread				19
#define SYSCALL_fwrite				20
#define SYSCALL_gettimeofday		21
#define SYSCALL_getwd				22
#define SYSCALL_pathconf			23

#define SYSCALL_H
#endif
