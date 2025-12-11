#!/bin/bash

# --- Styling & Colors ---
BOLD="\033[1m"
RED="\033[31m"
GREEN="\033[32m"
YELLOW="\033[33m"
BLUE="\033[34m"
MAGENTA="\033[35m"
CYAN="\033[36m"
WHITE="\033[37m"
RESET="\033[0m"
CLEAR_LINE="\033[K"

# --- Logging Helpers ---
print_header() {
    echo -e "\n${BOLD}${MAGENTA}╔══════════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${BOLD}${MAGENTA}║  $1${RESET}"
    echo -e "${BOLD}${MAGENTA}╚══════════════════════════════════════════════════════════════╝
            printf -v void "%${empty}s" ""
            
            # Draw the bar (using \r to overwrite line)
            # Bar format: [#####.....] 45% (120/500)
            echo -ne "\r${CLEAR_LINE}  ${BOLD}${BLUE}[${GREEN}${fill// /#}${RESET}${void// /.} ${BLUE}]${RESET} ${BOLD}${percent}%${RESET} (${curr}/${total})"
        else
            # If the line is NOT a progress update (e.g. a warning or error),
            # clear the progress bar line, print the message, and start a new line.
            echo -ne "\r${CLEAR_LINE}"
            echo "$line"
        fi
    done
    # Final newline to clear the last progress bar state
    echo ""
}

# --- Argument Parsing ---
TARGETS="r600 radeonsi radv" # Default

if [ ! -z "$1" ]; then
    TARGETS=$(echo "$1" | tr ',' ' ')
fi

print_header "Mesa Advanced Builder"
log_info "Selected Drivers: ${BOLD}$TARGETS${RESET}"

# --- Clone Check ---
if [ ! -d "mesa" ]; then
    log_warn "Mesa repository not found."
    log_task "Cloning Mesa source..."
    git clone --depth 1 https://gitlab.freedesktop.org/mesa/mesa.git
    if [ $? -ne 0 ]; then log_err "Clone failed."; exit 1; fi
    log_success "Clone complete."
else
    log_success "Mesa repository found."
fi

cd mesa

# --- Build Driver Function ---
build_driver() {
    local NAME=$1
    local GALLIUM=$2
    local VULKAN=$3
    
    local BUILD_DIR="build_${NAME}"
    local INSTALL_DIR="$(pwd)/install_${NAME}"

    print_header "Target: ${NAME}"
    
    # 1. Cleanup
    if [ -d "$BUILD_DIR" ]; then
        log_task "Cleaning old build artifacts..."
        rm -rf "$BUILD_DIR" "$INSTALL_DIR"
    fi

    # 2. Config
    echo -e "${YELLOW}» Configuration:${RESET}"
    echo -e "  • Gallium: ${BOLD}${GALLIUM}${RESET}"
    echo -e "  • Vulkan:  ${BOLD}${VULKAN}${RESET}"
    echo -e "  • Prefix:  ${BOLD}${INSTALL_DIR}${RESET}"
    echo ""

    # 3. Meson (Silent config)
    log_task "Configuring Build System..."
    meson setup "$BUILD_DIR" \
      --prefix="$INSTALL_DIR" \
      -Dgallium-drivers="$GALLIUM" \
      -Dvulkan-drivers="$VULKAN" \
      -Dplatforms=x11,wayland \
      -Dbuildtype=release \
      -Dllvm=enabled > "meson_log_${NAME}.txt" 2>&1

    if [ $? -ne 0 ]; then
        log_err "Configuration failed! See meson_log_${NAME}.txt"
        return 1
    fi
    log_success "Configured."

    # 4. Compile (With Custom Progress Bar)
    log_task "Compiling..."
    
    # We pipe Ninja into our drawing function
    # stdbuf -oL forces line-buffering so the bar updates smoothly
    if command -v stdbuf &> /dev/null; then
        stdbuf -oL ninja -C "$BUILD_DIR" | draw_progress_bar
    else
        # Fallback if stdbuf is missing (might be jumpy)
        ninja -C "$BUILD_DIR" | draw_progress_bar
    fi

    # Check for ninja failure
    # (Pipestatus array captures the exit code of ninja, not the drawing function)
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        log_err "Compilation Failed for $NAME."
        return 1
    fi
    
    # 5. Install (Silent)
    log_task "Installing..."
    ninja -C "$BUILD_DIR" install > /dev/null 2>&1
    
    log_success "${BOLD}$NAME${RESET} Installed Successfully!"
    echo -e "  -> Path: ${CYAN}${INSTALL_DIR}${RESET}"
}

# --- Execution Loop ---
for target in $TARGETS; do
    case $target in
        r600)     build_driver "r600" "r600" "[]" ;;
        radeonsi) build_driver "radeonsi" "radeonsi" "[]" ;;
        radv)     build_driver "radv" "[]" "amd" ;;
        *)        log_warn "Skipping unknown target: $target" ;;
    esac
done

print_header "Build Process Complete"
