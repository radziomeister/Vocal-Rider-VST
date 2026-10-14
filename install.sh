#!/bin/bash
# Copies the built VST3 bundle into the standard macOS plugin folder
# (no installer package - just a plain file copy) and ad-hoc code-signs
# it so Gatekeeper doesn't block a locally built, unsigned plugin.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build/VocalRiderVST_artefacts/Release"

VST3_SRC="$BUILD_DIR/VST3/Vocal Rider VST.vst3"
VST3_DEST="$HOME/Library/Audio/Plug-Ins/VST3"

mkdir -p "$VST3_DEST"

if [ -d "$VST3_SRC" ]; then
    rm -rf "$VST3_DEST/Vocal Rider VST.vst3"
    cp -R "$VST3_SRC" "$VST3_DEST/"
    codesign --force --deep --sign - "$VST3_DEST/Vocal Rider VST.vst3"
    echo "Installed VST3 -> $VST3_DEST/Vocal Rider VST.vst3"
else
    echo "VST3 build not found at $VST3_SRC (build it first)"
    exit 1
fi
