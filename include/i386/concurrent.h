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

#ifndef I386_CONCURRENT_H
#define I386_CONCURRENT_H

__inline void idle() {
	__asm volatile ( "hlt" );
}

__inline int lock_bts(unsigned long int *byte, int bit) {
	int ret;
	__asm volatile ( "lock bts %1, %2; xorl %0, %0; adcl $0, %0" : "=r" (ret) : "r" (bit), "m" (*byte) : "cc" );
	return ret;
}

__inline int lock_btc(unsigned long int *byte, int bit) {
	int ret;
	__asm volatile ( "lock btc %1, %2; xorl %0, %0; adcl $0, %0" : "=r" (ret) : "r" (bit), "m" (*byte) : "cc" );
	return ret;
}

__inline int lock_btr(unsigned long int *byte, int bit) {
	int ret;
	__asm volatile ( "lock btr %1, %2; xorl %0, %0; adcl $0, %0" : "=r" (ret) : "r" (bit), "m" (*byte) : "cc" );
	return ret;
}

__inline unsigned long int obtain_spinlock(unsigned long int *lock, unsigned long int thread_id) {
	__asm volatile ( "spin_lock: cmpl $0, %0;"
					 "je get_lock;"
					 "cli;"
					 "cmpl %1, %0;"
					 "je get_lock2;"
					 "sti;"
					 "pause;"
					 "jmp spin_lock;"
					 "get_lock: cli;"
					 "lock cmpxchgl %1, %0;"
					 "jz get_lock2;"
					 "sti;"
					 "jmp spin_lock;"
					 "get_lock2:" : : "m" (*lock), "r" (thread_id), "a" (0x0) : "cc" );
	return thread_id;
}

__inline unsigned long int obtain_spinlock_noint(unsigned long int *lock, unsigned long int thread_id) {
	__asm volatile ( "spin_lock_ni: cmpl $0, %0;"
					 "je get_lock_ni;"
					 "cmpl %1, %0;"
					 "je get_lock2_ni;"
					 "pause;"
					 "jmp spin_lock_ni;"
					 "get_lock_ni: lock cmpxchgl %1, %0;"
					 "jnz spin_lock_ni;"
					 "get_lock2_ni:" : : "m" (*lock), "r" (thread_id), "a" (0x0) : "cc" );
	return thread_id;
}

__inline unsigned long int release_spinlock(unsigned long int *lock, unsigned long int thread_id) {
	__asm volatile ( "cmpxchgl %0, %1;"
					 "jnz cont;"
					 "sti;"
					 "cont:" : : "r" (0x0), "m" (*lock), "a" (thread_id) : "cc" );
	return 0;
}

__inline unsigned long int release_spinlock_noint(unsigned long int *lock, unsigned long int thread_id) {
	__asm volatile ( "cmpxchgl %0, %1;" : : "r" (0x0), "m" (*lock), "a" (thread_id) : "cc" );
	return 0;
}

__inline bool ints_enabled() {
	int eflags;
	__asm volatile ( "pushfl; popl %0" : "=r" (eflags) );
	if(eflags & (1<<9))
		return true;
	return false;
}

#endif
