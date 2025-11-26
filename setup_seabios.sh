#!/bin/bash
# SeaBIOS Setup Script for i386 Support
# This script downloads SeaBIOS and configures Faux86 to use it

set -e  # Exit on error

echo "=================================="
echo "SeaBIOS Setup for Faux86 i386"
echo "=================================="
echo ""

# Check if we're in the right directory
if [ ! -d "pi/bin" ]; then
    echo "Error: pi/bin directory not found!"
    echo "Please run this script from the Faux86-remake-fork-fork root directory."
    exit 1
fi

# Step 1: Download SeaBIOS
echo "Step 1: Downloading SeaBIOS..."
if [ -f "pi/bin/seabios.bin" ]; then
    echo "  seabios.bin already exists. Skipping download."
    echo "  (Delete pi/bin/seabios.bin to re-download)"
else
    echo "  Downloading SeaBIOS 1.17.0 from seabios.org..."
    wget -q https://www.seabios.org/downloads/bios.bin-1.17.0.gz -O /tmp/seabios.gz

    if [ $? -eq 0 ]; then
        echo "  ✓ Downloaded, extracting..."
        gunzip -c /tmp/seabios.gz > pi/bin/seabios.bin
        rm /tmp/seabios.gz
        echo "  ✓ Extraction complete (256 KB)"
        ls -lh pi/bin/seabios.bin
    else
        echo "  ✗ Download failed!"
        echo "  Try manually: "
        echo "    wget https://www.seabios.org/downloads/bios.bin-1.17.0.gz"
        echo "    gunzip bios.bin-1.17.0.gz"
        echo "    mv bios.bin-1.17.0 pi/bin/seabios.bin"
        exit 1
    fi
fi

# Step 2: Backup original config
echo ""
echo "Step 2: Backing up config file..."
if [ -f "pi/bin/faux86-3.cfg.bak" ]; then
    echo "  Backup already exists: pi/bin/faux86-3.cfg.bak"
else
    cp pi/bin/faux86-3.cfg pi/bin/faux86-3.cfg.bak
    echo "  ✓ Created backup: pi/bin/faux86-3.cfg.bak"
fi

# Step 3: Update config to use SeaBIOS
echo ""
echo "Step 3: Updating config to use SeaBIOS..."
if grep -q "biosrom=seabios.bin" pi/bin/faux86-3.cfg; then
    echo "  Config already uses SeaBIOS. No changes needed."
else
    sed -i 's/biosrom=pcxtbios.bin/biosrom=seabios.bin/' pi/bin/faux86-3.cfg
    echo "  ✓ Updated faux86-3.cfg"
    echo "    Changed: biosrom=pcxtbios.bin"
    echo "    To:      biosrom=seabios.bin"
fi

# Step 4: Verify setup
echo ""
echo "Step 4: Verifying setup..."
echo ""

# Check kernel
if [ -f "pi/kernel8-32.img" ]; then
    KERNEL_SIZE=$(stat -c%s pi/kernel8-32.img 2>/dev/null || stat -f%z pi/kernel8-32.img 2>/dev/null)
    if [ "$KERNEL_SIZE" = "488164" ]; then
        echo "  ✓ i386 kernel present: pi/kernel8-32.img ($KERNEL_SIZE bytes)"
    else
        echo "  ⚠ Warning: kernel8-32.img size is $KERNEL_SIZE bytes (expected 488164)"
        echo "    You may need to rebuild the kernel."
    fi
else
    echo "  ✗ Error: pi/kernel8-32.img not found!"
    echo "    Build the kernel first: cd pi && make"
    exit 1
fi

# Check SeaBIOS
if [ -f "pi/bin/seabios.bin" ]; then
    BIOS_SIZE=$(stat -c%s pi/bin/seabios.bin 2>/dev/null || stat -f%z pi/bin/seabios.bin 2>/dev/null)
    echo "  ✓ SeaBIOS present: pi/bin/seabios.bin ($BIOS_SIZE bytes)"
else
    echo "  ✗ Error: pi/bin/seabios.bin not found!"
    exit 1
fi

# Check config
if grep -q "biosrom=seabios.bin" pi/bin/faux86-3.cfg; then
    echo "  ✓ Config file updated: biosrom=seabios.bin"
else
    echo "  ✗ Error: Config file not updated correctly!"
    exit 1
fi

# Check video ROM
if [ -f "pi/bin/videorom.bin" ]; then
    echo "  ✓ Video ROM present: pi/bin/videorom.bin"
else
    echo "  ⚠ Warning: videorom.bin not found (may need VGA ROM)"
fi

echo ""
echo "=================================="
echo "✓ SeaBIOS Setup Complete!"
echo "=================================="
echo ""
echo "Files ready to copy to SD card:"
echo "  - pi/kernel8-32.img (i386 kernel)"
echo "  - pi/bin/seabios.bin (BIOS with A20 support)"
echo "  - pi/bin/faux86-3.cfg (config file)"
echo "  - pi/bin/videorom.bin (VGA ROM)"
echo ""
echo "Next steps:"
echo "  1. Copy entire pi/ directory to your SD card"
echo "  2. Insert SD card into Raspberry Pi 3"
echo "  3. Boot and verify:"
echo "     - SeaBIOS banner appears"
echo "     - CPU shows as 80386"
echo "     - No 'A20 line' errors"
echo "     - HIMEM.SYS loads successfully"
echo ""
echo "To restore original BIOS:"
echo "  cp pi/bin/faux86-3.cfg.bak pi/bin/faux86-3.cfg"
echo ""
