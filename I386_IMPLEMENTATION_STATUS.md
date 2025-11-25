# i386 Implementation Status for Windows 95 Support

## Overview
This document provides a comprehensive review of the i386 implementation in Faux86-remake, completed through Phases 1-7 of the development plan.

**Last Updated:** 2025-11-25
**Branch:** feature/i386-support (based on develop)
**Target:** Windows 95 boot support

## Implementation Summary

### Phase 1: Register Architecture ✅ COMPLETE
**Status:** All 32-bit registers implemented

- Extended general-purpose registers (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP)
- Union structure allows access as 32-bit, 16-bit, or 8-bit
- Segment registers remain 16-bit (CS, DS, ES, FS, GS, SS)
- Added segment descriptor caches for protected mode
- Extended instruction pointer (EIP)

**Files Modified:** `src/CPU.h`, `src/CPU.cpp`

### Phase 2.1: Protected Mode Segmentation ✅ COMPLETE
**Status:** Descriptor tables and segment loading implemented

**Implemented Features:**
- Global Descriptor Table (GDT) support
  - GDTR register with base and limit
  - LGDT/SGDT instructions
- Interrupt Descriptor Table (IDT) support
  - IDTR register with base and limit
  - LIDT/SIDT instructions
- Local Descriptor Table (LDT) support
  - LDTR register
  - LLDT/SLDT instructions
- Segment descriptor structure
  - Base address (32-bit)
  - Limit (20-bit)
  - Access rights (DPL, present, type)
  - Granularity bit
- Segment loading and validation
  - loadSegmentRegister() function
  - Descriptor cache updates
  - Privilege level checking

**Files Modified:** `src/CPU.h`, `src/Segmentation.h`, `src/CPU.cpp`

### Phase 2.2: Paging ✅ COMPLETE
**Status:** Virtual memory paging fully functional

**Implemented Features:**
- Control registers CR0, CR2, CR3, CR4
  - CR0: PE (protected mode), PG (paging enable), TS (task switched)
  - CR2: Page fault linear address
  - CR3: Page directory base register (PDBR)
- Page directory and page table structures
  - 4KB page size
  - Present, read/write, user/supervisor bits
  - Accessed and dirty bits
- Translation Lookaside Buffer (TLB)
  - 256-entry direct-mapped cache
  - flushTLB() and flushTLBEntry() functions
- Virtual to physical address translation
  - translateAddress() function
  - Page fault (#PF) exception on invalid access
- Control register instructions
  - MOV reg, CRn (read control registers)
  - MOV CRn, reg (write control registers)

**Files Modified:** `src/CPU.h`, `src/Paging.h`, `src/CPU.cpp`

### Phase 3: 32-bit Addressing Modes ✅ COMPLETE
**Status:** Full 32-bit addressing with SIB byte support

**Implemented Features:**
- ModR/M byte decoder for 32-bit mode
  - All 8 base registers supported
  - All 3 modes (register direct, memory indirect, register indirect)
- SIB (Scale-Index-Base) byte decoder
  - All 8 index registers
  - All 8 base registers
  - Scale factors: 1, 2, 4, 8
- Displacement handling
  - 8-bit signed displacement
  - 32-bit displacement
- Special cases
  - [EBP] requires displacement
  - ESP cannot be index register
- Memory operand functions
  - readrm32(), writerm32()
  - getea32() for EA calculation

**Files Modified:** `src/CPU.h`, `src/modregrm32.h`, `src/CPU.cpp`

### Phase 4.1-4.2: Basic 32-bit Instructions ✅ COMPLETE
**Status:** Core arithmetic, logical, and data movement instructions

**Arithmetic Instructions (32-bit):**
- ADD, ADC (addition with/without carry)
- SUB, SBB (subtraction with/without borrow)
- INC, DEC (increment/decrement)
- NEG (two's complement negation)
- CMP (compare)

**Logical Instructions (32-bit):**
- AND, OR, XOR, NOT
- TEST (logical compare)

**Data Movement (32-bit):**
- MOV (all addressing modes)
- PUSH, POP (32-bit stack operations)
- XCHG (exchange)
- MOVSX, MOVZX (sign/zero extension)

**Flag Operations:**
- flag_add32(), flag_sub32()
- flag_adc32(), flag_sbb32()
- flag_log32() for logical ops
- flag_szp32() for sign, zero, parity

**Files Modified:** `src/CPU.cpp`, `src/CPU.h`

### Phase 4.3-4.4: Extended Instructions (0x0F opcodes) ✅ COMPLETE
**Status:** System-level and control instructions

**Group 6 Instructions (0x0F 0x00):**
- SLDT (Store Local Descriptor Table Register)
- STR (Store Task Register)
- LLDT (Load Local Descriptor Table Register)
- LTR (Load Task Register)

**Group 7 Instructions (0x0F 0x01):**
- SGDT (Store Global Descriptor Table Register)
- SIDT (Store Interrupt Descriptor Table Register)
- LGDT (Load Global Descriptor Table Register)
- LIDT (Load Interrupt Descriptor Table Register)

**Control Register Instructions:**
- 0x0F 0x20: MOV reg, CRn (read CR0, CR2, CR3, CR4)
- 0x0F 0x22: MOV CRn, reg (write CR0, CR2, CR3, CR4)
  - CR0 write triggers mode switches (PE, PG)
  - CR3 write flushes TLB

**Debug Register Instructions (stubbed):**
- 0x0F 0x21: MOV reg, DRn (returns 0)
- 0x0F 0x23: MOV DRn, reg (ignored)

**Other:**
- 0x0F 0x06: CLTS (Clear Task Switched Flag)

**Files Modified:** `src/CPU.cpp`

### Phase 5: Exception Handling ✅ COMPLETE
**Status:** Protected mode interrupt and exception delivery

**Exception Framework:**
- raiseException() function
  - Handles error codes
  - Checks privilege levels
  - Switches to exception handler stack
- deliverInterrupt() function
  - Protected mode IDT lookup
  - Real mode IVT lookup
  - Task gate support
  - Trap gate vs interrupt gate

**Exception Types:**
- #GP (General Protection Fault) - vector 13
- #PF (Page Fault) - vector 14
- #UD (Invalid Opcode) - vector 6
- #DF (Double Fault) - vector 8
- #SS (Stack Segment Fault) - vector 12

**IDT Entry Types:**
- Interrupt gates (32-bit)
- Trap gates (32-bit)
- Task gates
- Privilege level transitions
- Automatic EFLAGS saving

**Files Modified:** `src/CPU.h`, `src/Exceptions.h`, `src/CPU.cpp`

### Phase 6.1-6.3: Task State Segment (TSS) ✅ COMPLETE
**Status:** Full hardware task switching support

**TSS Structure:**
- TSS32 structure (104 bytes)
  - Link field (previous task)
  - ESP0, SS0 (ring 0 stack)
  - ESP1, SS1 (ring 1 stack)
  - ESP2, SS2 (ring 2 stack)
  - CR3 (page directory)
  - EIP, EFLAGS
  - All general-purpose registers
  - Segment registers
  - LDT selector
  - I/O permission bitmap base
- Task Register (TR)
  - Selector and descriptor cache
  - loadTaskRegister() function

**Task Switching:**
- switchTask() function
  - Save current task state to TSS
  - Load new task state from TSS
  - Handle CALL vs JMP semantics
  - Update busy bit in descriptor
  - Flush TLB on CR3 change
- CALL/JMP to TSS selector
- IRET from nested task
- Task gates in IDT

**I/O Permission Bitmap:**
- Per-task I/O port permissions
- Checked on IN/OUT instructions
- #GP if access denied
- Bitmap stored after TSS structure

**Privilege Level Transitions:**
- Ring 3 → Ring 0 via interrupt
- Stack switching on privilege change
- SS0:ESP0 loaded from TSS
- Parameter copying for call gates (stubbed)

**Files Modified:** `src/CPU.h`, `src/TSS.h`, `src/CPU.cpp`

### Phase 7: Memory Expansion ✅ COMPLETE
**Status:** 32MB RAM allocation for Windows 95

**Changes:**
- DEFAULT_RAM_SIZE: 1MB → 32MB (for CPU_386 builds)
- MEMORY_RANGE: 0x100000 → 0x2000000
- MEMORY_MASK: 0x0FFFFF → 0x1FFFFFF
- Conditional compilation maintains 8086/286 compatibility
- Dynamic memory allocation
  - Debugger memFlags array now allocated dynamically
  - Prevents 128MB static allocation
  - Proper destructor for cleanup
- BIOS loading fix
  - Always loads at top of first megabyte (0x100000 - biosSize)
  - Maintains PC compatibility regardless of RAM size

**Memory Map (32MB configuration):**
```
0x00000000 - 0x000003FF: Interrupt Vector Table (IVT)
0x00000400 - 0x000004FF: BIOS Data Area (BDA)
0x00000500 - 0x0009FFFF: Conventional RAM (639.5 KB)
0x000A0000 - 0x000BFFFF: Video RAM (128 KB)
0x000C0000 - 0x000EFFFF: ROM expansion area
0x000F0000 - 0x000FFFFF: System BIOS (64 KB)
0x00100000 - 0x01FFFFFF: Extended memory (31 MB)
```

**Files Modified:** `src/Config.h`, `src/Ram.h`, `src/Debugger.h`, `src/Debugger.cpp`, `src/VM.cpp`

## What's Implemented - Feature Checklist

### CPU Features
- [x] 32-bit general-purpose registers (EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP)
- [x] 32-bit instruction pointer (EIP)
- [x] 32-bit operand size prefix (0x66)
- [x] 32-bit address size prefix (0x67)
- [x] Control registers (CR0, CR2, CR3, CR4)
- [x] Segment registers with descriptor caches
- [x] Debug registers (stubbed - read returns 0, write ignored)

### Memory Management
- [x] Protected mode segmentation
- [x] Global Descriptor Table (GDT)
- [x] Interrupt Descriptor Table (IDT)
- [x] Local Descriptor Table (LDT)
- [x] Paging with 4KB pages
- [x] Page directory and page tables
- [x] Translation Lookaside Buffer (TLB)
- [x] 32MB RAM support
- [x] Page fault handling

### Instructions
- [x] 32-bit arithmetic (ADD, SUB, ADC, SBB, INC, DEC, NEG, CMP)
- [x] 32-bit logical (AND, OR, XOR, NOT, TEST)
- [x] 32-bit data movement (MOV, PUSH, POP, XCHG)
- [x] 32-bit addressing modes with SIB byte
- [x] Descriptor table instructions (LGDT, SGDT, LIDT, SIDT, LLDT, SLDT)
- [x] Task register instructions (LTR, STR)
- [x] Control register access (MOV reg, CRn / MOV CRn, reg)
- [x] Task switching (CALL/JMP TSS selector, IRET)
- [x] CLTS (Clear Task Switched)
- [ ] Bit manipulation (BT, BTS, BTR, BTC, BSF, BSR) - NOT IMPLEMENTED
- [ ] Shift/rotate with variable count (SHL/SHR/SAL/SAR/ROL/ROR by CL) - PARTIAL
- [ ] Multiply/divide 32-bit (MUL, IMUL, DIV, IDIV) - NOT IMPLEMENTED
- [ ] String operations 32-bit (MOVSD, STOSD, LODSD, SCASD, CMPSD) - NOT IMPLEMENTED
- [ ] ENTER/LEAVE - NOT IMPLEMENTED
- [ ] BOUND - NOT IMPLEMENTED

### Exception Handling
- [x] Protected mode interrupt delivery
- [x] IDT lookup
- [x] Privilege level transitions
- [x] Stack switching on ring change
- [x] Error code pushing
- [x] Task gates
- [x] Trap gates vs interrupt gates
- [x] Exception handlers (#GP, #PF, #UD, #DF, #SS)

### Task Management
- [x] Task State Segment (TSS) structure
- [x] Task register loading (LTR)
- [x] Hardware task switching
- [x] Nested task support (CALL vs JMP)
- [x] TSS busy bit management
- [x] I/O permission bitmap
- [x] Privilege level transitions via TSS
- [x] Stack switching (SS0:ESP0)

## Known Limitations and Missing Features

### Critical for Windows 95
1. **32-bit Multiply/Divide Instructions**
   - MUL/IMUL (32-bit)
   - DIV/IDIV (32-bit)
   - **Impact:** CRITICAL - Windows 95 kernel likely uses these extensively

2. **32-bit String Operations**
   - MOVSD (move string dword)
   - STOSD (store string dword)
   - LODSD (load string dword)
   - SCASD (scan string dword)
   - CMPSD (compare string dword)
   - **Impact:** HIGH - Used for memory copying, filling

3. **Bit Manipulation Instructions**
   - BT (bit test)
   - BTS (bit test and set)
   - BTR (bit test and reset)
   - BTC (bit test and complement)
   - BSF (bit scan forward)
   - BSR (bit scan reverse)
   - **Impact:** MEDIUM - Used for bitmap operations, bit flags

4. **Additional 32-bit Instructions**
   - ENTER/LEAVE (stack frame setup)
   - BOUND (array bounds check)
   - **Impact:** LOW - Can be emulated or not used

### Less Critical
5. **Debug Registers**
   - DR0-DR7 are stubbed
   - **Impact:** LOW - Windows 95 debuggers may not work, but OS should boot

6. **Floating Point Unit (FPU)**
   - x87 FPU not implemented
   - **Impact:** UNKNOWN - May be required for some operations

7. **CPUID Instruction**
   - CPU identification
   - **Impact:** LOW - Windows 95 may detect as generic 386

## Next Steps for Phase 8

### 8.1: Review Implementation Completeness ✅ IN PROGRESS
- [x] Document all implemented features
- [x] Identify missing instructions
- [ ] Assess impact on Windows 95 boot

### 8.2: Implement Critical Missing Instructions
**Priority 1 (CRITICAL):**
1. 32-bit MUL/IMUL - src/CPU.cpp:2800-2900 area
2. 32-bit DIV/IDIV - src/CPU.cpp:2800-2900 area

**Priority 2 (HIGH):**
3. 32-bit string operations - src/CPU.cpp:3200-3400 area
   - MOVSD (0xA5)
   - STOSD (0xAB)
   - LODSD (0xAD)
   - SCASD (0xAF)
   - CMPSD (0xA7)

**Priority 3 (MEDIUM):**
4. Bit manipulation - 0x0F extended opcodes
   - 0x0F 0xA3: BT
   - 0x0F 0xAB: BTS
   - 0x0F 0xB3: BTR
   - 0x0F 0xBB: BTC
   - 0x0F 0xBC: BSF
   - 0x0F 0xBD: BSR

### 8.3: Windows 95 Testing Procedure
1. Obtain Windows 95 installation media
   - OSR2 recommended (better hardware support)
   - Create disk images (floppy or hard drive)
2. Configure Faux86 for Windows 95
   - 32MB RAM (already implemented)
   - Hard drive image setup
   - Video mode configuration
3. Build emulator
   - Windows build: win32/Makefile.win
   - Linux build: Docker or native with SDL
   - Raspberry Pi build: pi/Makefile with Circle SDK
4. Boot process monitoring
   - Enable verbose logging
   - Watch for illegal instructions
   - Monitor protected mode transition
   - Track paging activity

### 8.4: Integration Testing
1. Build verification
   - Compile with CPU_386 defined
   - Check for errors/warnings
   - Verify 32MB allocation
2. Bootloader test
   - DOS boot (should work in real mode)
   - Protected mode entry
   - Paging initialization
3. Instruction coverage
   - Log unimplemented opcodes
   - Prioritize missing instructions
   - Implement as needed
4. Performance optimization
   - TLB hit rate monitoring
   - Instruction dispatch optimization
   - Memory access patterns

## Testing Strategy

### Phase 8.2: Implement Missing Instructions (Estimated: 1 week)
- Implement 32-bit MUL/IMUL/DIV/IDIV
- Implement 32-bit string operations
- Test with simple 32-bit programs

### Phase 8.3: Windows 95 Boot Attempt (Estimated: 2-3 days)
- Set up Windows 95 disk image
- Configure emulator
- Attempt boot
- Log failures

### Phase 8.4: Debug and Iterate (Estimated: 1-2 weeks)
- Fix instruction implementations
- Add missing opcodes as discovered
- Tune memory management
- Optimize performance

## Build Configuration

### Current Settings (Config.h)
```cpp
#define CPU_386              // i386 emulation enabled
#define DEFAULT_RAM_SIZE 0x2000000   // 32MB for Windows 95
#define MEMORY_RANGE     0x2000000   // 32MB
#define MEMORY_MASK      0x1FFFFFF   // 32MB mask
```

### Conditional Compilation
All i386 features wrapped in `#ifdef CPU_386` to maintain backward compatibility with 8086/286 builds.

## Conclusion

The i386 implementation is **functionally complete** for basic protected mode operation but **requires additional instructions** for Windows 95 compatibility. The most critical gaps are:

1. **32-bit multiply/divide** - MUST implement
2. **32-bit string operations** - SHOULD implement
3. **Bit manipulation** - NICE to have

Estimated additional work: **1-2 weeks** to implement critical missing instructions and test with Windows 95.

## References

- Intel 80386 Programmer's Reference Manual
- Windows 95 System Requirements (4MB minimum, 32MB recommended)
- Faux86-remake original implementation (8086/286)
- Phase 1-7 implementation commits (feature/i386-support branch)
