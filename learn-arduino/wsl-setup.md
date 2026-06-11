# Arduino on Windows WSL2 — Setup Guide

How to make an Arduino board (connected to Windows) usable from `arduino-cli`
running inside **WSL2 (Ubuntu)** — compile, upload, and serial monitor, all from Linux.

This is the WSL counterpart to [`setup.md`](./setup.md) (which targets macOS).
The only real difference is **getting the USB serial port into WSL** — everything
after that is identical to the Mac workflow, except the port name.

---

## Why this is needed

WSL2 runs inside a lightweight virtual machine. Unlike WSL1, it does **not** share
the host's USB devices. So even though Windows sees the Arduino on a COM port,
Linux sees nothing, and `arduino-cli board list` reports:

```text
No boards found.
```

The fix is **USB/IP forwarding** via [`usbipd-win`](https://github.com/dorssel/usbipd-win):
a Windows service that exports a USB device over USB/IP, which the WSL2 kernel
(which ships with USB/IP + serial drivers) then attaches as a native device.

```
[ Arduino ] --USB--> [ Windows + usbipd-win ] --USB/IP--> [ WSL2 kernel ] --> /dev/ttyUSB0
```

---

## Environment this was verified on

| Item              | Value                                            |
| ----------------- | ------------------------------------------------ |
| OS                | Windows 11 24H2 (build 26100)                    |
| WSL               | 2.5.9.0 — distro Ubuntu 24.04.3 LTS, version 2   |
| WSL kernel        | `6.6.87.2-microsoft-standard-WSL2`               |
| Board             | CH340-based Arduino Uno clone                    |
| USB ID (VID:PID)  | `1a86:7523` (the WCH CH340 USB-serial chip)      |
| Windows port      | `COM10`                                          |
| usbipd bus ID     | `3-2`                                             |
| WSL device        | `/dev/ttyUSB0`                                    |
| `arduino-cli`     | 1.5.1                                            |
| Core              | `arduino:avr` 1.8.8                              |
| FQBN              | `arduino:avr:uno`                                |

> **Your bus ID may differ.** It depends on which physical USB port the board is
> in. Always confirm it with `usbipd list` (below). The `1a86:7523` VID:PID is
> what identifies a CH340 board.

---

## Before you start — requirements

On **Windows**:

- **Windows 11**, or Windows 10 (22H2) with WSL kept up to date. USB/IP forwarding to
  WSL needs a recent WSL.
- **WSL2, not WSL1** — usbipd cannot forward USB to a WSL1 distro. Check from PowerShell:
  ```powershell
  wsl -l -v        # the distro's VERSION column must show 2
  wsl --version    # kernel should be 5.10.60 or newer (this guide used 6.6.87)
  ```
- **Update WSL** so its kernel includes the USB/IP + serial drivers used below
  (`vhci_hcd`, `ch341`, `cdc_acm`):
  ```powershell
  wsl --update
  ```
- **Administrator access** on Windows (needed once, for `usbipd bind`).
- The Arduino **plugged into a USB port** on the PC.

On **WSL (Ubuntu)**: a normal shell with `sudo` access. The next section installs
`arduino-cli` itself.

---

## Install arduino-cli (inside WSL)

If `arduino-cli version` already prints a version, skip to Part 1. Otherwise install the
official Linux binary from the [releases page](https://github.com/arduino/arduino-cli/releases)
(this is how the CLI in this repo was installed):

```bash
# 1. Download the release tarball (this guide uses v1.5.1, x86-64)
cd /tmp
wget https://github.com/arduino/arduino-cli/releases/download/v1.5.1/arduino-cli_1.5.1_Linux_64bit.tar.gz

# 2. Extract the arduino-cli binary
tar -xzf arduino-cli_1.5.1_Linux_64bit.tar.gz

# 3. Put it somewhere on your PATH
mkdir -p ~/opt/arduino
mv arduino-cli ~/opt/arduino/

# 4. Add that directory to PATH, then reopen the shell (use ~/.bashrc if you use bash)
echo 'export PATH="$HOME/opt/arduino:$PATH"' >> ~/.zshrc
```

> On ARM64 WSL, download the `_Linux_ARM64.tar.gz` asset instead of `_Linux_64bit.tar.gz`.
> Bump the version number in the URL to install a newer release.
> (Alternative one-line installer: `curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh`.)

Confirm it runs, then install the AVR core (the compiler/toolchain for Uno / Nano / Mega):

```bash
arduino-cli version
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli core list                  # should list arduino:avr
```

---

## Part 1 — Windows side (one-time install + bind)

Run these in **PowerShell as Administrator**.

### 1. Install usbipd-win

```powershell
winget install --exact --id dorssel.usbipd-win --accept-source-agreements --accept-package-agreements
```

Then **close and reopen PowerShell** so `usbipd` is on the PATH.
(If `usbipd` is still "not recognized", call it by full path:
`& 'C:\Program Files\usbipd-win\usbipd.exe' ...`)

> **No `winget`?** (older Windows 10) Download the latest `.msi` from the
> [usbipd-win releases page](https://github.com/dorssel/usbipd-win/releases) and run it.

### 2. Find the board's bus ID

```powershell
usbipd list
```

```text
Connected:
BUSID  VID:PID    DEVICE                              STATE
3-2    1a86:7523  USB-SERIAL CH340 (COM10)            Not shared
...
```

Note the **BUSID** of the line that matches your board
(`USB-SERIAL CH340` / VID:PID `1a86:7523`). Here it is `3-2`.

### 3. Bind the device (one-time, persistent — needs admin)

```powershell
usbipd bind --busid 3-2
```

After this, `usbipd list` shows the device as **`Shared`**.
Binding survives reboots — you only do this once per board/port.

---

## Part 2 — Attach the device to WSL

`attach` does **not** need admin. **Always look up the current bus ID first** —
don't assume it's `3-2`. The bus ID reflects which physical USB port the board is
in, so it can change if you move the board to another port or use a hub.

**Step 1 — list devices and grab the bus ID:**

```powershell
usbipd list
```

Find the row for your board (`USB-SERIAL CH340` / VID:PID `1a86:7523`) and read its
**BUSID** from the first column (it was `3-2` on this machine).

**Step 2 — attach that bus ID to WSL:**

```powershell
usbipd attach --wsl --busid <BUSID>     # e.g. usbipd attach --wsl --busid 3-2
```

> ⚠️ **`attach` is not permanent.** You must re-run it every time the device is
> disconnected — i.e. after unplugging/replugging the board, or after
> `wsl --shutdown`, or after a Windows reboot. (See [Reconnecting](#reconnecting).)

`usbipd list` should now show the device as **`Attached`**.

> **One-liner (skip the manual copy):** this finds the CH340's bus ID by its
> VID:PID and attaches it in one go —
> ```powershell
> $busid = (usbipd list | Select-String '1a86:7523').ToString().Trim().Split()[0]
> usbipd attach --wsl --busid $busid
> ```
> Or let usbipd match the device itself, no bus ID needed:
> `usbipd attach --wsl --hardware-id 1a86:7523`

---

## Part 3 — WSL side (one-time port permission)

After attaching, the device appears as `/dev/ttyUSB0`, owned by `root:dialout`:

```bash
ls -la /dev/ttyUSB0
# crw-rw---- 1 root dialout 188, 0 ... /dev/ttyUSB0
```

Your user must be in the **`dialout`** group to read/write the port, otherwise
uploads fail with `Permission denied`. Add yourself once:

```bash
sudo usermod -aG dialout $USER
newgrp dialout
```

- `usermod` adds you to the group **permanently** (every future shell gets it).
- `newgrp dialout` activates the group in your **current** shell immediately,
  so you don't have to run `wsl --shutdown` — which would detach the USB device
  and force you to re-attach. (You could restart WSL instead, but `newgrp` is
  the no-friction path.)

---

## Verify

```bash
arduino-cli board list
```

```text
Port         Protocol Type              Board Name FQBN Core
/dev/ttyUSB0 serial   Serial Port (USB) Unknown
```

> **"Unknown" board name is normal** for CH340 clones — the CH340 chip doesn't
> advertise Arduino's USB ID, so the CLI can't auto-detect the FQBN. You simply
> pass `--fqbn arduino:avr:uno` yourself (as the commands below do).

---

## Daily workflow

The port on WSL is **`/dev/ttyUSB0`** (on macOS it was `/dev/cu.usbserial-120`).

```bash
cd learn-arduino/01_blink_led

# Compile
arduino-cli compile --fqbn arduino:avr:uno .

# Upload
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno .

# Serial monitor (Ctrl+C to exit)
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```

### Suggested shell aliases (WSL versions)

Add to `~/.zshrc` or `~/.bashrc`. These match the Neovim/clangd setup — `ac` carries
`--build-path ./build`, which drops a `build/compile_commands.json` in the sketch that
**clangd reads** for autocomplete, hover (`K`), go-to-definition (`gd`), and diagnostics:

```bash
alias ac='arduino-cli compile --fqbn arduino:avr:uno --build-path ./build .'
alias au='arduino-cli upload  -p /dev/ttyUSB0 --fqbn arduino:avr:uno .'
alias am='arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600'
```

> A plain `ac` already generates the compilation database — no separate
> `--only-compilation-database` run is needed.
> The `build/` folders are git-ignored (see `.gitignore`).
> clangd does **not** auto-inject `#include <Arduino.h>` like the Arduino IDE, so add
> that line at the top of each sketch or symbols like `digitalWrite`/`OUTPUT` show as
> undeclared.

Typical loop:

```bash
ac && au && am     # compile, upload, monitor
```

---

## Reconnecting

The **bind is permanent; the attach is not.** Whenever the board "disappears"
from WSL (`No boards found`, or `/dev/ttyUSB0` missing), re-attach from PowerShell.
Look up the bus ID first, then attach it:

```powershell
usbipd list                              # read the BUSID for 1a86:7523 (CH340)
usbipd attach --wsl --busid <BUSID>      # e.g. 3-2
```

To make it reconnect automatically as long as the PowerShell window stays open
(no bus-ID lookup needed — it matches by hardware ID):

```powershell
usbipd attach --wsl --hardware-id 1a86:7523 --auto-attach
```

To stop forwarding (give the port back to Windows):

```powershell
usbipd detach --busid <BUSID>
```

---

## Troubleshooting

| Symptom | Cause | Fix |
| --- | --- | --- |
| `No boards found` / `/dev/ttyUSB0` missing | Device not attached to WSL | `usbipd list` to get the BUSID, then `usbipd attach --wsl --busid <BUSID>` (confirm it's bound/`Shared` first) |
| `cannot open port /dev/ttyUSB0: Permission denied` | Not in `dialout` group, or group not active in this shell | `sudo usermod -aG dialout $USER` then `newgrp dialout` (or open a new terminal) |
| Board gone after replug / reboot / `wsl --shutdown` | `attach` is per-connection | `usbipd list` → `usbipd attach --wsl --busid <BUSID>` |
| Wrong device attached / BUSID changed | Bus ID tracks the physical port, not the board | Re-check with `usbipd list`, or attach by `--hardware-id 1a86:7523` instead |
| `usbipd` not recognized in PowerShell | PATH not refreshed after install | Reopen PowerShell, or use `& 'C:\Program Files\usbipd-win\usbipd.exe' ...` |
| Board name shows `Unknown` | Normal for CH340 clones | Ignore — always pass `--fqbn arduino:avr:uno` |
| `bind` says access denied | Not running as admin | Use an **Administrator** PowerShell for `bind` |
| Upload fails: port busy | Serial monitor still open | Close the monitor (`Ctrl+C`), then upload |
| attach fails citing "mirrored" networking | Old usbipd-win | Upgrade to usbipd-win ≥ 4.0 (this guide used 5.3.0, which handles mirrored mode) |

### Genuine vs clone boards — port name

- **CH340 / CH341 clone** (USB ID `1a86:...`) → `/dev/ttyUSB0` (driver `ch341`)
- **Genuine Uno R3 / Mega** (ATmega16U2 USB) → `/dev/ttyACM0` (driver `cdc_acm`)
- **FTDI-based** (`0403:...`) → `/dev/ttyUSB0` (driver `ftdi_sio`)

If unsure which device node appeared:

```bash
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

---

## Quick reference (this machine)

```powershell
# Windows (PowerShell) — first time
winget install --exact --id dorssel.usbipd-win
usbipd list                          # find BUSID for 1a86:7523 (CH340)
usbipd bind --busid <BUSID>          # admin, one-time (e.g. 3-2)

# Windows — every time the board is (re)connected
usbipd list                          # re-check the BUSID (may change per port)
usbipd attach --wsl --busid <BUSID>
```

```bash
# WSL — first time
sudo usermod -aG dialout $USER
newgrp dialout

# WSL — daily
arduino-cli board list
arduino-cli compile --fqbn arduino:avr:uno .
arduino-cli upload  -p /dev/ttyUSB0 --fqbn arduino:avr:uno .
arduino-cli monitor -p /dev/ttyUSB0 -c baudrate=9600
```
