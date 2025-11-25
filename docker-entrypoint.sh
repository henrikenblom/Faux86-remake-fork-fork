#!/bin/bash
set -e

echo "============================================"
echo "Faux86-remake Docker Build Environment"
echo "============================================"
echo ""
echo "Circle SDK: d486a51dad128de8c0eafedbdc53cda3aa557c40"
echo "Target: Raspberry Pi 3 (kernel8-32.img)"
echo ""

# Check if workspace is mounted
if [ ! -d "/workspace" ]; then
    echo "ERROR: /workspace directory not found!"
    echo "Mount your Faux86 source directory to /workspace"
    exit 1
fi

cd /workspace

# Check if we're in the right directory
if [ ! -f "pi/Makefile" ]; then
    echo "ERROR: pi/Makefile not found!"
    echo "Make sure you mounted the Faux86-remake root directory to /workspace"
    exit 1
fi

echo "Building Faux86 for Raspberry Pi 3..."
echo ""

# Update Circle path in Makefile if needed
cd pi

# Clean previous build
echo "Cleaning previous build..."
make clean 2>/dev/null || true

# Build
echo "Building kernel8-32.img..."
make CIRCLEHOME=/build/circle

# Verify output
if [ -f "kernel8-32.img" ]; then
    SIZE=$(wc -c < kernel8-32.img)
    echo ""
    echo "============================================"
    echo "Build successful!"
    echo "============================================"
    echo "Output: pi/kernel8-32.img"
    echo "Size: $SIZE bytes"
    echo ""

    # Show strings comparison
    echo "Verifying build..."
    strings kernel8-32.img | grep "\.cpp$" | sort > /tmp/new_cpp.txt

    if [ -f "/workspace/pre_cpp.txt" ]; then
        echo "Comparing with prebuilt kernel..."
        if diff /workspace/pre_cpp.txt /tmp/new_cpp.txt > /tmp/diff.txt 2>&1; then
            echo "✓ Build output matches prebuilt exactly!"
        else
            echo "Build differs from prebuilt:"
            cat /tmp/diff.txt
            echo ""
            echo "Note: Small differences (like purecall.cpp) are acceptable"
        fi
    fi

    echo ""
    echo "Kernel ready for deployment to Raspberry Pi 3"
else
    echo ""
    echo "============================================"
    echo "Build FAILED!"
    echo "============================================"
    exit 1
fi
