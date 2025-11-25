/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  Contributions and Updates (c)2023 Curtis aka ArnoldUK
  i386 Protected Mode Support (c)2025

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
	// Segment Descriptor (8 bytes) - used in GDT/LDT
	// Format matches Intel i386 specification
	struct SegmentDescriptor
	{
		uint16_t limit_low;      // Limit bits 0-15
		uint16_t base_low;       // Base address bits 0-15
		uint8_t base_mid;        // Base address bits 16-23
		uint8_t access;          // Access byte (P, DPL, S, Type)
		uint8_t granularity;     // Flags + Limit 16-19 (G, D/B, 0, AVL, Limit[19:16])
		uint8_t base_high;       // Base address bits 24-31

		// Extract 32-bit base address from descriptor
		inline uint32_t getBase() const
		{
			return ((uint32_t)base_low) |
			       (((uint32_t)base_mid) << 16) |
			       (((uint32_t)base_high) << 24);
		}

		// Extract 20-bit limit and apply granularity
		inline uint32_t getLimit() const
		{
			uint32_t limit = ((uint32_t)limit_low) | (((uint32_t)(granularity & 0x0F)) << 16);

			// If G (granularity) bit is set, limit is in 4K pages
			if (granularity & 0x80)  // G bit
			{
				limit = (limit << 12) | 0xFFF;  // Convert to bytes
			}

			return limit;
		}

		// Get Descriptor Privilege Level (DPL) - bits 5-6 of access byte
		inline uint8_t getDPL() const
		{
			return (access >> 5) & 3;
		}

		// Check if segment is present (P bit - bit 7 of access byte)
		inline bool isPresent() const
		{
			return (access & 0x80) != 0;
		}

		// Check if this is a code segment
		inline bool isCode() const
		{
			// S bit must be 1 (code/data segment) and bit 3 of type must be 1 (executable)
			return (access & 0x10) && (access & 0x08);
		}

		// Check if this is a data segment
		inline bool isData() const
		{
			// S bit must be 1 (code/data segment) and bit 3 of type must be 0 (non-executable)
			return (access & 0x10) && !(access & 0x08);
		}

		// Check if this is a system segment
		inline bool isSystem() const
		{
			return !(access & 0x10);  // S bit = 0
		}

		// Check if code segment is conforming
		inline bool isConforming() const
		{
			return isCode() && (access & 0x04);
		}

		// Check if data segment is writable
		inline bool isWritable() const
		{
			return isData() && (access & 0x02);
		}

		// Check if code segment is readable
		inline bool isReadable() const
		{
			return isCode() && (access & 0x02);
		}

		// Check D/B bit (default operation size / big bit)
		inline bool is32Bit() const
		{
			return (granularity & 0x40) != 0;  // D/B bit
		}

		// Get segment type (lower 4 bits of access byte)
		inline uint8_t getType() const
		{
			return access & 0x0F;
		}
	};

	// Segment Selector (16-bit value loaded into segment register)
	struct Selector
	{
		uint16_t value;

		// Get RPL (Requested Privilege Level) - bits 0-1
		inline uint16_t getRPL() const
		{
			return value & 3;
		}

		// Get TI (Table Indicator) - bit 2 (0=GDT, 1=LDT)
		inline bool isTI() const
		{
			return (value & 0x04) != 0;
		}

		// Get Index into descriptor table - bits 3-15
		inline uint16_t getIndex() const
		{
			return value >> 3;
		}

		// Create selector from components
		static inline Selector make(uint16_t index, bool ti, uint8_t rpl)
		{
			Selector sel;
			sel.value = (index << 3) | (ti ? 0x04 : 0x00) | (rpl & 3);
			return sel;
		}
	};

	// Segment Cache Entry (hidden part of segment registers)
	// Stores cached descriptor information to avoid repeated descriptor table reads
	struct SegmentCache
	{
		SegmentDescriptor descriptor;  // Cached descriptor
		uint32_t base;                 // Cached base address
		uint32_t limit;                // Cached limit
		bool valid;                    // Cache validity flag

		// Invalidate cache
		inline void invalidate()
		{
			valid = false;
		}
	};

	// Descriptor table register (GDTR/IDTR)
	struct DescriptorTableRegister
	{
		uint32_t base;   // Linear base address of table
		uint16_t limit;  // Size of table in bytes - 1
	};

	// System Segment Types (when S bit = 0)
	enum SystemSegmentType
	{
		SYS_TSS_16_AVAILABLE = 0x1,
		SYS_LDT = 0x2,
		SYS_TSS_16_BUSY = 0x3,
		SYS_CALL_GATE_16 = 0x4,
		SYS_TASK_GATE = 0x5,
		SYS_INTERRUPT_GATE_16 = 0x6,
		SYS_TRAP_GATE_16 = 0x7,
		SYS_TSS_32_AVAILABLE = 0x9,
		SYS_TSS_32_BUSY = 0xB,
		SYS_CALL_GATE_32 = 0xC,
		SYS_INTERRUPT_GATE_32 = 0xE,
		SYS_TRAP_GATE_32 = 0xF
	};

} // namespace Faux86

#endif // CPU_386
