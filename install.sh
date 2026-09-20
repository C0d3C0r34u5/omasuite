#!/usr/bin/env bash
# Build and install OmaSuite into the user's local prefix (no root needed).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$HERE/build"
BIN_DIR="$HOME/.local/bin"
APPS_DIR="$HOME/.local/share/applications"
ICON_DIR="$HOME/.local/share/icons/hicolor/scalable/apps"

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
