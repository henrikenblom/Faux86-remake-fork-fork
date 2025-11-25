# Docker Build Instructions for Faux86-remake

This document explains how to build Faux86-remake using Docker for a completely reproducible build environment.

## Why Use Docker?

- **Reproducible builds**: Exact same environment every time
- **No local setup**: No need to install ARM toolchain on your system
- **Clean environment**: Isolated from your host system
- **Works everywhere**: Linux, macOS, Windows (with Docker Desktop)

## Prerequisites

- Docker installed on your system
  - Linux: Install from your package manager
  - macOS/Windows: Install [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- Optional: Docker Compose (usually included with Docker Desktop)

## Quick Start

### Option 1: Using Docker Compose (Recommended)

```bash
# Build the Docker image
docker-compose build

# Run the build
docker-compose run --rm builder
```

### Option 2: Using Docker Directly

```bash
# Build the Docker image
docker build -t faux86-builder .

# Run the build
docker run --rm -v $(pwd):/workspace faux86-builder
```

For Windows PowerShell:
```powershell
docker run --rm -v ${PWD}:/workspace faux86-builder
```

## What Happens During Build

The Docker build process:

1. **Creates Ubuntu 22.04 base image**
2. **Installs ARM cross-compilation toolchain**
   - gcc-arm-none-eabi
   - binutils-arm-none-eabi
   - libnewlib-arm-none-eabi
3. **Clones Circle SDK** at exact commit `d486a51d`
4. **Configures Circle** for Raspberry Pi 3 (32-bit ARM)
5. **Builds all Circle libraries and addons**
   - Main Circle library
   - SDCard addon
   - vc4/sound addon
   - vc4/vchiq addon
   - linux addon
   - fatfs addon
   - Properties addon
6. **Mounts your Faux86 source** directory
7. **Builds Faux86** kernel8-32.img
8. **Verifies build** output

## Build Output

After successful build:
- Output file: `pi/kernel8-32.img`
- Size: ~448KB
- Ready for deployment to Raspberry Pi 3

## Troubleshooting

### Permission Issues (Linux)

If you get permission errors on the output files:

```bash
# Run with your user ID
docker run --rm -v $(pwd):/workspace --user $(id -u):$(id -g) faux86-builder
```

### Docker Build Fails

If the Docker image build fails:

```bash
# Clean Docker cache and rebuild
docker system prune -a
docker-compose build --no-cache
```

### Circle SDK Clone Issues

If git clone fails in Docker build:

```bash
# Build with network debugging
docker build --progress=plain -t faux86-builder .
```

## Advanced Usage

### Interactive Shell

To explore the build environment:

```bash
docker run --rm -it -v $(pwd):/workspace faux86-builder /bin/bash
```

### Custom Circle Path

To use a different Circle SDK location:

```bash
docker run --rm -v $(pwd):/workspace -v /path/to/circle:/build/circle faux86-builder
```

### Build for Different Target

Edit `Dockerfile` and change the RASPPI value in Circle configuration:

```dockerfile
echo 'RASPPI=2' >> Config.mk  # For Raspberry Pi 2
```

## Development Workflow

### Recommended Workflow

1. Make changes to Faux86 source code on your host
2. Run Docker build to compile
3. Copy `pi/kernel8-32.img` to SD card
4. Test on Raspberry Pi
5. Repeat

### Fast Iteration

For faster rebuilds (skip Docker image rebuild):

```bash
# First time: build image
docker-compose build

# Subsequent builds: just compile
docker-compose run --rm builder
```

The Docker image caches the Circle SDK build, so rebuilds are fast (~30 seconds).

## Docker Image Details

- **Base image**: ubuntu:22.04
- **Image size**: ~1.5 GB (includes toolchain and Circle SDK)
- **Circle SDK**: Pre-built at commit d486a51d
- **Toolchain**: gcc-arm-none-eabi 10.3.1

## Continuous Integration

You can use this Dockerfile in CI/CD pipelines:

### GitHub Actions Example

```yaml
name: Build Faux86

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build with Docker
        run: |
          docker build -t faux86-builder .
          docker run --rm -v $PWD:/workspace faux86-builder
      - name: Upload kernel
        uses: actions/upload-artifact@v3
        with:
          name: kernel8-32.img
          path: pi/kernel8-32.img
```

## Cleanup

To remove the Docker image and free disk space:

```bash
# Remove just the Faux86 builder image
docker rmi faux86-builder

# Remove all unused Docker images
docker image prune -a
```

## Credits

Docker build environment created with assistance from Claude Code.
