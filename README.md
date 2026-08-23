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

## Privileges

Pacman operations are executed through `pkexec`, so they are authorised by PolicyKit. The shipped action
`com.astramarket.pacman.update` uses the standard defaults: administrator authentication is required, and
the authorisation is kept for the rest of the session (`auth_admin_keep`) so a full update does not ask
repeatedly.

A PolicyKit authentication agent has to be running for these operations; without one `pkexec` cannot ask
for credentials. Flatpak (`--user`) and AppImage operations do not require any elevated privileges.

Versions up to 1.1.0 also installed `/usr/share/polkit-1/rules.d/10-astramarket-pacman.rules`, which
allowed every member of the `wheel` group to run pacman through `pkexec` without authenticating. That file
is no longer shipped and should be removed from existing installations:

```bash
sudo rm -f /usr/share/polkit-1/rules.d/10-astramarket-pacman.rules
```

## Plugins

For creating and installing custom package sources or scrapers, see [PLUGINS](PLUGINS.md).

## Acknowledgements

- UI design language, tokens, and components adapted from [Caelestia Shell](https://github.com/caelestia-dots/shell).
- Material Design icons and shape specifications by Google.

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. See the [LICENSE](LICENSE) file for details.
