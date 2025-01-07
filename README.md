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

![before](https://raw.githubusercontent.com/mbachry/bash-zygote/refs/heads/media/media/before.gif)

After:

![after](https://raw.githubusercontent.com/mbachry/bash-zygote/refs/heads/media/media/after.gif)

The change might seem small, but makes a huge difference if you open
tabs a lot, especially during heavy compilations.

## Installation

Make sure all `bash` build requirements are installed. Also install
`meson` and `libvarlink` development package (`libvarlink-devel` in
Fedora case).

Run `make` and copy `build/bash-zygote` and `build/bash-zygote-client`
to bin location of your choice, eg. `~/.local/bin` or
`/usr/local/bin`.

Create the following systemd unit in `~/.config/systemd/user/bash-zygote.service`:

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
systemctl --user enable --now bash-zygote.service
```

Configure your terminal with `bash-zygote-client` as default
shell. For kitty that's `shell ~/.local/bin/bash-zygote-client` in
`kitty.conf`. Restart terminal processes.

## Limitations

You have to restart `bash-zygote` daemon after making changes in
`~/.bash_profile` or other configs. Restarting the daemon doesn't kill
existing sessions.
