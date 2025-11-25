# Faux86 i386 Implementation Tests

This directory contains tests for the i386 emulation implementation.

## Phase 1: Register Architecture Tests

The `test_registers.cpp` file contains unit tests for:

- **16-bit register access**: Validates AX, CX, DX, BX, SP, BP, SI, DI
- **8-bit register access**: Validates AL, AH, CL, CH, DL, DH, BL, BH
- **32-bit register access**: Validates EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
- **Register overlapping**: Ensures AL/AH overlap with AX, which overlaps with EAX
- **Control register bits**: Validates CR0 bit definitions (PE, PG, etc.)
- **EFLAGS bits**: Validates EFLAGS bit definitions

## Running Tests

### Requirements
- g++ or gcc with C++ support
- Make

### Build and Run
```bash
cd tests
make
```

### Expected Output
```
=== Phase 1: Register Architecture Tests ===

Running test: 16bit_register_access
Running test: 8bit_register_access

=== i386-specific tests ===
Running test: 32bit_register_access
Running test: register_overlapping
Running test: control_register_bits
Running test: eflags_bits

=== Test Results ===
Passed: [number]
Failed: 0
Total:  [number]
```

## Test Coverage

### Phase 1 - Register Architecture ✓
- [x] 16-bit register read/write
- [x] 8-bit register read/write
- [x] 32-bit register read/write
- [x] Register overlapping (8/16/32-bit views)
- [x] CR0 bit definitions
- [x] EFLAGS bit definitions

### Phase 2 - Memory Management (TODO)
- [ ] Segment descriptor loading
- [ ] GDT/LDT access
- [ ] Privilege checking
- [ ] Page table walking
- [ ] TLB functionality

### Phase 3 - 32-bit Addressing (TODO)
- [ ] SIB byte decoding
- [ ] 32-bit effective address calculation
- [ ] Address size prefix handling

### Phase 4 - Instructions (TODO)
- [ ] 32-bit arithmetic operations
- [ ] 32-bit logical operations
- [ ] 32-bit MOV/PUSH/POP
- [ ] 0x0F prefix instructions

### Phase 5 - Exceptions (TODO)
- [ ] Protected mode exception delivery
- [ ] Privilege transitions
- [ ] Stack switching via TSS

### Phase 6 - System Instructions (TODO)
- [ ] LGDT/LIDT
- [ ] MOV CR*
- [ ] Task switching

## Notes

All tests compile with `-DCPU_386` to enable i386-specific code paths.

Tests can be disabled by commenting out `#define CPU_386` in Config.h.
