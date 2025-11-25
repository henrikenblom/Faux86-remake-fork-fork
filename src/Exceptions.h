/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  Contributions and Updates (c)2023 Curtis aka ArnoldUK
  i386 Exception Handling (c)2025

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

#ifdef CPU_386

#include "Types.h"

namespace Faux86
{
	// i386 Exception/Interrupt Vector Numbers
	enum ExceptionVector
	{
		EXC_DIVIDE_ERROR = 0,        // #DE - Divide Error
		EXC_DEBUG = 1,               // #DB - Debug Exception
		EXC_NMI = 2,                 // NMI - Non-Maskable Interrupt
		EXC_BREAKPOINT = 3,          // #BP - Breakpoint
		EXC_OVERFLOW = 4,            // #OF - Overflow
		EXC_BOUND_RANGE = 5,         // #BR - BOUND Range Exceeded
		EXC_INVALID_OPCODE = 6,      // #UD - Invalid Opcode
		EXC_DEVICE_NOT_AVAILABLE = 7,// #NM - Device Not Available (FPU)
		EXC_DOUBLE_FAULT = 8,        // #DF - Double Fault (with error code)
		EXC_COPROCESSOR_SEGMENT = 9, // (reserved) - Coprocessor Segment Overrun
		EXC_INVALID_TSS = 10,        // #TS - Invalid TSS (with error code)
		EXC_SEGMENT_NOT_PRESENT = 11,// #NP - Segment Not Present (with error code)
		EXC_STACK_FAULT = 12,        // #SS - Stack-Segment Fault (with error code)
		EXC_GENERAL_PROTECTION = 13, // #GP - General Protection (with error code)
		EXC_PAGE_FAULT = 14,         // #PF - Page Fault (with error code)
		EXC_RESERVED_15 = 15,        // (reserved)
		EXC_X87_FPU_ERROR = 16,      // #MF - x87 FPU Floating-Point Error
		EXC_ALIGNMENT_CHECK = 17,    // #AC - Alignment Check (486+)
		EXC_MACHINE_CHECK = 18,      // #MC - Machine Check (Pentium+)
		// 19-31 reserved
		// 32-255 user-defined interrupts
	};

	// Exceptions that push an error code
	inline bool exceptionHasErrorCode(uint8_t vector)
	{
		return (vector == EXC_DOUBLE_FAULT ||
		        vector == EXC_INVALID_TSS ||
		        vector == EXC_SEGMENT_NOT_PRESENT ||
		        vector == EXC_STACK_FAULT ||
		        vector == EXC_GENERAL_PROTECTION ||
		        vector == EXC_PAGE_FAULT ||
		        vector == EXC_ALIGNMENT_CHECK);
	}

	// Gate Descriptor (8 bytes) - used in IDT
	// Format matches Intel i386 specification
	union GateDescriptor
	{
		uint64_t raw;

		struct
		{
			uint16_t offset_low;     // Offset bits 0-15
			uint16_t selector;       // Segment selector
			uint8_t  param_count;    // Parameter count (for call gates) / reserved
			uint8_t  access;         // P, DPL, Type
			uint16_t offset_high;    // Offset bits 16-31
		} __attribute__((packed));

		// Gate types (bits 0-3 of access byte when S=0)
		enum GateType
		{
			GATE_TASK_32 = 0x5,      // 32-bit Task Gate
			GATE_INTERRUPT_16 = 0x6, // 16-bit Interrupt Gate
			GATE_TRAP_16 = 0x7,      // 16-bit Trap Gate
			GATE_INTERRUPT_32 = 0xE, // 32-bit Interrupt Gate
			GATE_TRAP_32 = 0xF       // 32-bit Trap Gate
		};

		// Extract gate type (bits 0-4 of access byte)
		inline uint8_t getType() const
		{
			return access & 0x1F;
		}

		// Check if gate is present (P bit - bit 7 of access byte)
		inline bool isPresent() const
		{
			return (access & 0x80) != 0;
		}

		// Get DPL (bits 5-6 of access byte)
		inline uint8_t getDPL() const
		{
			return (access >> 5) & 3;
		}

		// Get full 32-bit offset
		inline uint32_t getOffset() const
		{
			return ((uint32_t)offset_low) | (((uint32_t)offset_high) << 16);
		}

		// Check if this is an interrupt gate (clears IF)
		inline bool isInterruptGate() const
		{
			uint8_t type = getType();
			return (type == GATE_INTERRUPT_16 || type == GATE_INTERRUPT_32);
		}

		// Check if this is a trap gate (preserves IF)
		inline bool isTrapGate() const
		{
			uint8_t type = getType();
			return (type == GATE_TRAP_16 || type == GATE_TRAP_32);
		}

		// Check if this is a task gate
		inline bool isTaskGate() const
		{
			return (getType() == GATE_TASK_32);
		}

		// Check if this is a 32-bit gate
		inline bool is32Bit() const
		{
			uint8_t type = getType();
			return (type == GATE_INTERRUPT_32 || type == GATE_TRAP_32 || type == GATE_TASK_32);
		}
	};

	// Selector Error Code (pushed by some exceptions)
	union SelectorErrorCode
	{
		uint32_t raw;

		struct
		{
			uint32_t ext    : 1;  // External event (1) vs internal (0)
			uint32_t idt    : 1;  // IDT (1) vs GDT/LDT (0)
			uint32_t ti     : 1;  // LDT (1) vs GDT (0) when idt=0
			uint32_t index  : 13; // Selector index
			uint32_t unused : 16;
		} __attribute__((packed));

		// Create error code from selector
		static inline SelectorErrorCode fromSelector(uint16_t selector, bool is_idt)
		{
			SelectorErrorCode err;
			err.raw = 0;
			err.ext = 0;  // Internal exception
			err.idt = is_idt ? 1 : 0;
			if (!is_idt)
			{
				err.ti = (selector & 0x04) ? 1 : 0;  // Table indicator bit
			}
			err.index = selector >> 3;  // Selector index
			return err;
		}
	};

} // namespace Faux86

#endif // CPU_386
