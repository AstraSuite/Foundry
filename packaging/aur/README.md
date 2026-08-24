# AUR packages

Three packages cover the same application:

| Package | Source |
| --- | --- |
| `astra-foundry` | builds from the release tarball |
| `astra-foundry-bin` | the precompiled binary from the GitHub release |
| `astra-foundry-git` | builds from `main` |

All three provide `astra-foundry` and conflict with each other and with the old `astramarket*`
packages, which they replace.

## Releasing a new version

For `astra-foundry` and `astra-foundry-bin`, bump `pkgver`, reset `pkgrel` to 1 and refresh the
checksums — the release workflow publishes a `.sha256` next to every asset, or:

```bash
updpkgsums
```

`astra-foundry-git` needs no version bump, its `pkgver()` derives the version from `git describe`.

Then regenerate the metadata and push to the AUR:

```bash
makepkg --printsrcinfo > .SRCINFO
git commit -am "astra-foundry 1.2.0"
git push
```

## Checking a package before pushing

```bash
makepkg -f            # builds the package in place
namcap PKGBUILD *.pkg.tar.zst
```
