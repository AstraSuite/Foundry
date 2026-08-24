# Plugin System Documentation

Foundry supports external plugins to add support for additional package managers, software repositories, or asset providers (such as Wallhaven or custom script scrapers).

## Plugin Directory Structure

Plugins are loaded automatically on startup from:
- User level: `~/.config/astra/plugins/<plugin-id>/`
- System level: `/usr/share/astra/plugins/<plugin-id>/`

Each plugin directory must contain a `plugin.json` manifest file and any associated scripts or binaries.

```
~/.config/astra/plugins/my-source/
├── plugin.json
├── search.sh
└── install.sh
```

## Manifest Specification (`plugin.json`)

```json
{
  "id": "my-source",
  "name": "My Custom Source",
  "description": "Custom source provider for Foundry",
  "icon": "extension",
  "requiredBinary": "curl",
  "commands": {
    "search": "./search.sh \"${QUERY}\"",
    "install": "./install.sh \"${ID}\" \"${SCOPE}\"",
    "uninstall": "./uninstall.sh \"${ID}\"",
    "details": "./details.sh \"${ID}\"",
    "list": "./list.sh",
    "updates": "./updates.sh",
    "launch": "./launch.sh \"${ID}\""
  }
}
```

### Manifest Fields

- `id` (string, required): Unique identifier for the plugin.
- `name` (string, required): Human-readable display name.
- `description` (string, optional): Short summary of the provider.
- `icon` (string, optional): Material icon identifier (e.g. `extension`, `download`, `wallpaper`).
- `requiredBinary` (string, optional): Executable that must exist for the plugin to be active. A plain name is looked up in `PATH`, a value containing a slash is treated as an absolute path.
- `commands` (object, required): Shell commands executed for respective actions. Variable placeholders `${QUERY}`, `${ID}`, `${SCOPE}` are substituted at runtime.

### Placeholder Substitution

Commands run through `/bin/sh -c`. Placeholders are **not** pasted into the command string: every
`${NAME}` is rewritten to a positional parameter and the value is handed to the shell as an argument, so
user input such as a search query is never interpreted as shell syntax.

```json
"search": "./search.sh \"${QUERY}\""
```

runs as `/bin/sh -c './search.sh "${1}"' astra-plugin <query>`, and `./search.sh` receives the query
verbatim in `$1`. Quoting the placeholder in the manifest is still recommended so values containing
whitespace stay a single argument.

Available placeholders are `${QUERY}` (search), `${ID}` (install, uninstall, details, launch) and the
install options, currently `${SCOPE}`. Unknown placeholders expand to an empty argument.

### Exit Codes and Timeouts

`install` and `uninstall` are reported as successful only when the command exits with status `0`;
everything they print on stdout is streamed into the operation log while they run. Metadata commands
(`search`, `list`, `updates`, `details`) are given 20 seconds before they are terminated, install and
uninstall are allowed to run for up to an hour, and `launch` is started detached without waiting.

## Expected JSON Outputs

### `search` and `list`

Commands should output a JSON array of package items:

```json
[
  {
    "id": "package-identifier",
    "name": "Display Name",
    "version": "1.0.0",
    "summary": "Short description of the item",
    "icon": "icon-name-or-path"
  }
]
```

### `details`

The details command should output a JSON object:

```json
{
  "id": "package-identifier",
  "name": "Display Name",
  "version": "1.0.0",
  "developer": "Author / Team",
  "license": "GPL-3.0",
  "homepage": "https://example.com",
  "summary": "Short description",
  "description": "Full description of the application or asset."
}
```

## Installing a Plugin

1. Create a directory inside `~/.config/astra/plugins/` with your plugin name:
   ```bash
   mkdir -p ~/.config/astra/plugins/my-source
   ```

2. Place `plugin.json` and any helper scripts inside this directory.

3. Make scripts executable:
   ```bash
   chmod +x ~/.config/astra/plugins/my-source/*.sh
   ```

4. Verify your plugin is loaded:
   ```bash
   astra sources
   ```
