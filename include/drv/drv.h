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

#ifndef DRV_H
#define DRV_H

#include "list.h"
#include "sys/section.h"

class IMemoryResourceUser {
public:
	virtual void RegisterMemory(IList<Section> *dest) = 0;
	virtual IList<Section> *GetSections() = 0;

	virtual ~IMemoryResourceUser() {}
};

class MemoryResourceUser : public IMemoryResourceUser {
public:
	virtual void RegisterMemory(IList<Section> *dest) = 0;
	virtual IList<Section> *GetSections() {
		LinkedList<Section> *ret = new LinkedList<Section>;
		RegisterMemory(ret);
		return ret;
	}
};

typedef void(*int_callback)(void);

class IHardwareInterruptHooker {
public:
	virtual int GetIrq() = 0;
	virtual void SetIrq(int irq) = 0;
	virtual int_callback *GetHandlerFunction() = 0;
	virtual bool RunInUserMode() = 0;
	virtual bool HandlerIsInterruptable() = 0;

	virtual ~IHardwareInterruptHooker() {}
};

class HardwareInterruptHooker : public IHardwareInterruptHooker {
public:
	virtual int GetIrq() { return _irq; }
	virtual void SetIrq(int irq) { _irq = irq; }

protected:
	int _irq;
};

class IHardwareInterruptEnabler {
public:
	virtual void Enable(int irq, void (**int_handler)(void), bool user_mode = false, bool interruptable = true) = 0;
	virtual void Enable(IHardwareInterruptHooker *hooker) = 0;
	virtual void EnableAll() = 0;
	virtual void DisableAll() = 0;

	virtual void EndInterrupt(int irq) = 0;

	virtual ~IHardwareInterruptEnabler() {}
};

class HardwareInterruptEnabler : public IHardwareInterruptEnabler {
public:
	virtual void Enable(int irq, void (**int_handler)(void), bool user_mode = false, bool interruptable = true) = 0;
	virtual void Enable(IHardwareInterruptHooker *hooker) {
		Enable(hooker->GetIrq(), hooker->GetHandlerFunction(), hooker->RunInUserMode(), hooker->HandlerIsInterruptable());
	}
};

#endif
