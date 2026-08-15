# AstraMarket

AstraMarket is a unified package manager providing both a graphical interface and a command-line interface for managing Flatpak, Pacman, AUR, AppImage, and custom package sources.

## Requirements

- Qt 6 (Core, Gui, Quick, QuickControls2, ShaderTools, Concurrent, Network, DBus, Svg)
- CMake (>= 3.19)
- C++20 compiler
- Supported package managers: Flatpak, Pacman, paru/yay (optional for AUR)

## Building

```bash
mkdir -p build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

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

For creating and installing custom package sources or scrapers, see `PLUGINS.md`.
