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

#ifndef SCHED_H
#define SCHED_H

#include "queue.h"
#include "sys/task.h"
#include "sys/concurrent.h"

#define DEF_PRIORITIES 11

#define NO_LOCK_TIMER_TICK
#define NO_LOCK_SCHEDULE_NEXT

template <int priorities = DEF_PRIORITIES> class Scheduler : public Lockable {
public:
	void Deschedule(Thread *thread)
	{
		Lock();
		_Release(thread);
		Unlock();
	}

	void Reschedule(Thread *thread)
	{
		Lock();
		_Reschedule(thread);
		Unlock();
	}

	void Block(Thread *thread)
	{
		Lock();
		_Release(thread);

		thread->location = LOC_BLOCKING;
		blocking_tasks.Add(*thread);
		Unlock();
	}

	void Sleep(Thread *thread, unsigned long long ns)
	{
		Lock();
		_Release(thread);

		thread->location = LOC_SLEEPING;
		sleeping_tasks.InsertAtDelta(*thread, ns);
		Unlock();
	}

	Thread *TimerTick(unsigned long long ns, Thread *cur, TaskSwitcher *switcher)
	{
		Thread *new_t;

#ifndef NO_LOCK_TIMER_TICK
		Lock();
#endif
		sleeping_tasks.DecreaseDelta(ns);

		do {
			new_t = sleeping_tasks.GetZero();

			if(new_t != NULL)
				_Reschedule(new_t);
		} while(new_t != NULL);
#ifndef NO_LOCK_TIMER_TICK
		Unlock();
#endif
		
		return ScheduleNext(ns, cur, switcher);
	}

	Thread *ScheduleNext(unsigned long long ns, Thread *cur, TaskSwitcher *switcher) {
#ifndef NO_LOCK_SCHEDULE_NEXT
		Lock();
#endif
		Thread *next = this->_GetNextThread(ns);
#ifndef NO_LOCK_SCHEDULE_NEXT
		Unlock();
#endif
		if((next != cur) && (next != NULL)) {
			switcher->Switch(next);
			return next;
		}
		return NULL;
	}

protected:
	Queue<Thread> running_tasks[priorities];
	DeltaQueue<Thread, unsigned long long> sleeping_tasks;
	LinkedList<Thread> blocking_tasks;

	void _Reschedule(Thread *thread)
	{
		_Release(thread);

		thread->location = thread->priority;
		thread->time_to_run = thread->default_slice;
		running_tasks[thread->priority].Add(*thread);
	}

	void _Release(Thread *thread)
	{
		if(thread->location >= 0) {
			running_tasks[thread->location].Remove(*thread);
		} else if(thread->location == LOC_SLEEPING) {
			sleeping_tasks.Remove(*thread);
		} else if(thread->location == LOC_BLOCKING) {
			blocking_tasks.Remove(*thread);
		}
		thread->location = LOC_RELEASED;
	}

	Thread *_GetNextThread(unsigned long long ns)
	{
		int i;

		for(i = priorities - 1; i >= 0; i--) {
			Thread *ret;

			ret = running_tasks[i].GetFirst(false);
			if(ret != NULL) {
				if(ret->time_to_run <= ns)
					_Reschedule(ret);
				else
					ret->time_to_run -= ns;

				return ret;
			}
		}

		return NULL;
	}
};

#endif
