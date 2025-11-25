/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers
  
  Contributions and Updates (c)2023 Curtis aka ArnoldUK

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#pragma once

// General purpose register indices (8/16/32-bit)
#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3
#define regsp 4
#define regbp 5
#define regsi 6
#define regdi 7

// Segment register indices
#define reges 0
#define regcs 1
#define regss 2
#define regds 3
#ifdef CPU_386
#define regfs 4
#define reggs 5
#endif

#ifdef __BIG_ENDIAN__
#define regal 1
#define regah 0
#define regcl 3
#define regch 2
#define regdl 5
#define regdh 4
#define regbl 7
#define regbh 6
#else
#define regal 0
#define regah 1
#define regcl 2
#define regch 3
#define regdl 4
#define regdh 5
#define regbl 6
#define regbh 7
#endif

#define StepIP(x)	ip += x
#define getmem8(x, y)	vm.memory.readByte(segbase(x) + (y))
#define getmem16(x, y)	vm.memory.readWord(segbase(x) + (y))
#define putmem8(x, y, z)	vm.memory.writeByte(segbase(x) + (y), z)
#define putmem16(x, y, z)	vm.memory.writeWord(segbase(x) + (y), z)
#define signext(value)	(int16_t)(int8_t)(value)
#define signext32(value)	(int32_t)(int16_t)(value)
#define getreg16(regid)	regs.wordregs[regid]
#define getreg8(regid)	regs.byteregs[byteregtable[regid]]
#define putreg16(regid, writeval)	regs.wordregs[regid] = writeval
#define putreg8(regid, writeval)	regs.byteregs[byteregtable[regid]] = writeval
#define getsegreg(regid)	segregs[regid]
#define putsegreg(regid, writeval)	segregs[regid] = writeval
#define segbase(x)	((uint32_t) x << 4)
#define segaddr(seg, x) (segbase(seg) + ((x) & 0xFFFF))

#define makeflagsword() \
	( \
	2 | (uint16_t) cf | ((uint16_t) pf << 2) | ((uint16_t) af << 4) | ((uint16_t) zf << 6) | ((uint16_t) sf << 7) | \
	((uint16_t) tf << 8) | ((uint16_t) ifl << 9) | ((uint16_t) df << 10) | ((uint16_t) of << 11) \
	)

#define decodeflagsword(x) { \
	temp16 = x; \
	cf = temp16 & 1; \
	pf = (temp16 >> 2) & 1; \
	af = (temp16 >> 4) & 1; \
	zf = (temp16 >> 6) & 1; \
	sf = (temp16 >> 7) & 1; \
	tf = (temp16 >> 8) & 1; \
	ifl = (temp16 >> 9) & 1; \
	df = (temp16 >> 10) & 1; \
	of = (temp16 >> 11) & 1; \
	}

#ifdef CPU_386
// 32-bit register access macros
#define getreg32(regid)	regs.dwordregs[regid]
#define putreg32(regid, writeval)	regs.dwordregs[regid] = writeval

// 32-bit memory access macros
#define getmem32(x, y)	vm.memory.readDword(segbase(x) + (y))
#define putmem32(x, y, z)	vm.memory.writeDword(segbase(x) + (y), z)

// Control Register 0 (CR0) bit definitions
#define CR0_PE	0x00000001	// Protected Mode Enable
#define CR0_MP	0x00000002	// Monitor Coprocessor
#define CR0_EM	0x00000004	// Emulation
#define CR0_TS	0x00000008	// Task Switched
#define CR0_ET	0x00000010	// Extension Type (always 1 on 386+)
#define CR0_NE	0x00000020	// Numeric Error
#define CR0_WP	0x00010000	// Write Protect
#define CR0_AM	0x00040000	// Alignment Mask
#define CR0_NW	0x20000000	// Not Write-through
#define CR0_CD	0x40000000	// Cache Disable
#define CR0_PG	0x80000000	// Paging Enable

// EFLAGS bit definitions (in addition to FLAGS bits)
#define EFLAGS_CF	0x00000001
#define EFLAGS_PF	0x00000004
#define EFLAGS_AF	0x00000010
#define EFLAGS_ZF	0x00000040
#define EFLAGS_SF	0x00000080
#define EFLAGS_TF	0x00000100
#define EFLAGS_IF	0x00000200
#define EFLAGS_DF	0x00000400
#define EFLAGS_OF	0x00000800
#define EFLAGS_IOPL	0x00003000	// I/O Privilege Level (bits 12-13)
#define EFLAGS_NT	0x00004000	// Nested Task
#define EFLAGS_RF	0x00010000	// Resume Flag
#define EFLAGS_VM	0x00020000	// Virtual 8086 Mode
#define EFLAGS_AC	0x00040000	// Alignment Check
#define EFLAGS_VIF	0x00080000	// Virtual Interrupt Flag
#define EFLAGS_VIP	0x00100000	// Virtual Interrupt Pending
#define EFLAGS_ID	0x00200000	// ID Flag

#endif // CPU_386
