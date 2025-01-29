# bash-zygote

Fast bash startup for compulsive terminal tab users.

## Introduction

`bash-zygote` applies [Android
zygote](https://stackoverflow.com/questions/9153166/understanding-android-zygote-and-dalvikvm/12703292#12703292)
concept to bash in order to optimize terminal tab startup. A zygote
server performs heavy initializations once (reading global configs,
bash completions, profile files, etc.), listens on a varlink socket
and spawns new pre-initialized shells that "steal" newly created
terminals.

Before:

![before](https://raw.githubusercontent.com/mbachry/bash-zygote/refs/heads/main/media/before.gif)

After:

![after](https://raw.githubusercontent.com/mbachry/bash-zygote/refs/heads/main/media/after.gif)

The change might seem small, but makes a huge difference if you open
tabs a lot, especially during heavy compilations.

## Installation

Clone the repo with `--recurse-submodules` or run `git submodule init`
and `git submodule update` after cloning.

Make sure all `bash` build requirements are installed. Also install
`meson` and `libvarlink` development package (`libvarlink-devel` in
Fedora case).

Run `make` and copy `build/bash-zygote` and `build/bash-zygote-client`
to bin location of your choice, eg. `~/.local/bin` or
`/usr/local/bin`.

Create the following systemd unit in `~/.config/systemd/user/bash-zygote.socket`

```
[Socket]
ListenStream=@bash-zygote-%U.socket

[Install]
WantedBy=sockets.target
```

and `~/.config/systemd/user/bash-zygote.service`:

```
[Unit]
Description=bash zygote

[Service]
ExecStart=%h/.local/bin/bash-zygote --login -i
Restart=on-failure
KillMode=process

[Install]
WantedBy=default.target
```

Start it with:

```
systemctl --user enable --now bash-zygote.socket
```

Configure your terminal with `bash-zygote-client` as default
shell. For kitty that's `shell ~/.local/bin/bash-zygote-client` in
`kitty.conf`. Restart terminal processes.

## Limitations

You have to restart `bash-zygote` daemon after making changes in
`~/.bash_profile` or other configs. Restarting the daemon doesn't kill
existing sessions.

The bash profile is evaluated only once at daemon startup. Any
per-session side effects like `neofetch` banners won't occur.

Kitty shell integration won't work unless you rename
`bash-zygote-client` to just `bash` or [set up the integration
manually](https://sw.kovidgoyal.net/kitty/shell-integration/#manual-shell-integration).
