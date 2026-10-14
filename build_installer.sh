#!/bin/bash
# Builds a native macOS .pkg installer for Vocal Rider VST.
#
# The resulting installer:
#   - authenticates via the standard macOS Installer.app flow (Touch ID or
#     admin password) when the user chooses to install for all users
#   - lets the user pick between the two standard, Reaper-scanned VST3
#     locations (system-wide vs. current user only) on the native
#     "Install for <disk>" destination screen
#   - only offers itself on Apple Silicon (arm64) Macs, matching this build
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION="1.0.0"
IDENTIFIER="com.openrider.vocalridervst"

BUILD_VST3="$SCRIPT_DIR/build/VocalRiderVST_artefacts/Release/VST3/Vocal Rider VST.vst3"
WORK_DIR="$SCRIPT_DIR/installer/work"
OUT_PKG="$SCRIPT_DIR/Vocal Rider VST Installer.pkg"

if [ ! -d "$BUILD_VST3" ]; then
    echo "Nie znaleziono zbudowanego pliku VST3: $BUILD_VST3" >&2
    echo "Uruchom najpierw: cmake --build build --config Release" >&2
    exit 1
fi

rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/payload"
cp -R "$BUILD_VST3" "$WORK_DIR/payload/"

pkgbuild \
    --root "$WORK_DIR/payload" \
    --install-location "Library/Audio/Plug-Ins/VST3" \
    --identifier "$IDENTIFIER" \
    --version "$VERSION" \
    "$WORK_DIR/VocalRiderVST.pkg"

productbuild \
    --distribution "$SCRIPT_DIR/installer/Distribution.xml" \
    --resources "$SCRIPT_DIR/installer/Resources" \
    --package-path "$WORK_DIR" \
    "$OUT_PKG"

echo ""
echo "Zbudowano instalator -> $OUT_PKG"
echo "Uwaga: pakiet nie jest podpisany certyfikatem Apple Developer ID -"
echo "przy pierwszym uruchomieniu na innym komputerze moze byc wymagane"
echo "\"kliknij prawym -> Otworz\" lub zezwolenie w Ustawieniach systemowych."
