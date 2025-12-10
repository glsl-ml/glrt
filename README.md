# glrt
Runtime around GL and VK

## Install Mesa on Fedora
### Prereqs:
```sh
# Install standard build tools
sudo dnf install git gcc-c++ meson ninja-build bison flex

# Install graphics dependencies
sudo dnf install libdrm-devel libxshmfence-devel zlib-devel elfutils-libelf-devel \
    expat-devel libX11-devel libXext-devel libXf86vm-devel libXfixes-devel \
    libxcb-devel libXrandr-devel libXdamage-devel

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
```sh
meson setup build/ \
  -Dgallium-drivers=r600 \
  -Dvulkan-drivers=[] \
  -Dplatforms=x11,wayland \
  -Dglx=dri \
  -Dbuildtype=debugoptimized \
  -Dllvm=enabled
```

#### Build
```sh
ninja -C build/
```

#### Test
Create dev_shell.sh file

```sh
#!/bin/bash

# Get the absolute path to the current directory
MESA_ROOT=$(pwd)

# Point to the local EGL / GL libraries
export LD_LIBRARY_PATH="$MESA_ROOT/build/src/egl:$MESA_ROOT/build/src/glx:$MESA_ROOT/build/src/gbm:$LD_LIBRARY_PATH"

# Point to the local DRI drivers (where r600_dri.so lives)
export LIBGL_DRIVERS_PATH="$MESA_ROOT/build/src/gallium/targets/dri"
export EGL_DRIVERS_PATH="$MESA_ROOT/build/src/egl/drivers/dri2"

# Debugging flags (Optional, but helpful)
# export EGL_LOG_LEVEL=debug
# export LIBGL_DEBUG=verbose

echo "--- Mesa Dev Environment Active ---"
echo "Drivers Path: $LIBGL_DRIVERS_PATH"
exec "$@"
```

```sh
chmod +x dev_shell.sh
```

Run your test
```sh
./dev_shell.sh ./app
```

