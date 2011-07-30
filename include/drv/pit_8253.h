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

#ifndef PIT_8253_H
#define PIT_8253_H

#include "drv/drv.h"
#include "sys/timer.h"

class Pit_8253 : public HardwareInterruptHooker, public ITimer
{
public:
	Pit_8253();

	virtual void SetIrq(int __attribute__ ((__unused__)) irq) { throw "PIT is not PnP!"; }
	virtual int_callback *GetHandlerFunction();

	virtual bool HandlerIsInterruptable() { return false; }
	virtual bool RunInUserMode() { return false; }
	
	virtual void SetFrequency(unsigned long int freq);
	virtual unsigned long int GetFrequency();
	virtual unsigned long long int GetInterval();
	virtual unsigned long long int GetRunningTime() { return _running_time; }
	virtual void IncreaseRunningTime(unsigned long long int delta) { _running_time += delta; }
	virtual void Enable();

protected:
	unsigned long long _reload_ns;
	unsigned long int _freq;
	unsigned long long _running_time;
	static const unsigned int DefaultFreq = 0x40;
	static const unsigned int PitInputFreq = 0x1234DD;
	static const unsigned char CommandPort = 0x43;
	static const unsigned char RatePort = 0x40;
	static const unsigned char DefaultMode = 0x34;
};

#endif
