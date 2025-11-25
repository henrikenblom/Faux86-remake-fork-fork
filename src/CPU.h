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
#include "Types.h"
#include "CPUMacros.h"
#ifdef CPU_386
#include "Segmentation.h"
#endif

namespace Faux86
{
	class VM;

	class CPU
	{
	public:
		CPU(VM& inVM);

		void reset86();
		void exec86(uint32_t execloops);

		// Register union supporting 8-bit, 16-bit, and 32-bit access
		// Layout preserves compatibility: AL/AH/AX overlap with EAX low bits
		union _bytewordregs_
		{
#ifdef CPU_386
			uint32_t dwordregs[8];  // EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
#endif
			uint16_t wordregs[8];   // AX, CX, DX, BX, SP, BP, SI, DI
			uint8_t byteregs[8];    // AL, CL, DL, BL, AH, CH, DH, BH (endian-dependent)
		} regs;

		uint8_t cf = 0;
		uint16_t ip = 0;
		uint32_t makeupticks = 0;
		uint16_t segregs[4] = { 0, 0, 0, 0 };
		uint8_t ethif = 0;
		uint64_t totalexec = 0;
		uint8_t didbootstrap = 0;

#ifdef CPU_386
		// i386-specific registers and state

		// CPU Mode
		enum CPUMode {
			MODE_REAL,           // Real mode (8086 compatible)
			MODE_PROTECTED,      // Protected mode
			MODE_VIRTUAL_8086    // Virtual 8086 mode (within protected mode)
		};
		CPUMode cpu_mode = MODE_REAL;

		// Control Registers
		uint32_t cr0 = 0;    // Control Register 0 (PE, PG, etc.)
		uint32_t cr2 = 0;    // Page Fault Linear Address
		uint32_t cr3 = 0;    // Page Directory Base Register
		uint32_t cr4 = 0;    // Control Register 4 (reserved for 486+)

		// Debug Registers (DR0-DR7)
		uint32_t dr[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		// Test Registers (TR6, TR7 - for TLB testing)
		uint32_t tr[8] = {0, 0, 0, 0, 0, 0, 0, 0};

		// Extended Instruction Pointer
		uint32_t eip = 0;    // 32-bit instruction pointer (ip is low 16 bits)

		// Extended FLAGS
		uint32_t eflags = 0; // 32-bit flags register

		// Descriptor Table Registers
		struct {
			uint32_t base = 0;
			uint16_t limit = 0;
		} gdtr, idtr;        // Global/Interrupt Descriptor Table Registers

		// Local Descriptor Table Register
		uint16_t ldtr = 0;

		// Task Register
		uint16_t tr_reg = 0;

		// Additional Segment Registers (FS, GS)
		uint16_t segregs_ext[2] = {0, 0}; // FS, GS

		// Segment Caches (hidden part of segment registers)
		// Index 0-3: ES, CS, SS, DS
		// Index 4-5: FS, GS
		struct {
			SegmentDescriptor descriptor;
			uint32_t base;
			uint32_t limit;
			bool valid;
		} seg_cache[6];

		// Prefix flags for instruction decoding
		uint8_t operand_size_32 = 0;  // 0x66 prefix flag
		uint8_t address_size_32 = 0;  // 0x67 prefix flag

		// Protected mode segment management methods
		SegmentDescriptor loadDescriptor(uint16_t selector);
		void loadSegmentRegister(uint8_t seg, uint16_t selector);
		uint32_t getSegmentBase(uint8_t seg);
		uint32_t getSegmentLimit(uint8_t seg);
		bool checkSegmentAccess(uint8_t seg, uint32_t offset, bool write);
		uint32_t segmentTranslate(uint8_t seg, uint32_t offset);
		uint8_t getCurrentPrivilegeLevel();

#endif // CPU_386
		
	private:
		void getea(uint8_t rmval);
		uint16_t pop();
		void push(uint16_t pushval);

		uint16_t readrm16(uint8_t rmval);
		uint8_t readrm8(uint8_t rmval);
		void writerm16(uint8_t rmval, uint16_t value);
		void writerm8(uint8_t rmval, uint8_t value);

#ifdef CPU_386
		void getea32(uint8_t rmval);
		uint32_t readrm32(uint8_t rmval);
		void writerm32(uint8_t rmval, uint32_t value);
		uint32_t pop32();
		void push32(uint32_t pushval);
#endif

		uint8_t op_grp2_8(uint8_t cnt);
		uint16_t op_grp2_16(uint8_t cnt);
		void op_grp3_8();
		void op_div16(uint32_t valdiv, uint16_t divisor);
		void op_div8(uint16_t valdiv, uint8_t divisor);
		void op_idiv8(uint16_t valdiv, uint8_t divisor);
		void op_idiv16(uint32_t valdiv, uint16_t divisor);
		void op_grp3_16();
		void op_grp5();

		void flag_szp8(uint8_t value);
		void flag_szp16(uint16_t value);
		void flag_log8(uint8_t value);
		void flag_log16(uint16_t value);
		void flag_adc8(uint8_t v1, uint8_t v2, uint8_t v3);
		void flag_adc16(uint16_t v1, uint16_t v2, uint16_t v3);
		void flag_add8(uint8_t v1, uint8_t v2);
		void flag_add16(uint16_t v1, uint16_t v2);
		void flag_sbb8(uint8_t v1, uint8_t v2, uint8_t v3);
		void flag_sbb16(uint16_t v1, uint16_t v2, uint16_t v3);
		void flag_sub8(uint8_t v1, uint8_t v2);
		void flag_sub16(uint16_t v1, uint16_t v2);

#ifdef CPU_386
		void flag_szp32(uint32_t value);
		void flag_log32(uint32_t value);
		void flag_add32(uint32_t v1, uint32_t v2);
		void flag_sub32(uint32_t v1, uint32_t v2);
		void flag_adc32(uint32_t v1, uint32_t v2, uint32_t v3);
		void flag_sbb32(uint32_t v1, uint32_t v2, uint32_t v3);
#endif

		void op_adc8();
		void op_adc16();
		void op_add8();
		void op_add16();
		void op_and8();
		void op_and16();
		void op_or8();
		void op_or16();
		void op_xor8();
		void op_xor16();
		void op_sub8();
		void op_sub16();
		void op_sbb8();
		void op_sbb16();

#ifdef CPU_386
		void op_add32();
		void op_adc32();
		void op_sub32();
		void op_sbb32();
		void op_and32();
		void op_or32();
		void op_xor32();
#endif


		void intcall86(uint8_t intnum);


		VM& vm;

		uint8_t	opcode = 0, segoverride = 0, reptype = 0, hltstate = 0;
		uint16_t savecs = 0, saveip = 0, useseg = 0, oldsp = 0;
		uint8_t	tempcf = 0, oldcf = 0, pf = 0, af = 0, zf = 0, sf = 0, tf = 0, ifl = 0, df = 0, of = 0, mode = 0, reg = 0, rm = 0;
		uint16_t oper1 = 0, oper2 = 0, res16 = 0, disp16 = 0, temp16 = 0, dummy = 0, stacksize = 0, frametemp = 0;
		uint8_t	oper1b = 0, oper2b = 0, res8 = 0, disp8 = 0, temp8 = 0, nestlev = 0, addrbyte = 0;
		uint32_t temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0, temp5 = 0, temp32 = 0, ea = 0;
		int32_t	result = 0;
		uint8_t didintr = 0;

#ifdef CPU_386
		// 32-bit addressing mode variables
		uint8_t sib = 0, sib_scale = 0, sib_index = 0, sib_base = 0, sib_used = 0;
		uint32_t disp32 = 0;
		uint32_t oper1_32 = 0, oper2_32 = 0, res32 = 0;
#endif

		uint8_t	debugmode = 0, showcsip = 0, mouseemu = 0;

	};
}


// TODO: refactor this
#ifdef CPU_ADDR_MODE_CACHE
struct addrmodecache_s {
	uint16_t exitcs;
	uint16_t exitip;
	uint16_t disp16;
	uint32_t len;
	uint8_t mode;
	uint8_t reg;
	uint8_t rm;
	uint8_t forcess;
	uint8_t valid;
};
#endif



