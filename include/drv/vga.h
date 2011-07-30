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

#include "sys/video.h"
#include "drv/drv.h"

#ifndef VGA_H
#define VGA_H

#define COLOR_FRAMEBUFFER			0xb8000
#define MONO_FRAMEBUFFER			0xb0000
#define VGA_WIDTH					80
#define VGA_HEIGHT					25
#define VGA_DEFATTR					7
#define VGA_BDA_PORT				0x463
#define VGA_BDA_TYPE				0x410

class Vga : public Video, public MemoryResourceUser {
public:

	virtual void RegisterMemory(IList<Section> *KernelSections);

	Vga(unsigned short int *framebuffer);
	Vga();

protected:
	virtual void _Scroll(int lines);
	virtual int _Putch(char ch, int xpos, int ypos, int attr = DEFAULT_ATTRIBUTE);
	virtual void _UpdateCursor();
	virtual void _Clear();

	unsigned short int *fb;
	unsigned short int base_port;

	Section fb_sect;
};

#endif
