# Understanding CPU Type in Faux86

## Important Discovery: cpuType Config Field is NOT Used

After thorough investigation, I discovered that **the `cpuType` field in Config.h is never actually checked or used by the emulator code**. It's a dead configuration option.

### How CPU Mode is Actually Determined

The CPU behavior is controlled **entirely at compile time** by the `CPU_386` preprocessor flag:

```cpp
#ifdef CPU_386
    // 32-bit i386 code executes
#else
    // 16-bit 8086/V20/286 code executes
#endif
```

The `CpuType` enum values (CpuV20, Cpu286, Cpu386) are **never checked** in any of the emulation code.

### What This Means

1. **Config files don't matter** for CPU type
   - `faux86-3.cfg` has NO `cpu=` setting
   - Even if it did, it wouldn't be checked

2. **The cpuType field does nothing**
   - Setting `cpuType = Cpu386` has zero effect
   - It's just a config variable that nothing reads

3. **Only the compile flag matters**
   - If compiled with `-DCPU_386`: i386 mode
   - If compiled without: 8086/V20 mode
   - **No runtime configuration can change this**

## How to Verify i386 Mode is Active

Since config files don't control this, here's how to verify your kernel is truly running i386:

### Method 1: Check Kernel Size
```bash
ls -lh pi/kernel8-32.img
```

- **i386 kernel**: ~487,276 bytes (476 KB)
- **V20 kernel**: ~472,460 bytes (461 KB)

If your kernel is 476 KB, it has i386 code compiled in.

### Method 2: Check Compile Output
When you ran `make`, you should have seen:
```
WC    kernel8-32.img => 487276
```

This confirms i386 was compiled in.

### Method 3: Check Git Log
```bash
git log --oneline -1
```

Should show:
```
648e820 Fix CPU type default - now automatically uses i386 when built with CPU_386
```

### Method 4: Try Running a 32-bit Instruction

The most definitive test is to run software that requires i386:
- Boot Windows 95 (requires 32-bit instructions)
- Run a 32-bit DOS extender program
- If it works, you're in i386 mode
- If it crashes with "Illegal instruction", you're not

## Why You Might Think It's Still V20

**Possible reasons you see "V20" references:**

1. **Old kernel still on SD card**
   - Solution: Verify you copied the new 476KB kernel

2. **Looking at config file comments**
   - The .cfg files may mention V20 in comments
   - These are just documentation, not active settings

3. **Looking at Config.h default**
   - Before my fix, Config.h said `CpuV20`
   - But this field is never used!

4. **Debug output or logs**
   - Some log message might print the cpuType field
   - But it doesn't affect actual CPU behavior

## The Bottom Line

**If your kernel is 476 KB (487,276 bytes), you ARE running i386.**

The compile-time `CPU_386` flag is the ONLY thing that matters. Config files, cpuType settings, and runtime options cannot change the CPU mode - it's baked into the kernel at compile time.

## What I "Fixed" (And Why It Didn't Matter)

I modified Config.h to set:
```cpp
#ifdef CPU_386
    CpuType cpuType = CpuType::Cpu386;
#else
    CpuType cpuType = CpuType::CpuV20;
#endif
```

**This accomplished nothing** because `cpuType` is never checked by any code. It's just a config field that sits there unused.

## Actual Verification Steps

To be 100% certain you're running i386:

1. **Check your SD card**:
   ```bash
   ls -lh /path/to/sdcard/kernel8-32.img
   # Should be 487,276 bytes
   ```

2. **Try to boot Windows 95**:
   - If it boots past the initial loader, i386 is working
   - If it crashes immediately, something's wrong

3. **Check build date**:
   ```bash
   stat pi/kernel8-32.img
   # Should show: Modify: 2025-11-26 12:57
   ```

4. **Verify -DCPU_386 flag**:
   ```bash
   grep CPU_386 pi/Makefile
   # Should show: -DCPU_386 in CPPFLAGS and CFLAGS
   ```

## How to Actually Tell What CPU Mode is Running

Since there's no runtime CPU type configuration, here are definitive tests:

### Test 1: Memory Size
- Run `MEM` command in DOS
- i386 mode: Should see 32MB
- V20 mode: Should see 1MB

### Test 2: Protected Mode
- Try to enter protected mode
- i386: Works
- V20: Crashes or hangs

### Test 3: 32-bit Instructions
- Execute any 32-bit instruction (like `MOVSD`)
- i386: Executes
- V20: Illegal instruction exception

## Conclusion

The `cpuType` configuration field is **legacy/unused code**. The actual CPU mode is determined **100% at compile time** by the `-DCPU_386` flag.

Your current kernel (if 476 KB) **IS** running i386, regardless of what any config file says about "V20".

If you're experiencing issues, they're not related to CPU type configuration - they're either:
1. Wrong kernel deployed to SD card
2. Software bugs in the i386 implementation
3. Missing instructions that need to be added

---

**How are you determining that it's "still using V20"?**

Please let me know what you're seeing that makes you think V20 is active, and I can help diagnose the real issue.
