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

#include <sys/types.h>
#include "string.h"
#include "stdlib.h"
#include "sys/arch.h"

extern Arch a;

void *memcpy(void *dest, const void *src, size_t n)
{
	size_t i;
	char *s = (char *)src;
	char *d = (char *)dest;
	for(i = 0; i < n; i++)
		d[i] = s[i];
	return dest;
}

void *memset(void *s, int c, size_t n)
{
	size_t i;
	char *d = (char *)s;
	for(i = 0; i < n; i++)
		d[i] = (char)(c & 0xff);
	return s;
}

unsigned long int errno;

void abort()
{
	a.kconsole->Puts("Abort() signalled");
	while(1);
}

int strcmp(const char *string1, const char *string2)
{
	while(*string1 != '\0') {
		if(*string1 != *string2)
			return (int)*string1 - (int)*string2;
		string1++;
		string2++;
	}
	return (int)*string1 - (int)*string2;
}

size_t strlen(const char *s)
{
	size_t n = 0;
	while(*s != '\0') {
		n++;
		s++;
	}
	return n;
}

extern "C" size_t write(int fd, const void *buf, size_t count)
{
	size_t ret = count;
	if(fd == 2) {
		char *s = (char *)buf;
		while(count > 0) {
			*a.kconsole << *s;
			s++;
			count--;
		}
	}
	return ret;
}

extern "C" int fputs(const char *s, void __attribute__ ((__unused__)) *stream)
{
	*a.kconsole << s;
	return 1;
}

extern "C" int fputc(int c, void __attribute__ ((__unused__)) *stream)
{
	*a.kconsole << (char)c;
	return c;
}

extern "C" size_t fwrite(const void *ptr, size_t size, size_t nmemb, void __attribute__ ((__unused__)) *stream)
{
	write(2, ptr, size * nmemb);
	return size;
}

extern "C" char *strcpy(char *dest, const char *src)
{
	memcpy(dest, src, strlen(src));
	return dest;
}

extern "C" char *strcat(char *dest, const char *src)
{
	int d_len = strlen(dest);
	int s_len = strlen(src);
	strcpy(dest + d_len, src);
	dest[d_len + s_len] = '\0';
	return dest;
}
