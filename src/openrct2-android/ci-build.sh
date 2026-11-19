#!/usr/bin/env bash
# Builds the OpenRCT2 Pocket APK. Run from the repository root, in OpenRCT2's Android build
# image (ghcr.io/openrct2/openrct2-build:26-android) or anywhere with the SDK, NDK and CMake
# versions named in app/build.gradle. See BUILDING-POCKET.md.
#
#   src/openrct2-android/ci-build.sh <output.apk> [version name]
set -euo pipefail

OUTPUT="${1:?usage: ci-build.sh <output.apk> [version name]}"
VERSION="${2:-0.0.0-dev}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ANDROID_DIR="$ROOT/src/openrct2-android"

# gxc builds the engine's own graphics (g2.dat and friends) for the APK's assets.
GXC_VERSION=v1.4.0
GXC_SHA256=e7246e50c7ed8b81d959341df6e724432f62eb3976fd2259c4a9ae1bf94227db
if ! command -v gxc >/dev/null 2>&1; then
    TOOLS="$(mktemp -d)"
    curl -fsSL -o "$TOOLS/libsawyer-tools.tar.gz" \
        "https://github.com/OpenRCT2/libsawyer/releases/download/$GXC_VERSION/libsawyer-tools-linux-x64.tar.gz"
    echo "$GXC_SHA256  $TOOLS/libsawyer-tools.tar.gz" | sha256sum -c -
    tar -xzf "$TOOLS/libsawyer-tools.tar.gz" -C "$TOOLS"
    export PATH="$TOOLS:$PATH"
fi

# The commit count only grows on main, so each build can be installed over the one before.
VERSION_CODE="$(git -C "$ROOT" rev-list --count HEAD 2>/dev/null || echo 1)"

cd "$ANDROID_DIR"
./gradlew --no-daemon assembleRelease -PpocketVersion="$VERSION" -PpocketVersionCode="$VERSION_CODE"
cp app/build/outputs/apk/release/app-release.apk "$ROOT/$OUTPUT"
echo "Built $OUTPUT ($VERSION, version code $VERSION_CODE)"
