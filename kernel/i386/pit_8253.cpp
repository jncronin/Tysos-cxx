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

#include "drv/pit_8253.h"
#include "i386/io.h"
#include "sys/arch.h"

extern Arch a;
#include "sys/sched.h"
extern Scheduler<> *scheduler;
extern Thread *CurrentThread;

extern void (*pit_8253_handler)(void);

extern "C" void pit_stub()
{
	a.timer->IncreaseRunningTime(a.timer->GetInterval());
	a.irqs->EndInterrupt(((Pit_8253 *)a.timer)->GetIrq());
	scheduler->TimerTick(a.timer->GetInterval(), CurrentThread, a.tswitch);
}

Pit_8253::Pit_8253()
{
	this->_irq = 0;
	SetFrequency(DefaultFreq);
}

void Pit_8253::SetFrequency(unsigned long int freq)
{
	// input frequency fires at 0x1234DD Hz
	// therefore reload value is 0x1234DD / freq

	unsigned long int reload = PitInputFreq / freq;
	if(reload < 5)
		reload = 5;
	if(reload > 65535)
		reload = 65535;

	_freq = PitInputFreq / reload;

	// freq times a second = freq / 1,000,000,000  times a nanosecond
	// therefore 1,000,000,000 / freq ns between ticks

	_reload_ns = (unsigned long long int)(1000000000 / _freq);

	// Program the pic
	out(CommandPort, DefaultMode);
	out(RatePort, (unsigned char)(reload & 0xff));
	out(RatePort, (unsigned char)((reload >> 8) & 0xff));
}

int_callback *Pit_8253::GetHandlerFunction()
{
	return &pit_8253_handler;
}

void Pit_8253::Enable()
{
	a.irqs->Enable(this);
}

unsigned long int Pit_8253::GetFrequency()
{
	return _freq;
}

unsigned long long int Pit_8253::GetInterval()
{
	return _reload_ns;
}
