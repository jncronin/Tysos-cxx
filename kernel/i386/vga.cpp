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

#include "drv/vga.h"
#include "i386/io.h"
#include "sys/section.h"
#include "list.h"

static const char fb_sect_str[] = "VGA Framebuffer";

Vga::Vga()
{
	// Read the byte at location 0x410 in the BDA, it'll tell us if we're using a color or mono display
	char c = (*(volatile unsigned short int *)VGA_BDA_TYPE) & 0x30;
	if(c == 0x30)
		fb = (unsigned short *)MONO_FRAMEBUFFER;
	else
		fb = (unsigned short *)COLOR_FRAMEBUFFER;
	// Base port is also in the BDA
	base_port = (*(volatile unsigned short int *)VGA_BDA_PORT) & 0xffff;
	w = VGA_WIDTH;
	h = VGA_HEIGHT;
	Clear();
}

void Vga::_Clear()
{
	__asm volatile ( "cld; rep; stosl" : : "D" (fb), "c" (VGA_WIDTH * VGA_HEIGHT / 2), "a" (0x0) );
}

int Vga::_Putch(char ch, int xpos, int ypos, int attr)
{
	if(attr == DEFAULT_ATTRIBUTE)
		attr = VGA_DEFATTR;

	if(xpos < 0)
		return -1;
	if(ypos < 0)
		return -1;
	if(xpos >= VGA_WIDTH)
		return -1;
	if(ypos >= VGA_HEIGHT)
		return -1;

	fb[xpos + ypos * VGA_WIDTH] = (ch & 0xff) | ((attr & 0xff) << 8);
	return (int)ch;
}

void Vga::_Scroll(int lines)
{
	if(lines == 0)
		return;

	if(lines > 0) {
		// scroll up
		__asm volatile ( "cld; rep; movsl" : : "D" (fb), "c" ((VGA_HEIGHT - lines) * VGA_WIDTH / 2),
			"S" (&fb[lines * VGA_WIDTH]) );
		__asm volatile ( "cld; rep; stosl" : : "D" (&fb[(VGA_HEIGHT - lines) * VGA_WIDTH]),
			"c" (lines * VGA_WIDTH / 2), "a" (0x0) );
	} else {
		Puts("Scrolling lines down not yet implemented");
	}
}

void Vga::_UpdateCursor()
{
	unsigned short position = (y * VGA_WIDTH) + x;
	out(base_port, (unsigned char)0x0f);
	out(base_port + 1, (unsigned char)(position & 0xff));
	out(base_port, (unsigned char)0x0e);
	out(base_port + 1, (unsigned char)((position >> 8) & 0xff));
}

void Vga::RegisterMemory(IList<Section> *KernelSections)
{
	fb_sect.Base = (uintptr)fb;
	fb_sect.Length = 0x1000;
	fb_sect.Flags = Section::Defined | Section::Bits | Section::Read | Section::Write | Section::KernelSection;
	fb_sect.Name = (char *)fb_sect_str;
	fb_sect.MemSource = (physaddr_t)fb;
	KernelSections->Add(fb_sect);
}
