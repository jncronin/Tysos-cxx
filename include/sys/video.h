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

#ifndef VIDEO_H
#define VIDEO_H

#include "stream.h"
#include "list.h"
#include "sys/section.h"
#include "sys/concurrent.h"

#define DEFAULT_ATTRIBUTE			-1

//#define NO_LOCK_WRITE

class Video : public Stream, public Lockable {
public:
	virtual int GetX();
	virtual int GetY();
	virtual void SetX(int newx);
	virtual void SetY(int newy);

	virtual int Putch(char ch, int xpos, int ypos, int attr = DEFAULT_ATTRIBUTE)
	{ Lock(); int ret = _Putch(ch, xpos, ypos, attr); Unlock(); return ret; }
	virtual int Putch(char ch, bool update = true, int attr = DEFAULT_ATTRIBUTE)
	{ Lock(); int ret = _Putch(ch, update, attr); Unlock(); return ret; }

	virtual void Write(char ch, bool batch = false) { 
#ifndef NO_LOCK_WRITE
		Lock();
#endif
		_Write(ch, batch);
#ifndef NO_LOCK_WRITE
		Unlock();
#endif
	}
	virtual void Write(const char *s, bool batch = false) {
#ifndef NO_LOCK_WRITE
		Lock();
#endif
		_Write(s, batch);
#ifndef NO_LOCK_WRITE
		Unlock();
#endif
	}
	virtual void EndBatch() { Lock(); _UpdateCursor(); Unlock(); }

	virtual void UpdateCursor() { Lock(); _UpdateCursor(); Unlock(); }

	virtual int Puts(const char *s, bool newline = true, int attr = DEFAULT_ATTRIBUTE, bool update = true) {
		Lock(); int ret = _Puts(s, newline, attr, update); Unlock(); return ret; }

	virtual int Printf(const char *format, ...);

	virtual int GetWidth();
	virtual int GetHeight();

	virtual void CarriageReturn() { Lock(); _CarriageReturn(); Unlock(); }
	virtual void LineFeed() { Lock(); _LineFeed(); Unlock(); }
	virtual void NewLine() { Lock(); _NewLine(); Unlock(); }
	virtual void AdvanceCursor() { Lock(); _AdvanceCursor(); Unlock(); }

	virtual void Clear() { Lock(); _Clear(); Unlock(); }

	virtual ~Video() {};
protected:
	virtual void _Write(char ch, bool batch = false) { if(batch) _Putch(ch, false); else _Putch(ch, true); };
	virtual void _Write(const char *s, bool batch = false) { if(batch) _Puts(s, false, DEFAULT_ATTRIBUTE, false); 
															else _Puts(s, false, DEFAULT_ATTRIBUTE, true); };
	virtual void _EndBatch() { _UpdateCursor(); };
	virtual int _Putch(char ch, int xpos, int ypos, int attr = DEFAULT_ATTRIBUTE) = 0;
	virtual int _Putch(char ch, bool update = true, int attr = DEFAULT_ATTRIBUTE);
	virtual void _Scroll(int lines) = 0;
	virtual void _UpdateCursor() = 0;
	virtual void _Clear() = 0;
	virtual void _CarriageReturn();
	virtual void _LineFeed();
	virtual void _NewLine();
	virtual void _AdvanceCursor();
	virtual int _Puts(const char *s, bool newline = true, int attr = DEFAULT_ATTRIBUTE, bool update = true);

	int x, y;
	int w, h;
};

#endif
