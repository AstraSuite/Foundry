# Contributing to Foundry

## Building

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/foundry --gui
```

Required at build time: CMake >= 3.19, a C++20 compiler and Qt 6.5 or newer with the Core, Gui,
Widgets, Quick, QuickControls2, ShaderTools, Concurrent, Network, DBus and Svg modules.

## Tests

```bash
cmake -B build -G Ninja -DASTRA_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

The tests cover the backend output parsers and the process helper. Anything that parses the output of
`pacman`, `flatpak` or an AUR helper should come with a fixture in `tests/`; those parsers are static
functions so they can be tested without running the tools.

Every push and pull request builds on Arch Linux and runs the test suite (`.github/workflows/build.yml`).

## Code style

- `.clang-format` describes the C++ style: four spaces, attached braces, pointers bound to the type.
  Format the lines you touch, not whole files.
- `.editorconfig` covers the rest (LF, UTF-8, trailing whitespace).
- QML follows the structure of the existing pages: tokens from `Foundry.Config` for sizing, spacing
  and typography, colours from `qs.services` (`Colours.palette`), and the shared components in
  `qml/qs/components` instead of raw `Rectangle`/`Text` items.
- User visible strings go through `qsTr()` in QML and `tr()` in C++, and end up in `translations/`.
  After adding or changing strings, run `cmake --build build --target update_translations` and translate
  the new entries in `translations/foundry_de.ts` (or leave them empty, English is the fallback).

## Backends

A new package source is either a `IPackagePlugin` subclass in `src/marketplace/plugins/` registered in
`PackageManager::initPlugins()`, or a scriptable plugin described by a `plugin.json` manifest — see
[PLUGINS.md](PLUGINS.md). Run external commands through `astra::runProcess()` /
`astra::runProcessStreaming()` so they get a fixed locale, a timeout and line based output.

## Commits and pull requests

- Conventional commit subjects (`fix(cli): …`, `feat(details): …`, `chore: …`), imperative mood.
- Describe what was broken and how it was verified. Numbers, command output or screenshots help.
- One topic per pull request.
