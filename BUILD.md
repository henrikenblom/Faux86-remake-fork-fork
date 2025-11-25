# Building Faux86-remake for Raspberry Pi

This document provides step-by-step instructions for building Faux86-remake for Raspberry Pi 3 (kernel8-32.img).

## Prerequisites

### Build System Requirements
- Linux-based operating system (tested on Ubuntu 22.04)
- ARM cross-compilation toolchain: `arm-none-eabi-gcc`
- Standard build tools: `make`, `git`

### Install ARM Toolchain on Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi g++-arm-none-eabi binutils-arm-none-eabi
```

## Exact Version Requirements

**CRITICAL**: These exact versions must be used for a successful build that works on hardware.

### Faux86-remake
- **Branch**: `develop`
- **Commit**: `18ada23` (includes case sensitivity fixes)
- **Based on**: `a5ad6e8` (parent of b1f4c24 which created the v1.0 prebuilt)

### Circle SDK
- **Repository**: https://github.com/rsta2/circle
- **Commit**: `d486a51dad128de8c0eafedbdc53cda3aa557c40`
- **Date**: July 2023
- **Reason**: This is the last version that:
  - Does NOT include `dwhcixactqueue.cpp` causing runtime assertion failures
  - DOES include all required vc4 and linux addons
  - Matches the .cpp file list from the working v1.0 prebuilt kernel

## Build Instructions

### Step 1: Clone and Setup Circle SDK

```bash
# Clone Circle SDK to a parallel directory
cd ~/projects
git clone https://github.com/rsta2/circle.git
cd circle

# Checkout the exact commit
git checkout d486a51dad128de8c0eafedbdc53cda3aa557c40

# Copy and configure Config.mk
cp Config.mk.sample Config.mk
```

Edit `Config.mk` and set:
```makefile
RASPPI=3
AARCH=32
PREFIX=arm-none-eabi-
DEFINE += -DKERNEL_MAX_SIZE=0x2000000
DEFINE += -DNO_SDHOST
DEFINE += -DNO_BUSY_WAIT
DEFINE += -DNO_REALTIME
DEFINE += -DNO_USB_SOF_INTR
```

### Step 2: Build Circle SDK

```bash
# Build all Circle libraries
./makeall --nosample

# Build required addons
cd addon/SDCard && make && cd ../..
cd addon/vc4/sound && make && cd ../../..
cd addon/vc4/vchiq && make && cd ../../..
cd addon/linux && make && cd ../..
cd addon/fatfs && make && cd ../..
cd addon/Properties && make && cd ../..
```

### Step 3: Clone Faux86-remake

```bash
# Clone Faux86-remake repository
cd ~/projects
git clone https://github.com/[your-repo]/Faux86-remake-fork-fork.git
cd Faux86-remake-fork-fork

# Checkout develop branch
git checkout develop
```

### Step 4: Configure Faux86 Build

Edit `pi/Config.mk` to ensure:
```makefile
RASPPI=1  # Note: Set to 1 for Faux86, not 3
```

Edit `pi/Makefile` and verify the Circle path points to your Circle SDK:
```makefile
CIRCLEHOME = ../../circle
```

Adjust the path if your Circle SDK is in a different location.

### Step 5: Build Faux86

```bash
cd pi
make clean
make
```

### Step 6: Verify Build

The build should complete successfully and produce:
- `kernel8-32.img` (~448KB)
- `kernel8-32.elf`
- `kernel8-32.lst`
- `kernel8-32.map`

## Build Output Verification

To verify your build matches the working prebuilt:

```bash
# Extract .cpp strings from your build
strings kernel8-32.img | grep "\.cpp$" | sort > /tmp/new_build_cpp.txt

# Compare with prebuilt (should differ by only purecall.cpp)
diff ../pre_cpp.txt /tmp/new_build_cpp.txt
```

Expected difference:
```
45d44
< purecall.cpp
```

This minor difference is acceptable and doesn't affect functionality.

## Troubleshooting

### Runtime Assertion Errors

If you get `synchronize.cpp(100): assertion failed: nTargetLevel == IRQ_LEVEL || nTargetLevel == FIQ_LEVEL`:
- You're using the wrong Circle SDK version
- Use commit `d486a51dad128de8c0eafedbdc53cda3aa557c40` exactly

### Multiple Definition Errors

If you get linker errors about `CleanAndInvalidateDataCacheRange`:
- Circle SDK was built with a different RASPPI value
- Clean and rebuild Circle with RASPPI=3 in Config.mk

### Case Sensitivity Errors

If you get errors about missing header files (RAM.h, ports.h, etc.):
- Use the `develop` branch which includes case sensitivity fixes
- The fixes are in commit `18ada23`

## Docker Build (Recommended)

For a reproducible build environment, use Docker:

```bash
# Build the Docker image
docker build -t faux86-builder .

# Run the build
docker run -v $(pwd):/workspace faux86-builder
```

See `Dockerfile` for details.

## Deployment to Raspberry Pi

1. Format an SD card as FAT32
2. Copy the following files to the SD card:
   - `kernel8-32.img` (your built kernel)
   - Raspberry Pi firmware files (bootcode.bin, start.elf, etc.)
   - `config.txt` (Raspberry Pi configuration)
   - `faux86-3.cfg` (Faux86 settings)
   - BIOS files (pcxtbios.bin, videorom.bin, etc.)
   - Disk images (fd0.img, hd0.img, etc.)

3. Insert SD card into Raspberry Pi 3 and power on

## Known Working Configuration

This build process was verified on:
- Ubuntu 22.04 LTS
- Linux kernel 6.8.0-60-generic
- gcc-arm-none-eabi 10.3.1
- GNU Make 4.3
- Target: Raspberry Pi 3 Model B/B+

Build date: 2025-11-25

## Credits

Build instructions compiled with assistance from Claude Code.

Based on Faux86-remake by Curtis (ArnoldUK)
Using Circle SDK by RSTA2
