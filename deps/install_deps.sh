# Install standard build tools
sudo dnf install git gcc-c++ meson ninja-build bison flex cmake -y

# Install python mako
sudo dnf install python3-mako -y

# Install graphics dependencies
sudo dnf install libdrm-devel libxshmfence-devel zlib-devel elfutils-libelf-devel \
    expat-devel libX11-devel libXext-devel libXxf86vm-devel libXfixes-devel \
    libxcb-devel libXrandr-devel libXdamage-devel -y

# Install glslangValidator
sudo dnf install glslang -y

# Install LLVM (Required for r600 to compile shaders)
sudo dnf install llvm-devel clang-devel -y

# Install Wayland/Protocols (Often required even for headless builds)
sudo dnf install wayland-devel wayland-protocols-devel -y
