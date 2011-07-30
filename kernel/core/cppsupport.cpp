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

#include "sys/types.h"
#include "sys/multiboot.h"
#include "stdlib.h"
#include <reent.h>

static struct _reent global_reent;
struct _reent *_impure_ptr = &global_reent;

int kmain(mb_header *mb_addr, unsigned long int mb_magic);
extern "C" int kstart(mb_header *mb_addr, unsigned long int mb_magic);

extern uintptr start_ctors, start_dtors, end_ctors, end_dtors;
extern uintptr start_eh_frame;
extern "C" void __register_frame(uintptr *address_of_eh_frames);

/*extern "C" void __cxa_pure_virtual()
{
}

void operator delete(void *)
{
}

void *operator new(unsigned long size)
{
	return malloc(size);
}

void *operator new[](unsigned long size)
{
	return malloc(size);
} */

extern "C"
        {
        int __cxa_atexit(void (*f)(void *), void *p, void *d);
        void __cxa_finalize(void *d);
        }

void *__dso_handle; /*only the address of this symbol is taken by gcc*/

struct object
{
        void (*f)(void*);
        void *p;
        void *d;
} object[32];
unsigned int iObject = 0;

int __cxa_atexit(void (*f)(void *), void *p, void *d)
{
        if (iObject >= 32) return -1;
        object[iObject].f = f;
        object[iObject].p = p;
        object[iObject].d = d;
        ++iObject;
        return 0;
}

/* This currently destroys all objects */
void __cxa_finalize(void __attribute__ ((__unused__)) *d)
{
        unsigned int i = iObject;
        for (; i > 0; --i)
        {
                --iObject;
                object[iObject].f(object[iObject].p);
        }
}

int kstart(mb_header *mb_addr, unsigned long int mb_magic)
{
	for(uintptr *call = &start_ctors; call < &end_ctors; call++) {
		((void (*)(void))*call)();
	}

	int ret = kmain(mb_addr, mb_magic);

	for(uintptr *call = &start_dtors; call < &end_dtors; call++) {
		((void (*)(void))*call)();
	}

	return ret;
}

void register_eh_frames()
{
	__register_frame(&start_eh_frame);
}

