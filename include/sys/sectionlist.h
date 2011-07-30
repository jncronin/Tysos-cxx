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

#include "sys/section.h"

#ifndef SECTIONLIST_H
#define SECTIONLIST_H

#include "list.h"
#include "stream.h"
#include "linkedlist.h"

class Section;

class SectionList : public IList<Section>, public Serializable, public Lockable {
public:
	SectionList(IList<Section> *kernelSections = NULL, IList<Section> *processSections = NULL) {
		if(kernelSections == NULL) KernelSections = new LinkedList<Section>(); else KernelSections = kernelSections;
		if(processSections == NULL) ProcessSections = new LinkedList<Section>(); else ProcessSections = processSections; }

	class Enumerator : public IEnumerator<Section> {
	public:
		virtual Section &Current() { if(proc) return pe->Current(); else return ke->Current(); };
		virtual bool MoveNext() { if(ke->MoveNext()) return true; else { proc = true; return pe->MoveNext(); }};
		virtual void Reset() { ke->Reset(); pe->Reset(); proc = false; };

		Enumerator(SectionList &list) : l(list), proc(false) {
			ke = l.KernelSections->GetEnumerator();
			pe = l.ProcessSections->GetEnumerator();
		}
	protected:
		SectionList &l;
		IEnumerator<Section> *pe, *ke;
		bool proc;
	};

	// IEnumerable members
	virtual IEnumerator<Section> *GetEnumerator() { return new SectionList::Enumerator(*this); }

	// ICollection members
	virtual int Count() { return ((KernelSections == NULL) ? 0 : KernelSections->Count()) + 
		((ProcessSections == NULL) ? 0 : ProcessSections->Count()); }
	virtual bool IsReadOnly() { return false; }
	virtual void Add(Section &item);
	virtual void Clear();
	virtual bool Contains(Section &item);
	virtual void CopyTo(IList<Section> *dest, int index = 0);
	virtual bool Remove(Section &item);
	virtual bool IsSynchronized() { return true; }
	virtual Lockable *SyncRoot() { return this; }

	// IList members
	virtual Section &GetItem(int index);
	virtual void SetItem(int index, Section &item);
	virtual int IndexOf(Section &item);
	virtual void Insert(Section &item, int index);
	virtual void RemoveAt(int index);

	// ISerializable members
	virtual void Serialize(IStream *s);

	// Class members
	Section *FindFree(VirtMemManager *vmem, Section *Request);
	bool IsFree(Section *Request);
	bool IsFree(Section::Extent &Request);
	IList<Section> *GetKernelSections() { return KernelSections; }
	void ConvertToLinkedList() {
		LinkedList<Section> *new_k = new LinkedList<Section>;
		KernelSections->CopyTo(new_k);
		LinkedList<Section> *new_p = new LinkedList<Section>;
		ProcessSections->CopyTo(new_p);
		KernelSections = new_k;
		ProcessSections = new_p;
	}

protected:
	IList<Section> *KernelSections, *ProcessSections;
	Section *_findfree(IList<Section> *Sections, VirtMemManager *vmem, Section *Request);
	bool _isfree(IList<Section> *Sections, Section::Extent &Request);
	bool _isfree(IList<Section> *Sections, Section *Request);
};

#endif
