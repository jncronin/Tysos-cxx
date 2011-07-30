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

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "list.h"

template <class T> class LinkedList : public IList<T>, public Lockable {
protected:
	class Node {
	public:
		T &item;
		Node *next, *prev;

		Node(T& i) : item(i), next(NULL), prev(NULL) {}
	};

public:
	LinkedList() : f(NULL), l(NULL), count(0) {};

	virtual int Count() { return count; }
	virtual bool IsReadOnly() { return false; }
	virtual void Add(T& item) {
		Node *n = new Node(item);
		_insertend(n);
	}

	virtual void Clear() { while(l) { _remove(l); } }
	virtual bool Contains(T& item) { if(IndexOf(item) >= 0) return true; else return false; }
	virtual void CopyTo(IList<T> *dest, int index = 0) {
		Node *c = f;
		while(c) { dest->Insert(c->item, index); c = c->next; index++; }
	}
	virtual bool Remove(T& item) {
		Node *n;
		if(_find(item, &n) < 0) return false;
		_remove(n);
		return true;
	}
	virtual bool IsSynchronized() { return true; }
	virtual Lockable *SyncRoot() { return this; }
	virtual T& GetItem(int index) { return _item(index)->item; }
	virtual void SetItem(int index, T& item) { _item(index)->item = item; }
	virtual int IndexOf(T& item) { return _find(item); }
	virtual void Insert(T& item, int index = 0) {
		Node *next = _item(index);
		Node *n = new Node(item);

		if(next) {
			_insertbef(next, n);
		} else {
			_insertend(n);
		}
	}

	virtual void RemoveAt(int index) { _remove(_item(index)); }
	virtual IEnumerator<T> *GetEnumerator() { return new Enumerator(this); }

	class Enumerator : public IEnumerator<T> {
	public:
		virtual T& Current() { return c->item; }
		virtual bool MoveNext() {
			if(c == NULL)
				c = l->f;
			else
				c = c->next;
			if(c == NULL)
				return false;
			else
				return true;
		}
		virtual void Reset() { c = NULL; }

		Enumerator(LinkedList *list) : l(list), c(NULL) {}
	protected:
		LinkedList *l;
		Node *c;
	};

protected:
	Node *f, *l;

	virtual void _insertbef(Node *cur, Node *n) {
		if(cur->prev) {
			cur->prev->next = n;
			n->prev = cur->prev;
		} else {
			n->prev = NULL;
			f = n;
		}

		cur->prev = n;
		n->next = cur;
		count++;
	}
	virtual void _insertend(Node *n) {
		if(l == NULL)
			f = l = n;
		else {
			n->prev = l;
			l->next = n;
			l = n;
		}
		count++;
	}
	virtual int _find(T& i, Node **n = NULL) {
		Node *c = f;
		int index = 0;
		while(c) { if(&i == &c->item) { if(n != NULL) *n = c; return index; } c = c->next; index++; }
		return -1;
	}
	virtual Node *_item(int index) {
		Node *c = f;
		while(c) { if(0 == index) return c; c = c->next; index--; }
		return NULL;
	}
	virtual void _remove(Node *n) {
		if(n == NULL) return;
		if(n->prev)
			n->prev->next = n->next;
		else
			f = n->next;
		if(n->next)
			n->next->prev = n->prev;
		else
			l = n->prev;
		delete n;
		count--;
	}

	int count;
};

#endif
