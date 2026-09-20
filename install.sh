#!/usr/bin/env bash
# Build and install OmaSuite into the user's local prefix (no root needed to
# *install*; root only needed if dependencies must be fetched via pacman).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$HERE/build"
BIN_DIR="$HOME/.local/bin"
APPS_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons/hicolor/scalable/apps"

# Map a missing tool/module to the Arch package that provides it.
missing=()
add_missing() { # $1 = package name
    case " ${missing[*]} " in
        *" $1 "*) : ;;
        *) missing+=("$1") ;;
    esac
}

command -v qmake6 >/dev/null 2>&1 || add_missing qt6-base
command -v g++     >/dev/null 2>&1 || add_missing gcc
command -v make    >/dev/null 2>&1 || add_missing make
pkg-config --exists Qt6Core           2>/dev/null || add_missing qt6-base
pkg-config --exists Qt6QuickControls2 2>/dev/null || add_missing qt6-declarative
pkg-config --exists Qt6NetworkAuth    2>/dev/null || add_missing qt6-networkauth
pkg-config --exists libsecret-1       2>/dev/null || add_missing libsecret

if [ ${#missing[@]} -gt 0 ]; then
    echo "==> Missing build dependencies: ${missing[*]}"
    echo "==> Installing with pacman (sudo may prompt for your password)..."
    sudo pacman -S --needed "${missing[@]}"
fi

echo "==> Building OmaSuite"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
qmake6 "$HERE/OmaSuite.pro"
make -j"$(nproc)"

echo "==> Installing to $HOME/.local"
mkdir -p "$BIN_DIR" "$APPS_DIR" "$ICON_DIR"
install -Dm755 "$BUILD_DIR/omasuite" "$BIN_DIR/omasuite"
install -Dm644 "$HERE/omasuite.desktop" "$APPS_DIR/omasuite.desktop"
install -Dm644 "$HERE/icons/omasuite.svg" "$ICON_DIR/omasuite.svg"

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$HOME/.local/share/applications" || true
fi

echo "==> Done. Launch with: omasuite"
