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

#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include "list.h"

#define DEF_CAPACITY 1024

template <class T, int capacity = DEF_CAPACITY> class ArrayList : public IList<T>, public Lockable {
public:
	virtual int Count() { return count; }
	virtual bool IsReadOnly() { return false; }

	ArrayList<T, capacity>() { count = 0; }

	virtual void Add(T& item) {
		if(count < capacity) {
			arr[count] = &item;
			count++;
		}
	}

	virtual void Clear() { count = 0; }
	virtual bool Contains(T& item) { if(IndexOf(item) >= 0) return true; else return false; }
	virtual void CopyTo(IList<T> *dest, int index = 0) {
		for(int i = 0; i < count; i++, index++) {
			dest->Insert(*arr[i], index);
		}
	}
	virtual bool Remove(T &item) {
		int loc = IndexOf(item);
		RemoveAt(loc);
		if(loc >= 0) {
			RemoveAt(loc);
			return true;
		} else
			return false;
	}
	virtual bool IsSynchronized() { return true; }
	virtual Lockable *SyncRoot() { return this; }
	virtual T& GetItem(int index) { return *arr[index]; }
	virtual void SetItem(int index, T& item) { arr[index] = &item; }
	virtual int IndexOf(T& item) { for(int i = 0; i < count; i++) { if(arr[i] == &item) return i; }; return -1; }
	virtual void Insert(T& item, int index = 0) {
		if((index >= 0) && (index <= count) && (count < capacity)) {
			ShiftUp(index + 1, index, count - index);
			SetItem(index, item);
			count++;
		}
	}
	virtual void RemoveAt(int index) {
		if((index >= 0) && (index < count)) {
			ShiftDown(index, index + 1, count - index - 1);
			count--;
		}
	}

	virtual IEnumerator<T> *GetEnumerator() { return new Enumerator(this); }

	class Enumerator : public IEnumerator<T> {
	public:
		virtual T& Current() { return *l->arr[i]; }
		virtual bool MoveNext() { if(i < l->count) { i++; return true; } else return false; }
		virtual void Reset() { i = -1; }

		Enumerator(ArrayList *list) : i(-1), l(list) {};

	protected:
		int i;
		ArrayList *l;
	};

protected:
	int count;
	T *arr[capacity];
	void ShiftDown(int d, int s, int c) {
		for(; c >= 0; c--, d++, s++)
			arr[d] = arr[s];
	}
	void ShiftUp(int d, int s, int c) {
		for(d = d + c, s = s + c; c >= 0; c--, d--, s--)
			arr[d] = arr[s];
	}
};

#endif
