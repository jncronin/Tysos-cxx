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

#ifndef I386_IO_H
#define I386_IO_H

static __inline void out(unsigned short port, unsigned char val)
{
	__asm volatile ( "outb %0,%1" :: "a"(val), "Nd"(port) );
}

static __inline void out(unsigned short port, unsigned short int val)
{
	__asm volatile ( "outb %0,%1" :: "a"(val), "Nd"(port) );
}

static __inline void out(unsigned short port, unsigned long int val)
{
	__asm volatile ( "outb %0,%1" :: "a"(val), "Nd"(port) );
}

static __inline unsigned char inb(unsigned short port)
{
	unsigned char ret;
	__asm volatile( "inb %1, %0" : "=a"(ret) : "Nd" (port) );
	return ret;
}

static __inline unsigned short int inw(unsigned short port)
{
	unsigned short int ret;
	__asm volatile( "inw %1, %0" : "=a"(ret) : "Nd" (port) );
	return ret;
}

static __inline unsigned long int ind(unsigned short port)
{
	unsigned long int ret;
	__asm volatile( "inl %1, %0" : "=a"(ret) : "Nd" (port) );
	return ret;
}

#endif
