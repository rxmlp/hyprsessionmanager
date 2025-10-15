# HyprSessionManager

HyprSessionManager is meant to be kinda like the xfce session manager but for Hyprland. It's far from good code. Some applications don't have same name in hyprctl initialClass and the .desktop file and those won't be added in the session cache file. So that may be a big limitation for some. 
One solution is just having those .desktop files hidden by cloning to ~/.local/share/applications and adding `NoDisplay=true`. Then just make a copt with the same name as the initialClass output.

---

## Features

- Save the current session (open apps) with a timestampe in .cache/hyprsession
- Restore saved sessions from a clean, user-friendly Qt6 GUI.
- Remove unwanted cached sessions easily.
- Restore the latest session with one click.
- Command-line flags for quick session management without the GUI:
  - `--new-cache` to save the current session.
  - `--restore-latest` asks to restore the latest session. (Useful on eg boot/login)

---

## Installation
Option 1
yay -S hyprsessionmanager-git

Option 2
You might have to run this to make the bin dir
- `mkdir ~/.local/bin`

Run this to copy the file to user bin
- `cp hyprsessionmanager ~/.local/bin/hyprsessionmanager`

Also add this to make your shell find the bin
- `export PATH="$HOME/.local/bin:$PATH"`

### Build

```
mkdir build && cd build
cmake ..
make
```

### Dependencies

- Qt6 (Qt6Widgets)
- hyprctl (Hyprland client tool)
- jq (JSON processor)
- bash shell environment