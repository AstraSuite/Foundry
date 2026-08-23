_astra() {
    local cur prev commands options sources scopes
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    commands="search install remove uninstall list update upgrade info sources plugins gui tray help version"
    options="--source --scope --json --no-color --help --version"
    sources="Flatpak Pacman AUR AppImage"
    scopes="user system"

    case "$prev" in
        -s|--source)
            COMPREPLY=($(compgen -W "$sources" -- "$cur"))
            return
            ;;
        --scope)
            COMPREPLY=($(compgen -W "$scopes" -- "$cur"))
            return
            ;;
    esac

    if [[ $cur == -* ]]; then
        COMPREPLY=($(compgen -W "$options" -- "$cur"))
        return
    fi

    if [[ $COMP_CWORD -eq 1 ]]; then
        COMPREPLY=($(compgen -W "$commands" -- "$cur"))
        return
    fi

    COMPREPLY=($(compgen -f -- "$cur"))
}

complete -F _astra astra
