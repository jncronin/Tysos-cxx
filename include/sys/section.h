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

#ifndef SECTION_H
#define SECTION_H

#include "sys/types.h"
#include "list.h"
#include "sys/stream.h"

class Section;
class VirtMemManager;

#include "sys/vmem.h"

class Section {
public:
	Section() : Name(NULL), Flags(0), Base(0), Length(0), ExpandDown(0), ExpandUp(0), LowerPadding(0), UpperPadding(0)  {};
	enum SectionFlags {
		Defined =			0x1,
		Read =				0x2,
		Write =				0x4,
		Execute =			0x8,
		Heap =				0x10,
		Stack =				0x20,
		AutoExpand =		0x40,
		Bits =				0x80,
		SourceOnDisk =		0x100,
		Shared =			0x200,
		Metadata =			0x400,
		IPC =				0x800,
		ThreadLocal =		0x1000,
		GC =				0x2000,
		KernelSection =		0x4000
	};

	enum OverlapFlags {
		Less =				0x1,
		More =				0x2,
		Equal =				0x4,
		Within =			0x8,
		Error =				0x10
	};

	char *Name;
	unsigned long int Flags;

	uintptr Base, Length;
	uintptr ExpandDown, ExpandUp;
	uintptr LowerPadding, UpperPadding;

	physaddr_t MemSource;

	class Extent {
	public:
		uintptr Upper, Lower;
		uintptr Length;

		static unsigned long int Overlap(Extent *e1, Extent *e2);
		unsigned long int Overlap(Extent *e2) { return Overlap(this, e2); }
		bool operator==(Extent &e2) { if(Overlap(this, &e2) & Section::Equal) return true; else return false; }
		bool operator!=(Extent &e2) { if((Overlap(this, &e2) & Section::Equal) != 
			Section::Equal) return true; else return false; }
	};

	bool CheckExtents(Extent *ext);

	static unsigned long int Overlap(Section *s1, Section *s2);
	unsigned long int Overlap(Section *s2) { return Overlap(this, s2); }
	bool operator==(Section &s2) { if(Overlap(this, &s2) & Section::Equal) return true; else return false; }
	bool operator!=(Section &s2) { if((Overlap(this, &s2) & Section::Equal) !=
		Section::Equal) return true; else return false; }
};

#include "sys/sectionlist.h"

#endif
