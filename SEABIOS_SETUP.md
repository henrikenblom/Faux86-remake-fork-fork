# SeaBIOS Setup for i386 Support

**Status:** ✅ RECOMMENDED for i386 builds
**Updated:** 2025-11-26

## Why SeaBIOS is Required for i386

The default `pcxtbios.bin` is an IBM PC/XT BIOS that:
- Only recognizes 8086/V20 CPUs
- Doesn't provide **A20 gate control** (critical for extended memory!)
- Doesn't report extended memory beyond 1MB
- Can't support HIMEM.SYS or Windows 95

This causes:
- Boot message shows "V20 (No FPU)" instead of "386"
- **"Unable to control A20 line" error** (fatal for extended memory access)
- HIMEM.SYS fails to load
- Windows 95 can't access extended memory

## Why SeaBIOS? (Better than INT 15h Interception)

An earlier attempt used CPU-level INT 15h interception, but this only solved memory detection - **it didn't provide A20 gate control**, which is essential.

SeaBIOS provides the complete solution:
- ✅ **A20 gate control** (enables extended memory access)
- ✅ INT 15h memory detection services
- ✅ Proper i386 CPU identification
- ✅ System configuration services
- ✅ Professional, well-tested implementation

## Solution: Use SeaBIOS

SeaBIOS is an open-source BIOS that provides everything i386 needs:
- **A20 gate control** - enables access to memory above 1MB
- i386 CPU detection and identification
- Extended memory reporting (INT 15h services)
- System configuration (INT 15h, AH=C0h)
- Full HIMEM.SYS support
- Windows 95 compatibility

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

### Step 3: Update Config File

You need to tell Faux86 to use SeaBIOS instead of pcxtbios.bin.

**Option A: Edit config file manually**
```bash
# On your SD card, edit pi/bin/faux86-3.cfg
# Change this line:
biosrom=pcxtbios.bin

# To this:
biosrom=seabios.bin
```

**Option B: Use sed (quick method)**
```bash
# On your SD card or before copying to SD card:
cd pi/bin
sed -i 's/biosrom=pcxtbios.bin/biosrom=seabios.bin/' faux86-3.cfg
```

The config file should be at: `pi/bin/faux86-3.cfg` on your SD card.

### Step 4: Verify Files on SD Card

Make sure you have these files in the right locations:
```
pi/
├── kernel8-32.img          (488,164 bytes - i386 kernel)
└── bin/
    ├── seabios.bin         (128-256 KB - downloaded BIOS)
    ├── faux86-3.cfg        (updated config)
    ├── pcxtbios.bin        (old BIOS, keep as backup)
    └── videorom.bin        (VGA ROM)
```

### Step 5: Boot and Verify

Power cycle your Raspberry Pi. SeaBIOS should:
- Show "SeaBIOS" banner on boot
- Detect CPU as "i386" or "80386"
- Report 32MB of RAM
- **Successfully control A20 line** (no more errors!)
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
