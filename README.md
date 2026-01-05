# glrt
Runtime around GL and VK

## Install Mesa on Fedora
### Prereqs:
```sh
# Install standard build tools
sudo dnf install git gcc-c++ meson ninja-build bison flex

# Install python mako
sudo dnf install python3-mako

# Install graphics dependencies
sudo dnf install libdrm-devel libxshmfence-devel zlib-devel elfutils-libelf-devel \
    expat-devel libX11-devel libXext-devel libXxf86vm-devel libXfixes-devel \
    libxcb-devel libXrandr-devel libXdamage-devel

# Install glslangValidator
sudo dnf install glslang

# Install LLVM (Required for r600 to compile shaders)
sudo dnf install llvm-devel clang-devel

# Install Wayland/Protocols (Often required even for headless builds)
sudo dnf install wayland-devel wayland-protocols-devel
```

### Installation
```sh
git clone https://gitlab.freedesktop.org/mesa/mesa.git
cd mesa
```

#### Configure

##### To build `r600` driver:
```sh
rm -rf build_r600/ install_r600/

meson setup build_r600/ \
  --prefix=$(pwd)/install_r600 \
  -Dgallium-drivers=r600 \
  -Dvulkan-drivers=[] \
  -Dplatforms=x11,wayland \
  -Dglx=dri \
  -Dbuildtype=release \
  -Dllvm=enabled

ninja -C build_r600/ install_r600/
```

##### To build `radeonsi` driver for OpenGL:
```sh
rm -rf build_si/ install_si/

meson setup build_si/ \
  --prefix=$(pwd)/install_si \
  -Dgallium-drivers=radeonsi \
  -Dvulkan-drivers=[] \
  -Dplatforms=x11,wayland \
  -Dglx=dri \
  -Dbuildtype=release \
  -Dllvm=enabled

ninja -C build_si/ install_si/
```

##### To build `radv` driver for Vulkan:
```sh
rm -rf build_radv/ install_radv/

meson setup build_radv/ \
  --prefix=$(pwd)/install_radv \
  -Dgallium-drivers=[] \
  -Dvulkan-drivers=amd \
  -Dplatforms=x11,wayland \
  -Dbuildtype=release \
  -Dllvm=enabled

ninja -C build_radv/ install_radv/
```


#### Test
Create dev_shell.sh file

##### For `r600`:
```sh
cat << 'EOF' > dev_shell_r600.sh
#!/bin/bash
MESA_INSTALL="$(pwd)/install_r600"

# 1. Point to the Installed Libraries (EGL, GL)
export LD_LIBRARY_PATH="$MESA_INSTALL/lib64:$MESA_INSTALL/lib:$LD_LIBRARY_PATH"

# 2. Point to the Installed Drivers (DRI)
export LIBGL_DRIVERS_PATH="$MESA_INSTALL/lib64/dri:$MESA_INSTALL/lib/dri"
export EGL_DRIVERS_PATH="$LIBGL_DRIVERS_PATH"

# 3. Verify
echo "--- Legacy AMD (r600) Dev Environment ---"
echo "Drivers: $LIBGL_DRIVERS_PATH"
echo "Running: $@"
echo "-----------------------------------------"

exec "$@"
EOF
chmod +x dev_shell_r600.sh
```

##### For `radeonsi`:
```sh
cat << 'EOF' > dev_shell_si.sh
#!/bin/bash
MESA_INSTALL="$(pwd)/install_si"

# 1. Point to the Installed Libraries (EGL, GL)
export LD_LIBRARY_PATH="$MESA_INSTALL/lib64:$MESA_INSTALL/lib:$LD_LIBRARY_PATH"

# 2. Point to the Installed Drivers (DRI)
export LIBGL_DRIVERS_PATH="$MESA_INSTALL/lib64/dri:$MESA_INSTALL/lib/dri"
export EGL_DRIVERS_PATH="$LIBGL_DRIVERS_PATH"

# 3. Verify
echo "--- Modern AMD (radeonsi) Dev Environment ---"
echo "Drivers: $LIBGL_DRIVERS_PATH"
echo "Running: $@"
echo "---------------------------------------------"

exec "$@"
EOF
chmod +x dev_shell_si.sh
```

##### For `radv`:
```sh
cat << 'EOF' > dev_shell_radv.sh
#!/bin/bash
MESA_INSTALL="$(pwd)/install_radv"

# 1. Locate the JSON Manifest
# Meson installs this into share/vulkan/icd.d/
# The filename is usually radeon_icd.x86_64.json
ICD_FILE="$MESA_INSTALL/share/vulkan/icd.d/radeon_icd.x86_64.json"

# Safety Check
if [ ! -f "$ICD_FILE" ]; then
    echo "Error: Could not find Vulkan ICD manifest at:"
    echo "$ICD_FILE"
    exit 1
fi

# 2. Tell Vulkan Loader to use ONLY this driver
# (Remove this export if you want to see both system and custom drivers)
export VK_ICD_FILENAMES="$ICD_FILE"

# 3. Verify
echo "--- Vulkan (RADV) Dev Environment ---"
echo "ICD Config: $VK_ICD_FILENAMES"
echo "Running: $@"
echo "-------------------------------------"

exec "$@"
EOF
chmod +x dev_shell_radv.sh
```

##### Run your test
```sh
./dev_shell_r600.sh eglxinfo
./dev_shell_si.sh eglxinfo
./dev_shell_radv.sh vulkaninfo
```
