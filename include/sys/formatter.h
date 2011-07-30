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

#ifndef FORMATTER_H
#define FORMATTER_H

#include "stdarg.h"
#include "stream.h"

class Formatter {
public:
	static int vfPrintf(IStream *s, const char *format, va_list ap);
	static int fPrintf(IStream *s, const char *format, ...);
	static void NumToStr(IStream *s, unsigned long int n, int base = 10);
	static void NumToChar(IStream *s, unsigned long int n);
};

// To display numbers as strings, formatter requires a simple stream class into which
// writes can be reversed.  It is designed to be used before malloc/new are available
// and therefore uses a static array as the string.  We assume the maximum length to
// convert is 64 bits, in base 2 that implies an array of 64 chars, plus null.
#define RS_MAX 65
template <int max = RS_MAX> class ReversibleString : public Stream
{
public:
	bool Forwards;
	
	virtual void Write(const char *s, 
		bool __attribute__ ((__unused__)) batch = false) { while(*s != '\0') Write(*s); };

	virtual void Write(char ch, bool batch = false)
	{
		if(Forwards) {
			if(pos >= (max - 1))
				return;
			str[pos] = ch;
			pos++;
			if(batch == false)
				str[pos] = '\0';
		} else {
			if(pos >= (max - 1))
				pos = max - 2;
			if(pos == 0)
				pos = max - 2;
			str[pos] = ch;
			pos--;
		}
	}

	virtual void EndBatch()
	{
		if(Forwards)
			str[pos] = '\0';
	}

	virtual char *GetString()
	{
		if(Forwards) {
			EndBatch();	// null terminate the string
			return str;
		} else {
			str[max - 1] = '\0';		// ensure the string is still null-terminated
			return &str[pos + 1];
		}
	}

	ReversibleString() { pos = 0; Forwards = true; str[max] = '\0'; };
private:
	char str[max];
	unsigned int pos;
};

#endif
