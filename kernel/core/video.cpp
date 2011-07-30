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

#include "sys/video.h"
#include "stdarg.h"
#include "sys/formatter.h"

int Video::GetHeight()
{
	return this->h;
}

int Video::GetWidth()
{
	return this->w;
}

int Video::GetX()
{
	return this->x;
}

int Video::GetY()
{
	return this->y;
}

void Video::SetX(int newx)
{
	if((newx >= 0) && (newx < this->w))
		this->x = newx;
}

void Video::SetY(int newy)
{
	if((newy >= 0) && (newy < this->h))
		this->y = newy;
}

int Video::_Putch(char ch, bool update, int attr)
{
	int ret;

	switch(ch) {
		case '\n':
			_NewLine();
			ret = (int)'\n';
			break;

		default:
			ret = _Putch(ch, x, y, attr);
			_AdvanceCursor();
	}

	if(update)
		_UpdateCursor();
	return ret;
}

int Video::_Puts(const char *s, bool newline, int attr, bool update)
{
	while(*s != '\0') {
		_Putch(*s, false, attr);
		s++;
	}
	if(newline)
		_NewLine();
	if(update)
		_UpdateCursor();
	return 1;
}

void Video::_CarriageReturn()
{
	x = 0;
}

void Video::_LineFeed()
{
	y++;
	if(y >= h) {
		_Scroll(y - h + 1);
		y = h - 1;
	}
}

void Video::_NewLine()
{
	_CarriageReturn();
	_LineFeed();
}

void Video::_AdvanceCursor()
{
	x++;
	if(x >= w)
		_NewLine();
}

int Video::Printf(const char *format, ...)
{
	va_list args;
	int ret;

	va_start(args, format);
	ret = Formatter::vfPrintf(this, format, args);
	va_end(args);

	return ret;
}
