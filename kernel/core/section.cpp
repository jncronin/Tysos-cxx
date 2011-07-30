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
#include "sys/formatter.h"
#include "sys/arch.h"
#include "sys/sectionlist.h"
#include "errno.h"

extern Arch a;

Section *SectionList::FindFree(VirtMemManager *vmem, Section *Request)
{
	if(Request == NULL) return NULL;
	if(vmem == NULL) return NULL;

	if(Request->Flags & Section::KernelSection) {
		if(this->KernelSections != NULL)
			return _findfree(this->KernelSections, vmem, Request);
	} else {
		if(this->ProcessSections != NULL)
			return _findfree(this->ProcessSections, vmem, Request);
	}

	return NULL;
}

void SectionList::Serialize(IStream *s)
{
	Formatter::fPrintf(s, "Section dump:\n");
	/*int i;
	int c = this->Count();
	for(i = 0; i < c; i++) {
		Section &sect = GetItem(i);
		Formatter::fPrintf(s, "  %s %x - %x\n", sect.Name, sect.Base, sect.Base + sect.Length);
	}*/
	IEnumerator<Section> *e = this->GetEnumerator();
	while(e->MoveNext()) {
		Section &sect = e->Current();
		Formatter::fPrintf(s, "  %s %x - %x\n", sect.Name, sect.Base, sect.Base + sect.Length);
	}
}

Section *SectionList::_findfree(IList<Section> *Sections, VirtMemManager *vmem, Section *Request)
{
	int i;
	int c = Sections->Count();
	uintptr range_min, range_max;

	Section::Extent re;
	Request->CheckExtents(&re);

	if(Request->Flags & Section::KernelSection) {
		range_min = vmem->GetKernelStart();
		range_max = vmem->GetKernelEnd();
	} else {
		range_min = vmem->GetProcessStart();
		range_max = vmem->GetProcessEnd();
	}

	for(i = -1; i < c; i++) {
		if(i == -1) {
			re.Lower = range_min;
		} else {
			re.Lower = Sections->GetItem(i).Base + Sections->GetItem(i).Length + Sections->GetItem(i).ExpandUp +
				Sections->GetItem(i).UpperPadding;
		}

		// Fix up to a page
		re.Lower = (re.Lower + vmem->GetPageSize() - 1) & vmem->GetPageMask();
		re.Upper = re.Lower + re.Length - 1;

		if((re.Lower >= range_min) && (re.Upper <= range_max) && _isfree(Sections, re)) {
			Request->Base = re.Lower + Request->LowerPadding + Request->ExpandDown;
			return Request;
		}
	}

	return NULL;
}

bool Section::CheckExtents(Section::Extent *ext)
{
	uintptr min, max, r_min, r_max;

	ext->Length = LowerPadding + ExpandDown + Length + ExpandUp + UpperPadding;

	if(!(this->Flags & Section::Defined))
		return false;

	if(this->Flags & Section::KernelSection) {
		min = a.vmm->GetKernelStart();
		max = a.vmm->GetKernelEnd();
	} else {
		min = a.vmm->GetProcessStart();
		max = a.vmm->GetProcessEnd();
	}

	// Be very careful that we don't cause an integer overflow when calculating our section size
	r_min = this->Base;
	if(this->ExpandDown > r_min)
		return false;
	r_min -= this->ExpandDown;
	if(this->LowerPadding > r_min)
		return false;
	r_min -= this->LowerPadding;
	if(r_min < min)
		return false;

	r_max = this->Base;
	if(r_max > max)
		return false;
	if(this->Length > (max - r_max))
		return false;
	r_max += this->Length;
	if(this->ExpandUp > (max - r_max))
		return false;
	r_max += this->ExpandUp;
	if(this->UpperPadding > (max - r_max))
		return false;
	r_max += this->UpperPadding;

	if(r_min >= r_max)
		return false;		// Can't have a zero-sized extent as upper is like a limit, i.e maximum byte used

	r_max--;				// Make upper a limit

	if(ext != NULL) {
		ext->Lower = r_min;
		ext->Upper = r_max;
	}

	return true;
}

bool SectionList::_isfree(IList<Section> *Sections, Section *Request)
{
	Section::Extent re;
	if(Request->CheckExtents(&re) == false) {
		return false;
	}
	return _isfree(Sections, re);
}

bool SectionList::_isfree(IList<Section> *Sections, Section::Extent &re)
{	
	int i, c;
	c = Sections->Count();

	for(i = 0; i < c; i++) {
		Section::Extent se;
		if(Sections->GetItem(i).CheckExtents(&se) == false) {
			continue;
		}
		if(re == se) {
			return false;
		}
	}

	return true;
}

unsigned long int Section::Overlap(Section *s1, Section *s2)
{
	Section::Extent e1, e2;
	if(s1->CheckExtents(&e1) == false)
		return Section::Error;
	if(s2->CheckExtents(&e2) == false)
		return Section::Error;

	return e1.Overlap(&e2);
}

unsigned long int Section::Extent::Overlap(Section::Extent *e1, Section::Extent *e2)
{
	// Compares region 1 with region 2, sets bits in return value according to result.
	// If part of region 1 is less than region 2, sets LESS
	// If part of region 1 overlaps region 2, sets EQUAL
	// If part of region 1 is greater than region 2, sets MORE
	// If EQUAL is set, without either LESS or MORE, then it also sets WITHIN (ie. region 1 is totally within 2)

	int ret = 0;

	if(e1->Lower < e2->Lower)
		ret |= Section::Less;
	if(e1->Upper > e2->Upper)
		ret |= Section::More;
	if((e1->Lower >= e2->Lower) && (e1->Lower <= e2->Upper))
		ret |= Section::Equal;
	else if((e1->Upper >= e2->Lower) && (e1->Upper <= e2->Upper))
		ret |= Section::Equal;
	else if((ret & Section::Less) && (ret & Section::More))
		ret |= Section::Equal;
	if(ret == Section::Equal)
		ret |= Section::Within;

	return ret;
}

void SectionList::Add(Section &item)
{
	if(item.Flags & Section::KernelSection) {
		if(KernelSections != NULL)
			KernelSections->Add(item);
	} else {
		if(ProcessSections != NULL)
			ProcessSections->Add(item);
	}
}

void SectionList::Clear()
{
	if(KernelSections != NULL)
		KernelSections->Clear();
	if(ProcessSections != NULL)
		ProcessSections->Clear();
}

bool SectionList::Contains(Section &item)
{
	if(KernelSections != NULL) {
		if(KernelSections->Contains(item)) return true; }
	if(ProcessSections != NULL) {
		if(ProcessSections->Contains(item)) return true; }

	return false;
}

void SectionList::CopyTo(IList<Section> *dest, int index)
{
	if(KernelSections != NULL) {
		KernelSections->CopyTo(dest, index);
		index += KernelSections->Count();
	}
	if(ProcessSections != NULL)
		ProcessSections->CopyTo(dest, index);
}

bool SectionList::Remove(Section &item)
{
	if(item.Flags & Section::KernelSection) {
		if(KernelSections != NULL)
			return KernelSections->Remove(item);
	} else {
		if(ProcessSections != NULL)
			return ProcessSections->Remove(item);
	}

	return false;
}

Section &SectionList::GetItem(int index)
{
	if(KernelSections != NULL) {
		if(index < KernelSections->Count())
			return KernelSections->GetItem(index);
		else
			index -= KernelSections->Count();
	}
	if(ProcessSections != NULL) {
		return KernelSections->GetItem(index);
	}
	throw E_INDEX;
}

void SectionList::SetItem(int index, Section &item)
{
	if(KernelSections != NULL) {
		if(index < KernelSections->Count()) {
			KernelSections->SetItem(index, item);
			return;
		} else
			index -= KernelSections->Count();
	}
	if(ProcessSections != NULL) {
		KernelSections->SetItem(index, item);
		return;
	}

	throw E_INDEX;
}

int SectionList::IndexOf(Section &item)
{
	int i = -1;
	if(KernelSections != NULL) {
		i = KernelSections->IndexOf(item);
		if(i >= 0)
			return i;
	}
	if(ProcessSections != NULL)
		i = ProcessSections->IndexOf(item);
	return i;
}

void SectionList::Insert(Section &item, int __attribute__ ((__unused__)) index)
{
	Add(item);
}

void SectionList::RemoveAt(int index)
{
	if(KernelSections != NULL) {
		if(index < KernelSections->Count()) {
			KernelSections->RemoveAt(index);
			return;
		} else
			index -= KernelSections->Count();
	}
	if(ProcessSections != NULL) {
		KernelSections->RemoveAt(index);
	}
}
