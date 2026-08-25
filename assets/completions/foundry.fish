set -l astra_commands search install remove uninstall list update upgrade info sources plugins gui tray help version

complete -c foundry -f
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a search -d 'Search for packages across all or specific sources'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a install -d 'Install a package or a local .AppImage file'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a remove -d 'Uninstall a package'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a list -d 'List installed packages'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a update -d 'List available updates'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a upgrade -d 'Apply updates, all of them or a single package'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a info -d 'Display metadata and details for a package'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a sources -d 'List registered package plugins and sources'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a gui -d 'Launch the graphical interface'
complete -c foundry -n "not __fish_seen_subcommand_from $astra_commands" -a tray -d 'Launch minimized in system tray'

complete -c foundry -s s -l source -x -a 'Flatpak Pacman AUR AppImage' -d 'Package source'
complete -c foundry -l scope -x -a 'user system' -d 'Installation scope'
complete -c foundry -l json -d 'Print machine readable output'
complete -c foundry -l no-color -d 'Disable coloured output'
complete -c foundry -s h -l help -d 'Show help'
complete -c foundry -s v -l version -d 'Show version'
complete -c foundry -n "__fish_seen_subcommand_from install" -F
