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

#include "sys/formatter.h"

static const char digits[] = "0123456789abcdef";

int Formatter::fPrintf(IStream *s, const char *format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = Formatter::vfPrintf(s, format, args);
	va_end(args);

	return ret;
}

int Formatter::vfPrintf(IStream *s, const char *format, va_list ap)
{
	char *p = (char *)format;
	int i = 0;
	while(*p != '\0') {
		if(*p == '\\') {
			p++;

			switch(*p) {
				case '\\':
					s->Write('\\', true);
					i++;
					break;
				case 'n':
					s->Write('\n', true);
					i++;
					break;
				case '\'':
					s->Write('\'', true);
					i++;
					break;
				case '\"':
					s->Write('\"', true);
					i++;
					break;
				case 't':
					s->Write('\t', true);
					i++;
					break;
			}
		} else if(*p == '%') {
			p++;

			switch(*p) {
				case 'x':
					// Print hex number
					s->Write('0', true);
					s->Write('x', true);
					Formatter::NumToStr(s, va_arg(ap, unsigned long int), 16);
					i += 10;
					break;
				case 'u':
					// Print unsigned decimal
					Formatter::NumToStr(s, va_arg(ap, unsigned long int), 10);
					i += 12;
					break;
				case 's':
					// Print string
					s->Write(va_arg(ap, const char *), true);
					break;
				case 'c':
					// Print char
					char ch = va_arg(ap, char);
					s->Write(ch, true);
					i++;
					break;
			}
		} else {
			s->Write(*p, true);
			i++;
		}

		p++;
	}

	s->EndBatch();

	return i;
}

void Formatter::NumToStr(IStream *s, unsigned long n, int base)
{
	// Output an integer as a string with a specified base
	ReversibleString<> str;
	str.Forwards = false;
	int digit;
	do {
		digit = n % base;
		NumToChar(&str, digit);
		n /= base;
	} while (n != 0);
	str.EndBatch();
	s->Write(str.GetString(), true);
}

void Formatter::NumToChar(IStream *s, unsigned long n)
{
	if(n >= 16)
		s->Write('X', true);
	else
		s->Write(digits[n], true);
}
