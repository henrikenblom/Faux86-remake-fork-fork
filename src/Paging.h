/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  Contributions and Updates (c)2023 Curtis aka ArnoldUK
  i386 Paging Support (c)2025

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
	// Page Directory Entry / Page Table Entry
	// Both have the same format on i386
	union PageEntry
	{
		uint32_t raw;

		struct
		{
			uint32_t present    : 1;  // P: Page present in memory
			uint32_t rw         : 1;  // R/W: Read/write permission (0=read-only, 1=read-write)
			uint32_t us         : 1;  // U/S: User/supervisor (0=supervisor, 1=user)
			uint32_t pwt        : 1;  // PWT: Page-level write-through
			uint32_t pcd        : 1;  // PCD: Page-level cache disable
			uint32_t accessed   : 1;  // A: Accessed flag (set by CPU)
			uint32_t dirty      : 1;  // D: Dirty flag (set by CPU on write) - PTE only
			uint32_t pat        : 1;  // PAT/PS: Page size (PDE) or PAT (PTE)
			uint32_t global     : 1;  // G: Global page (Pentium+, ignored on 386)
			uint32_t avail      : 3;  // Available for OS use
			uint32_t frame      : 20; // Physical frame number (bits 12-31 of address)
		} __attribute__((packed));

		// Extract physical frame address
		inline uint32_t getFrameAddress() const
		{
			return frame << 12;
		}

		// Set physical frame address
		inline void setFrameAddress(uint32_t addr)
		{
			frame = (addr >> 12) & 0xFFFFF;
		}
	};

	// TLB Entry (Translation Lookaside Buffer)
	struct TLBEntry
	{
		uint32_t virtual_page;   // Virtual page number (linear address >> 12)
		uint32_t physical_page;  // Physical page number (physical address >> 12)
		bool valid;              // Entry is valid
		bool writable;           // Page is writable
		bool user;               // Page is user-accessible
		uint32_t generation;     // For bulk TLB invalidation

		// Check if this entry matches a virtual address
		inline bool matches(uint32_t linear_addr, uint32_t gen) const
		{
			return valid &&
			       (generation == gen) &&
			       ((linear_addr >> 12) == virtual_page);
		}

		// Invalidate this entry
		inline void invalidate()
		{
			valid = false;
		}

		// Update this entry
		inline void update(uint32_t virt, uint32_t phys, bool wr, bool us, uint32_t gen)
		{
			virtual_page = virt >> 12;
			physical_page = phys >> 12;
			writable = wr;
			user = us;
			generation = gen;
			valid = true;
		}

		// Get physical address for a linear address
		inline uint32_t translate(uint32_t linear_addr) const
		{
			return (physical_page << 12) | (linear_addr & 0xFFF);
		}
	};

	// Page Fault Error Code
	union PageFaultErrorCode
	{
		uint32_t raw;

		struct
		{
			uint32_t p      : 1;  // 0: Not present, 1: Protection violation
			uint32_t wr     : 1;  // 0: Read access, 1: Write access
			uint32_t us     : 1;  // 0: Supervisor mode, 1: User mode
			uint32_t rsvd   : 1;  // Reserved bit violation
			uint32_t id     : 1;  // Instruction fetch (386+)
			uint32_t unused : 27;
		} __attribute__((packed));
	};

} // namespace Faux86

#endif // CPU_386
