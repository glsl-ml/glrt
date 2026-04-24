#!/bin/bash
set -e

# Target installation directory
PREFIX="$HOME/.local/shaderc"
SRC_DIR="shaderc"

echo "=== Setting up Shaderc (glslc) Build ==="
echo "Install prefix: $PREFIX"

# Clone the Shaderc repository if it doesn't exist
if [ ! -d "$SRC_DIR" ]; then
    echo "Cloning Shaderc repository..."
    git clone https://github.com/google/shaderc.git "$SRC_DIR"
fi

cd "$SRC_DIR"

echo "=== Syncing Dependencies ==="
# This Python script downloads the required versions of glslang, 
# spirv-tools, and spirv-headers into the third_party/ directory.
./utils/git-sync-deps

echo "=== Configuring build with CMake ==="
mkdir -p build
cd build

# Configure with CMake, generating Ninja build files.
# We skip the tests to significantly speed up the build time.
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$HOME/.local/shaderc \
    -DSHADERC_SKIP_TESTS=ON \
    ..

echo "=== Compiling with Ninja ==="
ninja

echo "=== Installing to $PREFIX ==="
ninja install

echo "========================================================"
echo "✅ Shaderc build complete!"
echo "The glslc executable is located at: $PREFIX/bin/glslc"
