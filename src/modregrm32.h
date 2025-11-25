/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  Contributions and Updates (c)2023 Curtis aka ArnoldUK
  i386 32-bit Addressing (c)2025

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

// 32-bit ModR/M and SIB byte decoding for i386
// This header handles 32-bit addressing modes with SIB (Scale-Index-Base) support

#pragma once

#ifdef CPU_386

// Decode 32-bit ModR/M byte with optional SIB byte
// Sets: mode, reg, rm, and potentially sib, disp32
#define modregrm32() { \
	addrbyte = getmem8(segregs[regcs], ip); \
	StepIP(1); \
	mode = addrbyte >> 6; \
	reg = (addrbyte >> 3) & 7; \
	rm = addrbyte & 7; \
	sib_used = 0; \
	disp32 = 0; \
	\
	/* Check if SIB byte is present (when mod != 3 and rm == 4) */ \
	if (mode != 3 && rm == 4) { \
		sib_used = 1; \
		sib = getmem8(segregs[regcs], ip); \
		StepIP(1); \
		sib_scale = sib >> 6; \
		sib_index = (sib >> 3) & 7; \
		sib_base = sib & 7; \
	} \
	\
	/* Handle displacement based on mod and rm */ \
	switch(mode) { \
	case 0: \
		/* Special cases: */ \
		if (rm == 5) { \
			/* [disp32] - Direct addressing */ \
			disp32 = getmem32(segregs[regcs], ip); \
			StepIP(4); \
		} else if (rm == 4 && sib_base == 5) { \
			/* [scaled index] + disp32 */ \
			disp32 = getmem32(segregs[regcs], ip); \
			StepIP(4); \
		} \
		/* Determine default segment (SS for EBP/ESP-based addressing) */ \
		if (((rm == 4 && (sib_base == 4 || sib_base == 5)) || rm == 5) && !segoverride) { \
			useseg = segregs[regss]; \
		} \
		break; \
	\
	case 1: \
		/* [base/SIB] + disp8 */ \
		disp32 = (int32_t)(int8_t)getmem8(segregs[regcs], ip); \
		StepIP(1); \
		/* Use SS for EBP/ESP-based addressing */ \
		if (((rm == 4 && (sib_base == 4 || sib_base == 5)) || rm == 5) && !segoverride) { \
			useseg = segregs[regss]; \
		} \
		break; \
	\
	case 2: \
		/* [base/SIB] + disp32 */ \
		disp32 = getmem32(segregs[regcs], ip); \
		StepIP(4); \
		/* Use SS for EBP/ESP-based addressing */ \
		if (((rm == 4 && (sib_base == 4 || sib_base == 5)) || rm == 5) && !segoverride) { \
			useseg = segregs[regss]; \
		} \
		break; \
	\
	default: \
		/* Mode 3: Register direct - no displacement */ \
		disp32 = 0; \
		break; \
	} \
}

#endif // CPU_386
