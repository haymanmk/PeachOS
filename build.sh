#!/usr/bin/env bash
set -e

# Prepare build environment
echo "Setting up build environment..."
export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# Start build process
echo "Starting build process..."
make clean
make all