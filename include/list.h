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
#include "sys/concurrent.h"

#ifndef LIST_H
#define LIST_H

template <class T> class IEnumerator {
public:
	virtual T& Current() = 0;
	virtual bool MoveNext() = 0;
	virtual void Reset() = 0;

	virtual ~IEnumerator() {};
};

template <class T> class IEnumerable {
public:
	virtual IEnumerator<T> *GetEnumerator() = 0;

	virtual ~IEnumerable() {};
};

template <class T> class IList;

template <class T> class ICollection : public IEnumerable<T> {
public:
	virtual int Count() = 0;
	virtual bool IsReadOnly() = 0;
	virtual void Add(T &item) = 0;
	virtual void Clear() = 0;
	virtual bool Contains(T &item) = 0;
	virtual void CopyTo(IList<T> *dest, int index = 0) = 0;
	virtual bool Remove(T &item) = 0;
	virtual bool IsSynchronized() = 0;
	virtual Lockable *SyncRoot() = 0;

	virtual ~ICollection() {};
};

template <class T> class IList : public ICollection<T> {
public:
	virtual T &GetItem(int index) = 0;
	virtual void SetItem(int index, T& item) = 0;
	virtual int IndexOf(T &item) = 0;
	virtual void Insert(T &item, int index = 0) = 0;
	virtual void RemoveAt(int index) = 0;

	virtual ~IList() {};
};


#endif
