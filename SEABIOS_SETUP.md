# Using SeaBIOS for i386 Support

## Problem

The default `pcxtbios.bin` is an IBM PC/XT BIOS that:
- Only recognizes 8086/V20 CPUs
- Doesn't report extended memory beyond 1MB
- Can't support HIMEM.SYS or Windows 95

This causes:
- Boot message shows "V20 (No FPU)" instead of "386"
- HIMEM.SYS fails to load
- Windows 95 can't access extended memory

## Solution: Use SeaBIOS

SeaBIOS is an open-source BIOS that:
- Supports i386 CPU detection
- Reports extended memory correctly
- Provides INT 15h services for HIMEM.SYS
- Works with Windows 95

## Installation Steps

### Step 1: Download SeaBIOS

```bash
# Download the latest SeaBIOS release
wget https://www.seabios.org/downloads/bios.bin -O seabios.bin

# Or use a specific version
wget https://www.seabios.org/downloads/bios-1.16.3.bin -O seabios.bin
```

### Step 2: Copy to Raspberry Pi SD Card

```bash
# Copy to SD card (adjust path to your SD card mount point)
cp seabios.bin /media/youruser/SDCARD/

# Or if building for Pi, copy to pi/bin directory
cp seabios.bin pi/bin/
```

### Step 3: Update faux86-3.cfg

Edit `faux86-3.cfg` and change the BIOS line:

```ini
# OLD:
biosrom=pcxtbios.bin

# NEW:
biosrom=seabios.bin
```

### Step 4: Reboot

Power cycle your Raspberry Pi. SeaBIOS should:
- Detect CPU as "386" or "i386"
- Report 32MB of RAM
- Allow HIMEM.SYS to load

## Verification

After booting with SeaBIOS:

### 1. Check Boot Message
Should see something like:
```
SeaBIOS (version 1.16.3)
Machine: i386 compatible
```

Instead of:
```
V20 (No FPU)
```

### 2. Test HIMEM.SYS

Add to CONFIG.SYS:
```
DEVICE=HIMEM.SYS
```

Boot and check:
```
C:\> MEM
  ...
  Extended Memory: 31744 KB
```

### 3. Check Available Memory

```
C:\> MEM /C
```

Should show extended memory beyond 1MB.

## Alternative: Build SeaBIOS from Source

If the pre-built binary doesn't work:

```bash
# Clone SeaBIOS repository
git clone https://git.seabios.org/seabios.git
cd seabios

# Build with default config
make

# Output will be in out/bios.bin
cp out/bios.bin ../seabios.bin
```

## Alternative BIOSes

If SeaBIOS doesn't work, other options:

### Option 1: Bochs BIOS
```bash
# Usually found in /usr/share/bochs/ on Linux
cp /usr/share/bochs/BIOS-bochs-latest .
# Rename and use
mv BIOS-bochs-latest bochs-bios.bin
```

Update config:
```ini
biosrom=bochs-bios.bin
```

### Option 2: QEMU BIOS
```bash
# Usually in /usr/share/qemu/
cp /usr/share/qemu/bios.bin qemu-bios.bin
```

Update config:
```ini
biosrom=qemu-bios.bin
```

### Option 3: Extract from Old 386 PC

If you have an old 386/486 PC:
1. Boot to DOS
2. Use DEBUG.COM or similar tool
3. Dump ROM from F000:0000 to F000:FFFF
4. Save as 386bios.bin

## Troubleshooting

### SeaBIOS doesn't boot

Try these settings in faux86-3.cfg:
```ini
# Slower boot
speed=8

# More memory for BIOS
# (may need to adjust kernel)
```

### "ROM not found" error

Make sure:
1. seabios.bin is in the same directory as other ROMs
2. Path in config is correct
3. File permissions are readable

### Still shows V20

Double-check:
1. Config file is actually being read
2. Kernel is the new 476KB version
3. SeaBIOS file is not corrupted:
   ```bash
   ls -lh seabios.bin
   # Should be around 128-256 KB
   ```

## Expected Behavior with SeaBIOS

With SeaBIOS and the i386 kernel:

✅ Boot message: "i386 compatible" or similar
✅ HIMEM.SYS loads successfully
✅ MEM command shows 32MB
✅ Windows 95 can access extended memory
✅ Protected mode works
✅ Paging works

## BIOS Services SeaBIOS Provides

SeaBIOS implements these INT 15h services needed for i386:

- **INT 15h, AH=88h**: Get extended memory size
- **INT 15h, AH=E801h**: Get memory size (newer method)
- **INT 15h, AH=E820h**: Get memory map (Windows 95 uses this)
- **INT 15h, AH=C0h**: Get system configuration

## BIOS vs Kernel

Remember:
- **Kernel (kernel8-32.img)**: The emulator itself (i386 CPU emulation)
- **BIOS (seabios.bin)**: Firmware that initializes hardware and loads OS

Both must support i386:
- ✅ Kernel: Already supports i386 (if 476 KB)
- ❌ BIOS: pcxtbios.bin does NOT support i386
- ✅ Solution: Replace with SeaBIOS

## Summary

1. Download SeaBIOS: `wget https://www.seabios.org/downloads/bios.bin -O seabios.bin`
2. Copy to SD card
3. Update `faux86-3.cfg`: `biosrom=seabios.bin`
4. Reboot

This should fix both issues:
- "V20 (No FPU)" → "i386 compatible"
- HIMEM.SYS failure → HIMEM.SYS loads successfully

---

**After you set up SeaBIOS, the i386 kernel will finally work as intended!**
