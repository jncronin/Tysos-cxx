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

#ifndef QUEUE_H
#define QUEUE_H

#include "linkedlist.h"

template <class T> class Queue : public LinkedList<T> {
public:
	virtual T* GetFirst(bool remove = true) {
		T *ret = NULL;
		if(this->f != NULL) {
			ret = &this->f->item;
			if(remove)
				_remove(this->f);
		}
		return ret;
	}
};

template <class T, class delta_type> class DeltaQueue : public Queue<T> {
protected:
	class DeltaNode : public LinkedList<T>::Node {
	public:
		delta_type d;

		DeltaNode(T& item, delta_type delta) : LinkedList<T>::Node(item), d(delta) {}
	};

	virtual delta_type _sumto(DeltaNode *n) {
		DeltaNode *c = static_cast<DeltaNode *>(this->f);
		delta_type sum = 0;
		while(c != NULL) { sum += c->d; if(c == n) return sum; c = static_cast<DeltaNode *>(c->next); }
		return sum;
	}

	virtual void _remove(typename LinkedList<T>::Node *n) {
		if(n == NULL) return;
		if(n->prev)
			n->prev->next = n->next;
		else
			this->f = n->next;
		if(n->next) {
			n->next->prev = n->prev;
			(static_cast<DeltaNode *>(n->next))->d += (static_cast<DeltaNode *>(n))->d;
		} else
			this->l = n->prev;

		delete n;
		this->count--;
	}

public:
	virtual void Add(T& item, delta_type value, bool absolute = false) {
		if(absolute) value -= _sumto(static_cast<DeltaNode *>(this->l));
		if(value <= 0) value = 0;

		DeltaNode *n = new DeltaNode(item, value);
		_insertend(n);
	}

	virtual void InsertAtDelta(T& item, delta_type value) {
		DeltaNode *c = static_cast<DeltaNode *>(this->f);
		DeltaNode *n = new DeltaNode(item, value);
		while(c != NULL) {
			if(c->next != NULL) {
				if(value <= (static_cast<DeltaNode *>(c->next))->d) {
					n->d = value;
					_insertbef(c->next, n);
					(static_cast<DeltaNode *>(c->next))->d -= value;
					return;
				}
			}

			value -= c->d;
			c = static_cast<DeltaNode *>(c->next);
		}
		n->d = value;
		_insertend(n);
	}

	virtual T *GetZero() {
		T* ret = NULL;
		if(this->f == NULL)
			return NULL;
		if((static_cast<DeltaNode *>(this->f))->d == 0) {
			ret = &this->f->item;
			_remove(this->f);
		}
		return ret;
	}

	virtual delta_type DecreaseDelta(delta_type decval) {
		DeltaNode *c = static_cast<DeltaNode *>(this->f);
		while(c != NULL) {
			if(c->d >= decval) {
				c->d -= decval;
				return 0;
			}
			decval -= c->d;
			c->d = 0;
			c = static_cast<DeltaNode *>(c->next);
		}
		return decval;
	}
};

#endif
