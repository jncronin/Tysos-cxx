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

#ifndef PIC_8259A_H
#define PIC_8259A_H

#include "drv/drv.h"

class Pic_8259a : public HardwareInterruptEnabler {
public:
	Pic_8259a();

	virtual void EnableAll();
	virtual void DisableAll();

	virtual void Enable(int irq, void (**int_handler)(void), bool user_mode = false, bool interruptable = true);
	virtual void EndInterrupt(int irq);

protected:
	void _enable(int irq);

	static const unsigned char MasterCommand = 0x20;
	static const unsigned char MasterData = 0x21;
	static const unsigned char SlaveCommand = 0xa0;
	static const unsigned char SlaveData = 0xa1;

	static const unsigned char EOI = 0x20;
	static const unsigned char RemapBase = 0x20;
	static const unsigned char Cascade = 0x02;
	static const unsigned char InitIcw4 = 0x11;
	static const unsigned char Interval4 = 0x4;
	static const unsigned char Icw4_8086 = 0x01;
};

#endif
