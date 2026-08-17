#!/usr/bin/env bash
set -e
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root / sudo to install to /usr"
    exit 1
fi
PREFIX="${PREFIX:-/usr}"
mkdir -p "$PREFIX/bin" "$PREFIX/share/applications" "$PREFIX/share/icons/hicolor/scalable/apps"
install -m755 bin/astra "$PREFIX/bin/astra"
install -m644 share/applications/astra.desktop "$PREFIX/share/applications/astra.desktop"
install -m644 share/icons/hicolor/scalable/apps/AstraMarket.svg "$PREFIX/share/icons/hicolor/scalable/apps/AstraMarket.svg"
echo "Astra Market installed successfully to $PREFIX"
