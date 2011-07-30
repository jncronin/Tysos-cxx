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

#include "sys/task.h"
#include "sys/arch.h"
#include "sys/section.h"

extern Arch a;

static const char def_thr[] = "Unnamed thread";
static const char stack_str[] = "Stack";
static const char kstack_str[] = "User process kernel stack";

extern Thread *CurrentThread;

#define NO_LOCK_SWITCH

Thread *Thread::Create(Process *process, uintptr entry_address, void *param, const char *name, uintptr stacksize)
{
	Thread *ret = new Thread;

	if(name == NULL)
		name = (char *)def_thr;

	if(stacksize == 0)
		stacksize = a.vmm->GetPageSize() * 5;

	ret->name = name;
	ret->proc = process;
	ret->tsi = a.CreateTaskSwitchInfo();
	ret->priority = ret->proc->priority;
	ret->default_slice = ret->proc->default_slice;
	ret->ThreadLocal = NULL;
	ret->location = LOC_RELEASED;
	ret->KernelStack = NULL;

	ret->Stack = new Section;
	ret->Stack->Name = (char *)stack_str;
	ret->Stack->Length = a.vmm->GetPageSize();
	ret->Stack->ExpandDown = stacksize;
	ret->Stack->UpperPadding = ret->Stack->LowerPadding = a.vmm->GetPageSize();
	ret->Stack->Flags = Section::Defined | Section::Read | Section::Write | Section::AutoExpand;
	if(ret->proc->user_mode == false)
		ret->Stack->Flags |= Section::KernelSection;

	ret->proc->Sections->SyncRoot()->Lock();
	ret->Stack = ret->proc->Sections->FindFree(a.vmm, ret->Stack);
	if(ret->Stack != NULL)
		ret->proc->Sections->Add(*ret->Stack);
	ret->proc->Sections->SyncRoot()->Unlock();

	if(ret->Stack == NULL)
		return NULL;

	if(ret->proc->user_mode) {
		// We also need to define a kernel stack
		ret->KernelStack = new Section;
		ret->KernelStack->Name = (char *)kstack_str;
		ret->KernelStack->Length = a.vmm->GetPageSize();
		ret->KernelStack->Flags = Section::Defined | Section::Read | Section::Write | Section::AutoExpand |
			Section::KernelSection;

		ret->proc->Sections->SyncRoot()->Lock();
		ret->KernelStack = ret->proc->Sections->FindFree(a.vmm, ret->KernelStack);
		if(ret->KernelStack != NULL)
			ret->proc->Sections->Add(*ret->KernelStack);
		ret->proc->Sections->SyncRoot()->Unlock();

		if(ret->KernelStack == NULL)
			return NULL;
	}

	ret->tsi->Init(entry_address, ret->Stack, param, ret->KernelStack);

	ret->proc->Threads->Add(*ret);
	return ret;
}

void __exit()
{
	*a.kconsole << "Reached __exit()\n";
	while(1);
}

/*void TaskSwitcher::Switch(Thread *new_thread) {
#ifndef NO_LOCK_SWITCH
	Lock();
#endif
	TaskSwitchInfo *ctsi = CurrentThread->tsi;
	Process *cp = CurrentThread->proc;
	CurrentThread = new_thread;
#ifndef NO_LOCK_SWITCH
	Unlock();
#endif
	Switch(new_thread->tsi, ctsi, (new_thread->proc == cp) ? NULL : new_thread->proc->Address_Space, NULL);
}*/

