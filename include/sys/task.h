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

#ifndef TASK_H
#define TASK_H

#include "sys/section.h"
#include "list.h"
#include "sys/module.h"
#include "sys/vmem.h"
#include "sys/concurrent.h"

class TaskSwitchInfo {
public:
	virtual void Init(uintptr entry_address, Section *Stack, void *param = NULL, Section *KernelStack = NULL) = 0;

	virtual ~TaskSwitchInfo() {};
};

extern "C" void __exit();

class Thread;

class TaskSwitcher : public Lockable {
public:
	virtual void Switch(Thread *new_thread) = 0;

	virtual ~TaskSwitcher() {};
};

class Process;

#define LOC_CURRENT		-1
#define LOC_SLEEPING	-2
#define LOC_BLOCKING	-3
#define LOC_RELEASED	-4

#define DEF_SLICE		10000000  // = 10ms
#define DEF_PRIORITY	5

class Thread : public Lockable {
public:
	TaskSwitchInfo *tsi;
	Process *proc;

	// Section pointer to thread's stack and tls
	Section *Stack;
	Section *KernelStack;
	Section *ThreadLocal;

	unsigned long long int time_to_run;
	int priority;
	unsigned long long default_slice;

	const char *name;

	int location;

	static Thread *Create(Process *process, uintptr entry_address, void *param = NULL, const char *name = NULL,
		uintptr stacksize = NULL);

	Thread() : tsi(NULL), proc(NULL), Stack(NULL), KernelStack(NULL), ThreadLocal(NULL), time_to_run(0),
		priority(DEF_PRIORITY),	default_slice(DEF_SLICE), name(NULL), location(LOC_RELEASED) {}
};

class SectionList;

class Process {
public:
	// List of threads
	IList<Thread> *Threads;

	char *name;
	int priority;
	int default_slice;

	bool user_mode;

	// List of sections
	SectionList *Sections;

	// List of modules
	IList<Module> *Modules;

	// Pointer to heap
	Section *Heap;

	// Address Space
	AddressSpace *Address_Space;

	Process() : name(NULL), priority(DEF_PRIORITY), default_slice(DEF_SLICE), user_mode(true), Sections(NULL),
		Modules(NULL), Heap(NULL), Address_Space(NULL) {}
};

#endif
