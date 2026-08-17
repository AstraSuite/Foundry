# AstraMarket

AstraMarket is a unified package manager providing both a graphical interface and a command-line interface for managing Flatpak, Pacman, AUR, AppImage, and custom package sources.

## Requirements

- Qt 6 (Core, Gui, Quick, QuickControls2, ShaderTools, Concurrent, Network, DBus, Svg)
- CMake (>= 3.19)
- C++20 compiler
- Supported package managers: Flatpak, Pacman, paru/yay (optional for AUR)

## Installation

### Arch Linux / AUR

AstraMarket is available on the [AUR](https://aur.archlinux.org):

```bash
# Release (builds from source)
paru -S astramarket
# or yay -S astramarket

# Precompiled binary (fast install)
paru -S astramarket-bin
# or yay -S astramarket-bin

# Latest git development
paru -S astramarket-git
# or yay -S astramarket-git
```

### Manual Installation (From Source)

Build and install AstraMarket system-wide:

```bash
mkdir -p build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

This installs the `astra` binary to `/usr/bin`, the desktop launcher to `/usr/share/applications/astra.desktop`, and the application icon to `/usr/share/icons/hicolor/scalable/apps/AstraMarket.svg`.


## Running the GUI


Launch the graphical marketplace interface:

```bash
./build/astra --gui
```

or:

```bash
./build/astra -g
```

## Using the CLI

Search across enabled package sources:

```bash
astra search <query>
astra search <query> --source <Flatpak|Pacman|AUR|AppImage>
```

Install a package:

```bash
astra install <package-id>
astra install <package-id> --source <source>
astra install <package-id> --source Flatpak --scope <user|system>
```

Uninstall a package:

```bash
astra remove <package-id>
astra remove <package-id> --source <source>
```

List installed packages:

```bash
astra list
```

Check for updates:

```bash
astra update
```

View package details:

```bash
astra info <package-id>
astra info <package-id> --source <source>
```

List active package sources and plugins:

```bash
astra sources
```

Show help and all available commands:

```bash
astra --help
```

## Plugins

For creating and installing custom package sources or scrapers, see [PLUGINS](PLUGINS.md).

## Acknowledgements

- UI design language, tokens, and components adapted from [Caelestia Shell](https://github.com/caelestia-dots/shell).
- Material Design icons and shape specifications by Google.

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. See the [LICENSE](LICENSE) file for details.
