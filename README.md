<p align="center">
  <img src="assets/icons/astra-foundry.svg" width="140" alt="Foundry Logo">
</p>

<h1 align="center">Foundry</h1>

Foundry is a unified package manager providing both a graphical interface and a command-line interface for managing Flatpak, Pacman, AUR, AppImage, and custom package sources.

## Requirements

Build:

- Qt 6.5+ (Core, Gui, Widgets, Quick, QuickControls2, ShaderTools, Concurrent, Network, DBus, Svg)
- CMake (>= 3.19)
- C++20 compiler

Runtime, all optional — a source that is not installed is simply inactive:

- `flatpak` for the Flatpak source
- `pacman` for the Pacman source, plus `pacman-contrib` for `checkupdates` (without it no Pacman
  updates are reported) and a PolicyKit authentication agent for installing and removing packages
- `paru` or `yay` for the AUR source
- `update-desktop-database` (`desktop-file-utils`) for AppImage desktop integration

## Installation

### Arch Linux / AUR

Foundry is available on the [AUR](https://aur.archlinux.org):

```bash
# Release (builds from source)
paru -S astra-foundry
# or yay -S astra-foundry

# Precompiled binary (fast install)
paru -S astra-foundry-bin
# or yay -S astra-foundry-bin

# Latest git development
paru -S astra-foundry-git
# or yay -S astra-foundry-git
```

### Manual Installation (From Source)

Build and install Foundry system-wide:

```bash
mkdir -p build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

This installs the `astra` binary to `/usr/bin`, the desktop launcher to `/usr/share/applications/astra.desktop`, and the application icon to `/usr/share/icons/hicolor/scalable/apps/astra-foundry.svg`.


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
astra search <query> --json
```

When no `--source` is given, the source is resolved from the package id: an ambiguous id lists the
sources that provide it, so `astra install yay --source AUR` picks the AUR package rather than the
repository one.

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

List available updates:

```bash
astra update
astra update --json
```

Apply updates, either everything or a single package:

```bash
astra upgrade
astra upgrade <package-id>
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

Output is coloured only when it goes to a terminal; `--no-color` and `NO_COLOR` disable it, and
`--json` prints machine readable output for `search`, `list`, `update`, `info` and `sources`.

Completions for bash, zsh and fish are installed alongside the binary.

## Privileges

Pacman operations are executed through `pkexec`, so they are authorised by PolicyKit. The shipped action
`com.astra-foundry.pacman.update` uses the standard defaults: administrator authentication is required, and
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

## Translations

The interface is translatable and ships with German. Translation files live in `translations/` and are
compiled into the binary when the Qt Linguist tools are available (`qt6-tools` on Arch); without them the
build falls back to English. The language is picked from the system locale at startup.

Refresh the strings after changing the UI:

```bash
cmake --build build --target update_translations
```

To add a language, copy `translations/foundry_de.ts` to `translations/foundry_<code>.ts`, add it to the
`qt_add_translations()` call in `CMakeLists.txt` and translate it with Qt Linguist.

## Contributing

Build instructions, the test setup and the code style are described in [CONTRIBUTING](CONTRIBUTING.md).

## Acknowledgements

- UI design language, tokens, and components adapted from [Caelestia Shell](https://github.com/caelestia-dots/shell).
- Material Design icons and shape specifications by Google.

## License

This project is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. See the [LICENSE](LICENSE) file for details.
