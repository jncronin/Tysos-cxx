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

#ifndef STREAM_H
#define STREAM_H

class IInputStream {
public:
	virtual char ReadChar() = 0;
	virtual char *ReadString() = 0;
	virtual ~IInputStream() {};
};

class ISerializable;

class IStream {
public:
	virtual void Putch(char ch) { Write(ch); };
	virtual void Puts(const char *s) { Write(s); };
	virtual void Write(char ch, bool batch = false) = 0;
	virtual void Write(const char *s, bool batch = false) = 0;
	virtual void Write(IInputStream *s) = 0;
	virtual IStream &operator<< (IInputStream *s) = 0;
	virtual IStream &operator<< (IInputStream &s) = 0;
	virtual IStream &operator<< (ISerializable *s) = 0;
	virtual IStream &operator<< (ISerializable &s) = 0;
	virtual IStream &operator<< (const char *s) = 0;
	virtual IStream &operator<< (const char c) = 0;
	virtual void EndBatch() = 0;
	virtual ~IStream() {};
};

class ISerializable {
public:
	virtual void Serialize(IStream *s) = 0;
	virtual void Serialize(IStream &s) = 0;

	virtual ~ISerializable() {};
};

class Serializable : public ISerializable {
public:
	virtual void Serialize(IStream &s) { return Serialize(&s); }
	virtual void Serialize(IStream *s) = 0;
};

class Stream : public IStream {
public:
	virtual void Write(IInputStream *s) { Write(s->ReadString()); }
	virtual IStream &operator<<(IInputStream *s) { Write(s); return *this; }
	virtual IStream &operator<<(IInputStream &s) { Write(&s); return *this; }
	virtual IStream &operator<<(ISerializable *s) { s->Serialize(this); return *this; }
	virtual IStream &operator<<(ISerializable &s) { s.Serialize(this); return *this; }
	virtual IStream &operator<<(const char *s) { Write(s); return *this; }
	virtual IStream &operator<<(const char c) { Write(c); return *this; }
	virtual void Write(const char *s, bool batch = false) = 0;
	virtual void Write(char ch, bool batch = false) = 0;
};

#endif
