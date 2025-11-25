/*
  Test suite for Phase 1: i386 Register Architecture

  Tests:
  - 32-bit register access (EAX, ECX, EDX, etc.)
  - 16-bit register access (AX, CX, DX, etc.)
  - 8-bit register access (AL, AH, CL, CH, etc.)
  - Register overlapping (AL/AH within AX within EAX)
  - Control register bit definitions
*/

#include <cstdint>
#include <cstdio>
#include <cstring>

// Simulate the CPU register union
union _bytewordregs_
{
#ifdef CPU_386
    uint32_t dwordregs[8];  // EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
#endif
    uint16_t wordregs[8];   // AX, CX, DX, BX, SP, BP, SI, DI
    uint8_t byteregs[8];    // AL, CL, DL, BL, AH, CH, DH, BH (endian-dependent)
};

// Register indices
#define regax 0
#define regcx 1
#define regdx 2
#define regbx 3

#ifdef __BIG_ENDIAN__
#define regal 1
#define regah 0
#else
#define regal 0
#define regah 1
#endif

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("Running test: %s\n", #name); \
    test_##name(); \
} while(0)

#define ASSERT_EQ(actual, expected) do { \
    if ((actual) == (expected)) { \
        tests_passed++; \
    } else { \
        printf("  FAIL: Expected 0x%X, got 0x%X\n", (unsigned)(expected), (unsigned)(actual)); \
        tests_failed++; \
    } \
} while(0)

#define ASSERT_TRUE(condition) do { \
    if (condition) { \
        tests_passed++; \
    } else { \
        printf("  FAIL: Condition failed: %s\n", #condition); \
        tests_failed++; \
    } \
} while(0)

TEST(16bit_register_access)
{
    _bytewordregs_ regs;
    memset(&regs, 0, sizeof(regs));

    // Test 16-bit register write and read
    regs.wordregs[regax] = 0x1234;
    ASSERT_EQ(regs.wordregs[regax], 0x1234);

    regs.wordregs[regcx] = 0x5678;
    ASSERT_EQ(regs.wordregs[regcx], 0x5678);

    // Ensure registers are independent
    ASSERT_EQ(regs.wordregs[regax], 0x1234);
}

TEST(8bit_register_access)
{
    _bytewordregs_ regs;
    memset(&regs, 0, sizeof(regs));

    // Test 8-bit register write and read
    regs.byteregs[regal] = 0xAB;
    ASSERT_EQ(regs.byteregs[regal], 0xAB);

    regs.byteregs[regah] = 0xCD;
    ASSERT_EQ(regs.byteregs[regah], 0xCD);

    // AL and AH should combine to form AX
    ASSERT_EQ(regs.wordregs[regax], 0xCDAB);
}

#ifdef CPU_386
TEST(32bit_register_access)
{
    _bytewordregs_ regs;
    memset(&regs, 0, sizeof(regs));

    // Test 32-bit register write and read
    regs.dwordregs[regax] = 0x12345678;
    ASSERT_EQ(regs.dwordregs[regax], 0x12345678);

    regs.dwordregs[regcx] = 0xABCDEF01;
    ASSERT_EQ(regs.dwordregs[regcx], 0xABCDEF01);

    // Ensure registers are independent
    ASSERT_EQ(regs.dwordregs[regax], 0x12345678);
}

TEST(register_overlapping)
{
    _bytewordregs_ regs;
    memset(&regs, 0, sizeof(regs));

    // Set 32-bit value
    regs.dwordregs[regax] = 0x12345678;

    // Check 16-bit view (low word)
    ASSERT_EQ(regs.wordregs[regax], 0x5678);

    // Check 8-bit views
    ASSERT_EQ(regs.byteregs[regal], 0x78);
    ASSERT_EQ(regs.byteregs[regah], 0x56);

    // Modify 8-bit register
    regs.byteregs[regal] = 0xAB;

    // Check 32-bit value updated
    ASSERT_EQ(regs.dwordregs[regax], 0x123456AB);

    // Check 16-bit view updated
    ASSERT_EQ(regs.wordregs[regax], 0x56AB);
}

TEST(control_register_bits)
{
    // CR0 bit definitions
    #define CR0_PE  0x00000001
    #define CR0_PG  0x80000000

    uint32_t cr0 = 0;

    // Test PE bit (Protected Mode Enable)
    cr0 |= CR0_PE;
    ASSERT_TRUE(cr0 & CR0_PE);
    ASSERT_TRUE((cr0 & 0x00000001) != 0);

    // Test PG bit (Paging Enable)
    cr0 |= CR0_PG;
    ASSERT_TRUE(cr0 & CR0_PG);
    ASSERT_TRUE((cr0 & 0x80000000) != 0);

    // Both bits should be set
    ASSERT_EQ(cr0, 0x80000001);
}

TEST(eflags_bits)
{
    // EFLAGS bit definitions
    #define EFLAGS_CF   0x00000001
    #define EFLAGS_ZF   0x00000040
    #define EFLAGS_SF   0x00000080
    #define EFLAGS_IF   0x00000200

    uint32_t eflags = 0;

    // Test individual flag bits
    eflags |= EFLAGS_CF;
    ASSERT_TRUE(eflags & EFLAGS_CF);

    eflags |= EFLAGS_ZF;
    ASSERT_TRUE(eflags & EFLAGS_ZF);

    eflags |= EFLAGS_SF;
    ASSERT_TRUE(eflags & EFLAGS_SF);

    eflags |= EFLAGS_IF;
    ASSERT_TRUE(eflags & EFLAGS_IF);

    // Check combined value
    ASSERT_EQ(eflags, 0x000002C1);
}
#endif

int main()
{
    printf("=== Phase 1: Register Architecture Tests ===\n\n");

    RUN_TEST(16bit_register_access);
    RUN_TEST(8bit_register_access);

#ifdef CPU_386
    printf("\n=== i386-specific tests ===\n");
    RUN_TEST(32bit_register_access);
    RUN_TEST(register_overlapping);
    RUN_TEST(control_register_bits);
    RUN_TEST(eflags_bits);
#else
    printf("\n=== i386 tests skipped (CPU_386 not defined) ===\n");
#endif

    printf("\n=== Test Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
