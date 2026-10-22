# UNO R4 WiFi bridge firmware v0.6.0 — build and flash procedure

This procedure builds the stock ESP32-S3 bridge firmware v0.6.0 from
this repository and flashes it to an Arduino UNO R4 WiFi. It contains
only what you do and what you should see. The reasons behind each
step, the sources, and the background are in
[build-original-reference.md](build-original-reference.md) — you never
need it to follow this procedure.

> [!CAUTION]
> Step 5 writes to the board. Do not start it until steps P–4 have
> passed every ✅ check. Never flash with the esptool command in the
> repository README — its offsets are wrong for this build.

## How to use this procedure

- Do the actions in order. Each one has a number (`2.1`, `2.2`, …).
- Each step starts with a **status line**: whether the board is
  plugged in, whether the jumper is on, and where to run commands.
- Each action shows what to run, then:
  - ✅ — what you must see. If you see it, go to the next action.
  - ❌ — what to do if you see something else. Usually "→ **Fix 2.1**
    below": the fix is in its own box directly underneath, inside the
    same action. It ends with **Then:**, which tells you what to do
    next. If you got a ✅, skip the fix boxes.
  - **Expected noise** — warnings you will see that are normal.
- If a result matches neither the ✅ nor any ❌: **stop.** Do not
  improvise, especially in step 5.
- `sudo` asks for your login password. Typing it shows nothing on
  screen; that is normal.

> ## Step P — Prerequisites (one time)
>
> **Board:** UNPLUGGED · **Jumper:** none · **Run from:** anywhere
>
> > [!IMPORTANT]
> > Keep the board unplugged until step 5.0 tells you to plug it in.
> > Nothing before step 5 needs it.
>
> > ### P.1 Gather the hardware
> >
> > On the desk: the UNO R4 WiFi, a USB-C **data** cable (charge-only
> > cables power the board but connect nothing), and one
> > **female-to-female jumper wire**.
> >
> > - ✅ All three are in front of you.
> > - ❌ No jumper wire → you can do steps P–4 without it, but get one
> >   before step 5: it is the recovery path.
>
> > ### P.2 Install the packages
> >
> > ```
> > sudo apt install git zip unzip curl wget usbutils python3 python-is-python3 python3-cryptography python3-serial
> > ```
> >
> > - ✅ It finishes back at the prompt with no line starting with `E:`.
> > - ❌ A line starting with `E:` → **Fix P.2** below.
> >
> > > #### Fix P.2 — `apt install` printed a line starting with `E:`
> > >
> > > - `Could not get lock` → another update is running (often the
> > >   automatic updater). Wait a few minutes.
> > > - `Unable to locate package` → run `sudo apt update`.
> > > - Anything else → stop and look up the exact `E:` message.
> > >
> > > **Then:** repeat P.2.
>
> > ### P.3 Check for arduino-cli
> >
> > ```
> > arduino-cli version
> > ```
> >
> > - ✅ Prints `arduino-cli  Version: 1.` followed by more numbers →
> >   skip P.3.1, go to P.4.
> > - ❌ `command not found` → do P.3.1.
> >
> > > #### P.3.1 Install arduino-cli (only if P.3 said `command not found`)
> > >
> > > ```
> > > curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/tmp/arduino-cli sh
> > > sudo install -m 755 /tmp/arduino-cli/arduino-cli /usr/local/bin/
> > > ```
> > >
> > > Then repeat P.3.
>
> > ### P.4 Check for espflash
> >
> > ```
> > espflash --version
> > ```
> >
> > - ✅ Prints `espflash 2.0.0` → skip P.4.1, go to P.5.
> > - ❌ `command not found` → do P.4.1.
> > - ❌ Any other version → do P.4.1 (this procedure uses 2.0.0, the
> >   version Arduino's own updater ships).
> >
> > > [!WARNING]
> > > Do not `pip install esptool`. Ubuntu refuses it, and this procedure
> > > uses the esptool that step 2 downloads with the platform.
> >
> > > #### P.4.1 Install espflash 2.0.0 (only if P.4 sent you here)
> > >
> > > ```
> > > curl -sL -o /tmp/espflash.zip https://github.com/esp-rs/espflash/releases/download/v2.0.0/espflash-x86_64-unknown-linux-gnu.zip
> > > unzip -o /tmp/espflash.zip -d /tmp/espflash
> > > sudo install -m 755 /tmp/espflash/espflash /usr/local/bin/
> > > ```
> > >
> > > Then repeat P.4.
>
> > ### P.5 Give yourself access to the board's serial port
> >
> > ```
> > sudo usermod -aG dialout $USER
> > ```
> >
> > ✅ It prints nothing.
>
> > ### P.6 Reboot
> >
> > > [!IMPORTANT]
> > > The group change in P.5 does not apply until you start a new
> > > session. Reboot now — it also applies any pending system updates.
> >
> > ```
> > sudo reboot
> > ```
>
> > ### P.7 Confirm the serial-port access
> >
> > After the reboot, open a new terminal:
> >
> > ```
> > groups
> > ```
> >
> > - ✅ The list includes `dialout`.
> > - ❌ It does not → **Fix P.7** below.
> >
> > > #### Fix P.7 — `groups` does not list `dialout`
> > >
> > > Repeat P.5 and P.6 (the `usermod` command and the reboot).
> > >
> > > **Then:** repeat P.7.
>
> > ### P.8 Go to the repository and the stock branch
> >
> > The stock firmware is built from the `feature/docs` branch: Arduino's
> > code unchanged, plus these docs. This procedure keeps the repository
> > in `~/git/unor4-wifi-interrupt-bridge` (`~` is your home folder).
> >
> > ```
> > cd ~/git/unor4-wifi-interrupt-bridge
> > git switch feature/docs
> > git status
> > ```
> >
> > - ✅ `git status` prints `On branch feature/docs` → go to 1.1.
> > - ❌ `cd` says `No such file or directory` → **Fix P.8-A** below.
> > - ❌ `git switch` says `invalid reference: feature/docs` →
> >   **Fix P.8-B** below.
> >
> > **Every command from here on runs from this folder** unless an
> > action says otherwise.
> >
> > > #### Fix P.8-A — the repository folder does not exist
> > >
> > > Get the repository:
> > >
> > > ```
> > > git clone https://github.com/TonyMarkham/unor4-wifi-interrupt-bridge.git ~/git/unor4-wifi-interrupt-bridge
> > > ```
> > >
> > > **Then:** repeat P.8.
> >
> > > #### Fix P.8-B — the `feature/docs` branch does not exist
> > >
> > > Your copy of the repository is older than the branch. Fetch it:
> > >
> > > ```
> > > git fetch origin
> > > ```
> > >
> > > If P.8 still fails after this, **stop**: the branch is missing on
> > > GitHub too, and there is nothing to build.
> > >
> > > **Then:** repeat P.8.

> ## Step 1 — Get the source
>
> **Board:** unplugged · **Jumper:** none · **Run from:** repository root
>
> > ### 1.1 Fetch the submodules
> >
> > ```
> > git submodule update --init --depth 1 --no-single-branch
> > ```
> >
> > It takes about 2 minutes and can go quiet for long stretches. It has
> > not hung — let it finish.
> >
> > - ✅ It ends with seven lines of the form
> >   `Submodule path '…': checked out '…'`.
> > - ❌ A line starting with `fatal:` → **Fix 1.1** below.
> >
> > > #### Fix 1.1 — fetching the submodules failed
> > >
> > > Usually a network problem. Repeating 1.1 is safe — it only fetches
> > > what is still missing.
> > >
> > > **Then:** repeat 1.1.
>
> > ### 1.2 Check the submodules
> >
> > ```
> > git submodule status
> > ```
> >
> > - ✅ Exactly seven lines, and none of them starts with `-`.
> > - ❌ Any line starts with `-` → **Fix 1.2** below.
> >
> > > #### Fix 1.2 — a submodule is missing
> > >
> > > 1.1 did not complete. Repeat 1.1 — it only fetches what is still
> > > missing.
> > >
> > > **Then:** repeat 1.2.

> ## Step 2 — Get the toolchain
>
> **Board:** unplugged · **Jumper:** none · **Run from:** repository root
>
> Needs about 3 GB of free disk space.
>
> > ### 2.1 Download the toolchain
> >
> > Run all three lines; the last one brings you back to the repository
> > root:
> >
> > ```
> > cd hardware/esp32-patched/esp32/tools
> > python3 ./get.py
> > cd ~/git/unor4-wifi-interrupt-bridge
> > ```
> >
> > It prints a `Downloading …` and `Extracting …` line for each of ten
> > tools. It took 26 seconds when this procedure was tested; a slower
> > connection takes longer.
> >
> > - ✅ The last line `get.py` prints is `Platform Tools Installed`.
> > - ❌ It stops with an error → **Fix 2.1** below.
> >
> > > #### Fix 2.1 — `get.py` stopped with an error
> > >
> > > This is a network problem, not a build problem (proxy, firewall,
> > > dropped connection). `get.py` keeps partly downloaded archives in
> > > `hardware/esp32-patched/esp32/tools/dist/` and will not download
> > > them again, so delete the one it was working on when it failed
> > > (the last `Downloading …` name it printed):
> > >
> > > ```
> > > rm hardware/esp32-patched/esp32/tools/dist/<that-archive-name>
> > > ```
> > >
> > > **Then:** repeat 2.1.
>
> > ### 2.2 Check the compiler
> >
> > ```
> > hardware/esp32-patched/esp32/tools/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gcc --version | head -1
> > ```
> >
> > - ✅ Prints `xtensa-esp32s3-elf-gcc (crosstool-NG esp-2021r2-patch5) 8.4.0`.
> > - ❌ Prints nothing, or `No such file or directory` → **Fix 2.2**
> >   below.
> >
> > > #### Fix 2.2 — the compiler is missing
> > >
> > > `get.py` did not finish. Repeat 2.1; it skips archives it already
> > > has.
> > >
> > > **Then:** repeat 2.2.
>
> > ### 2.3 Check esptool
> >
> > ```
> > python3 hardware/esp32-patched/esp32/tools/esptool/esptool.py version
> > ```
> >
> > - ✅ The last two lines are `esptool.py v4.5.1` and `4.5.1`.
> > - ❌ `ModuleNotFoundError: No module named 'serial'` → **Fix 2.3**
> >   below.
> >
> > > #### Fix 2.3 — `No module named 'serial'`
> > >
> > > ```
> > > sudo apt install python3-serial
> > > ```
> > >
> > > **Then:** repeat 2.3.
> >
> > **Expected noise:** above the version, two lines ending in
> > `SyntaxWarning: 'return' in a 'finally' block` and `return inst`.
> > That is Ubuntu's Python 3.14 objecting to esptool's old code. It is
> > harmless and will appear every time esptool runs, including inside
> > the build in step 3 and in step 5.

> ## Step 3 — Build the firmware
>
> **Board:** unplugged · **Jumper:** none · **Run from:** repository root
>
> > ### 3.1 Build
> >
> > Run this as one line — the `echo` reports whether the build
> > succeeded:
> >
> > ```
> > ./compile.sh; echo "EXIT STATUS: $?"
> > ```
> >
> > It prints about 450 lines. It took 30 seconds when this procedure was
> > tested; a slower machine can take a few minutes.
> >
> > - ✅ The last line is `EXIT STATUS: 0`, and about 20 lines above it
> >   (above a list of libraries) you can see
> >   `Sketch uses … bytes (70%) of program storage space`.
> > - ❌ `EXIT STATUS:` followed by anything other than `0` → **Fix 3.1**
> >   below.
> >
> > **Expected noise:** the esptool `SyntaxWarning` from 2.3, twice.
> > Ignore the `70%` figure — it is measured against the wrong limit;
> > 3.3 does the real check.
> >
> > > #### Fix 3.1 — the build failed
> > >
> > > Scroll up to the **first** line containing `error:` — later errors
> > > are usually knock-on effects. Common causes:
> > >
> > > - `Error resolving FQBN` or "platform not found" → you are not in
> > >   the repository root (P.8), or step 1 did not complete (1.2).
> > > - `xtensa-esp32s3-elf … does not exist` → step 2 did not complete
> > >   (2.2).
> > > - `Sketch too big` → `compile.sh` was edited; restore it with
> > >   `git checkout -- compile.sh`.
> > > - Anything else → stop and look up the first `error:` line.
> > >
> > > **Then:** repeat 3.1.
>
> > ### 3.2 Check the build output
> >
> > ```
> > ls -l UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.{bin,bootloader.bin,partitions.bin}
> > ```
> >
> > - ✅ Exactly three rows, and no `No such file or directory`.
> > - ❌ Any `No such file or directory` → **Fix 3.2** below.
> >
> > > #### Fix 3.2 — the build output is missing
> > >
> > > The build did not complete, even if you missed the error. Repeat
> > > 3.1 and check its `EXIT STATUS:` line.
> > >
> > > **Then:** repeat 3.2.
>
> > ### 3.3 Check the firmware fits
> >
> > ```
> > stat -c %s UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bin
> > ```
> >
> > - ✅ A number **no larger than `1638400`**. For the stock firmware it
> >   is about `1180000`: `1179856` in `~/git/unor4-wifi-interrupt-bridge`
> >   on 2026-10-22 (the build that was flashed and released), `1180480`
> >   in another folder. The folder's path is compiled into the
> >   firmware, so a difference of a few hundred bytes is normal.
> > - ❌ Larger than `1638400` → **Fix 3.3** below. Do not continue.
> >
> > > #### Fix 3.3 — the firmware is larger than 1638400 bytes
> > >
> > > The stock firmware is about 1180000 bytes, so this means the
> > > source has been modified. A firmware this large builds without
> > > complaint but will not boot. **Do not flash it.** Undo the
> > > modification (or make it smaller).
> > >
> > > **Then:** repeat from 3.1.

> ## Step 4 — Make the flashable image
>
> **Board:** unplugged · **Jumper:** none · **Run from:** repository root
>
> > ### 4.1 Export
> >
> > ```
> > ./export.sh
> > ```
> >
> > It deletes the step-3 build, rebuilds from scratch (the same ~450
> > lines), then combines everything into one image.
> >
> > - ✅ Near the end you see
> >   `gen_crt_bundle.py: Successfully added 151 certificates in total`,
> >   then `Combine binaries`, then `Done`.
> > - ❌ `python: command not found`, `Traceback`, or any `Error` →
> >   **Fix 4.1** below.
> >
> > **Expected noise:** the esptool `SyntaxWarning`, and one
> > `CryptographyDeprecationWarning: Parsed a serial number which wasn't positive …`.
> > Both are harmless.
> >
> > > #### Fix 4.1 — the export failed
> > >
> > > Scroll up through `export.sh`'s output for the first error:
> > >
> > > - `python: command not found` →
> > >   `sudo apt install python-is-python3`.
> > > - `The cryptography package is not installed` →
> > >   `sudo apt install python3-cryptography`.
> > > - Anything else → stop and look up the first error line.
> > >
> > > **Then:** repeat 4.1 (it always starts from scratch, so re-running
> > > it is safe).
> >
> > > [!WARNING]
> > > `Done` on its own proves nothing — `export.sh` prints it even when
> > > it failed. 4.2 is the real check.
>
> > ### 4.2 Check the images
> >
> > ```
> > ls -l UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-*.bin
> > ```
> >
> > ✅ Exactly three rows with exactly these sizes (the fifth column):
> >
> > | File | Size in bytes |
> > |---|---|
> > | `S3-ALL.bin` | `4194304` |
> > | `S3-APP.bin` | `1908736` |
> > | `S3-BOOT-APP.bin` | `1966080` |
> >
> > ❌ Any file missing, or any size different → **Fix 4.2** below.
> >
> > > #### Fix 4.2 — the images are missing or the wrong size
> > >
> > > The export did not complete, even though it printed `Done`.
> > > Repeat 4.1 and read its output for the first error.
> > >
> > > **Then:** repeat 4.2.
> >
> > > [!WARNING]
> > > The only file you will flash is `S3-BOOT-APP.bin`. Never flash
> > > `S3-ALL.bin` (it erases the board's saved settings) or `S3-APP.bin`
> > > (it stops the board booting).

> ## Step 5 — Back up, then flash the board
>
> From here on, the board is involved. Each sub-step's board state:
>
> | Step | Board | Jumper |
> |---|---|---|
> | 5.0 Who owns USB | plugged in | off |
> | 5.1 Record the current firmware | plugged in | off |
> | 5.2 Download mode (method A) | plugged in | off |
> | 5.2B Download mode (method B) | unplugged → plugged in with jumper | on while plugging in, then off |
> | 5.3 Verify download mode | plugged in | off |
> | 5.4 Backup | plugged in — **do not touch** | off |
> | 5.5 Flash | plugged in — **do not touch** | off |
> | 5.6 Check | unplug → plug in | **off** |
>
> > [!IMPORTANT]
> > In step 5, any result that differs from its ✅ and is not covered by
> > a ❌ means **stop**. Never unplug the board while 5.4 or 5.5 is
> > running.
>
> > [!IMPORTANT]
> > Close the Arduino IDE before step 5 and keep it closed until 5.6.
> > Its background serial-port scanner (`serial-discovery` 1.5.0) is a
> > known cause of 5.5 failing part-way with `Bad data checksum` and
> > leaving the board unbootable (Arduino bridge issue #78). Check that
> > nothing is scanning — this must print nothing:
> >
> > ```
> > pgrep -a serial-discovery
> > ```
>
> You never type the board's serial port yourself. Commands that talk
> to the board use `"${PORT:?is not set}"`: the shell variable `PORT`,
> set from the port the board actually has. 5.3 sets it in download
> mode (and Fix 5.0-A part 4 sets it for its upload). If `PORT` is not
> set, the command does not run at all and the terminal prints
> `bash: PORT: is not set` — never a guess at the port.
>
> > [!IMPORTANT]
> > `PORT` exists only in the terminal window where it was set. Run
> > every command from 5.3 to the end of 5.5 in that same window. If
> > you close it or switch to another one, repeat 5.3 there first.
>
> The port capture was added after this procedure's run on the board
> (2026-10-22). The same capture for the board running normally was
> checked on the board; the download-mode capture in 5.3 has not yet
> been used on hardware.
>
> > ### 5.0 Check who owns the USB connector
> >
> > **Board:** plugged in · **Jumper:** none
> >
> > > [!IMPORTANT]
> > > **Plug the board in now**, with no jumper on it, using a USB-C
> > > data cable — preferably straight into the computer rather than
> > > through a hub or dock. Its power LED must be on before you run the
> > > command below.
> >
> > ```
> > lsusb | grep -Ei ' (2341|303a):'
> > ```
> >
> > Exactly one line appears:
> >
> > - ✅ `2341:1002` → go to 5.1.
> > - ✅ `303a:1001` → the board is already in download mode; go to 5.3.
> > - ❌ `2341:006d` → **Fix 5.0-A** below.
> > - ❌ Nothing → **Fix 5.0-B** below.
> >
> > > #### Fix 5.0-A — `lsusb` shows `2341:006d`
> > >
> > > The USB connector has been handed to the board's other chip (the
> > > Renesas RA4M1), because the sketch on it uses the USB port
> > > directly. While that sketch runs, the computer cannot reach the
> > > ESP32 at all — neither download-mode method will work, the jumper
> > > included.
> > >
> > > The fix is to replace that sketch with an empty one.
> > >
> > > > [!CAUTION]
> > > > This erases the sketch currently on the board. If you want it
> > > > back later, make sure you have its source.
> > >
> > > > ##### Fix 5.0-A, part 1 — Install the UNO R4 board support
> > > >
> > > > One time, about 20 seconds. It goes into `~/.arduino15`, not the
> > > > repository:
> > > >
> > > > ```
> > > > arduino-cli core update-index
> > > > arduino-cli core install arduino:renesas_uno
> > > > ```
> > > >
> > > > ✅ The last line is `Platform arduino:renesas_uno@1.6.0 installed`
> > > > (or a newer version number).
> > > >
> > > > **Expected noise:** just before that line, a suggestion to run
> > > > `sudo "…/renesas_uno/1.6.0/post_install.sh"`. Do not run it — it
> > > > makes every Arduino USB device writable by every user, which
> > > > this procedure does not need (P.5's `dialout` group covers the
> > > > upload).
> > >
> > > > ##### Fix 5.0-A, part 2 — Create and build an empty sketch
> > > >
> > > > ```
> > > > mkdir -p /tmp/Empty
> > > > printf 'void setup() {}\nvoid loop() {}\n' > /tmp/Empty/Empty.ino
> > > > arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi /tmp/Empty
> > > > ```
> > > >
> > > > ✅ Ends with `Sketch uses … bytes` and `Global variables use …`
> > > > lines and no `Error`.
> > >
> > > > ##### Fix 5.0-A, part 3 — Put the board into bootloader mode
> > > >
> > > > Double-tap the board's RESET button (two quick presses).
> > > >
> > > > - ✅ The onboard LED fades in and out — the board is in
> > > >   bootloader mode and waiting for an upload. Do parts 4 and 5
> > > >   straight away.
> > > > - ❌ The LED does not fade → the taps were too slow; try again,
> > > >   faster.
> > >
> > > > ##### Fix 5.0-A, part 4 — Capture the board's port
> > > >
> > > > The port can change after the double-tap, so capture it now.
> > > > Run both lines; the first finds the port and stores it in
> > > > `PORT`, the second shows it:
> > > >
> > > > ```
> > > > PORT=$(readlink -f $(find /dev/serial/by-id/ -iname '*UNO_WiFi_R4_CMSIS-DAP*' 2>/dev/null) 2>/dev/null)
> > > > echo "PORT is $PORT"
> > > > ```
> > > >
> > > > - ✅ The last line is `PORT is /dev/ttyACM` followed by a number
> > > >   (for example `PORT is /dev/ttyACM0`). It finds the port that
> > > >   `ls -l /dev/serial/by-id/` lists as
> > > >   `usb-Arduino_UNO_WiFi_R4_CMSIS-DAP_…` (`…` is the board's serial
> > > >   number), which is what showed up after the double-tap on
> > > >   2026-10-22. Do part 5 straight away, in this same window.
> > > > - ❌ Just `PORT is` with nothing after it → the board is not in
> > > >   bootloader mode any more. Repeat from part 3.
> > >
> > > > ##### Fix 5.0-A, part 5 — Upload the empty sketch
> > > >
> > > > ```
> > > > arduino-cli upload --fqbn arduino:renesas_uno:unor4wifi --port "${PORT:?is not set}" /tmp/Empty
> > > > ```
> > > >
> > > > - ✅ It finishes without `Error`.
> > > > - ❌ `bash: PORT: is not set` → this window never ran part 4.
> > > >   Repeat from part 3 in this window.
> > > > - ❌ It fails → repeat from part 3 (double-tap) once. If it fails
> > > >   again, **stop** — recovering this chip is outside this
> > > >   procedure.
> > >
> > > > ##### Fix 5.0-A, part 6 — Power-cycle the board
> > > >
> > > > Unplug the board, wait five seconds, plug it back in.
> > >
> > > This whole fix was run on a board on 2026-10-22: after part 3's
> > > double-tap, the port showed up as `UNO_WiFi_R4_CMSIS-DAP`, the
> > > upload succeeded, and 5.0 then showed `2341:1002`. A double-tap
> > > before uploading is the established practice for uploads to this
> > > board.
> > > The UNO R4 board support officially lists `2341:006d` as an upload
> > > port, so it is a path Arduino designed for.
> > >
> > > **Then:** repeat 5.0. If 5.0 *still* shows `2341:006d` after this,
> > > stop: the board may have solder jumper SJ1 bridged, which hands the
> > > connector to the RA4M1 permanently.
> >
> > > #### Fix 5.0-B — `lsusb` shows nothing
> > >
> > > - Use a USB-C **data** cable; charge-only cables connect nothing.
> > > - Try a different USB port on the computer.
> > > - Check the board's power LED is on.
> > >
> > > **Then:** repeat 5.0.
>
> > ### 5.1 Record the firmware that is on the board now
> >
> > **Board:** plugged in · **Jumper:** none
> >
> > Only after 5.0 showed `2341:1002`.
> >
> > ```
> > lsusb -v -d 2341:1002 2>/dev/null | grep -E 'bcdDevice|iProduct'
> > ```
> >
> > ✅ Shows an `iProduct` line containing `UNO WiFi R4 CMSIS-DAP` and a
> > `bcdDevice` line. Write down the `bcdDevice` number, then go to 5.2.
> >
> > - `0.06` means firmware 0.6.0.
> > - `1.00` is **not** a version. It is a placeholder that means "this
> >   firmware does not report its version", which is what every release
> >   older than 0.6.0 shows. So `1.00` is *older* than `0.06`.
>
> > ### 5.2 Enter download mode — method A (by command)
> >
> > **Board:** plugged in · **Jumper:** none
> >
> > > #### 5.2.1 Fetch Arduino's reboot helper
> > >
> > > Download and unpack it:
> > >
> > > ```
> > > curl -sL -o /tmp/unor4wifi-reboot-linux64.zip https://github.com/bcmi-labs/uno-r4-wifi-usb-bridge/releases/download/0.1.0/unor4wifi-reboot-linux64.zip
> > > unzip -o /tmp/unor4wifi-reboot-linux64.zip -d /tmp/unor4wifi-reboot
> > > ```
> > >
> > > ✅ `unzip` prints `inflating: /tmp/unor4wifi-reboot/unor4wifi-reboot-linux64`.
> >
> > > #### 5.2.2 Run the helper
> > >
> > > Run it as one line:
> > >
> > > ```
> > > sudo /tmp/unor4wifi-reboot/unor4wifi-reboot-linux64; echo "EXIT STATUS: $?"
> > > ```
> > >
> > > - ✅ The only thing printed is `EXIT STATUS: 0`. Wait two seconds,
> > >   then go to **5.3**.
> > > - ❌ Anything else → **Fix 5.2.2** below.
> > >
> > > > ##### Fix 5.2.2 — the reboot helper failed
> > > >
> > > > - `Cannot enumerate, try direct open` then `No board connected`
> > > >   → the helper could not see the board's firmware. Repeat 5.0
> > > >   and follow what its table says.
> > > > - `panic: runtime error: invalid memory address or nil pointer
> > > >   dereference` → it was run without `sudo`. Repeat 5.2.2 exactly
> > > >   as written.
> > > > - `sudo: …: command not found` or `No such file or directory` →
> > > >   5.2.1 did not complete. Repeat 5.2.1, then 5.2.2.
> > > >
> > > > If it fails a second time, do not keep retrying.
> > > >
> > > > **Then:** use method B, 5.2B, just below.
>
> > ### 5.2B Enter download mode — method B (by jumper)
> >
> > Only if the fix in 5.2.2 or the fix in 5.3 sent you here.
> >
> > **Board:** unplugged → plugged in with jumper · **Jumper:** see below
> >
> > > #### 5.2B.1 Unplug the board
> > >
> > > Unplug the board from USB.
> >
> > > #### 5.2B.2 Find the two pins
> > >
> > > The jumper goes across **pin 4
> > > (DOWNLOAD)** and **pin 6 (GND)** of the ESP header, the 2×3 header
> > > next to the USB-C connector and the RESET button. Use the picture
> > > (Arduino's official pinout, annotated), then confirm against the
> > > photo in
> > > [Arduino's restore article](https://support.arduino.cc/hc/en-us/articles/16379769332892-Restore-the-USB-connectivity-firmware-on-UNO-R4-WiFi-with-espflash).
> > > Both must agree.
> > >
> > > ![UNO R4 WiFi: jumper on ESP header pins 6 (GND) and 4 (DOWNLOAD); ICSP header marked as wrong](img/unor4wifi-esp-download-jumper.png)
> > >
> > > > [!CAUTION]
> > > > Do not use the other 2×3 header (ICSP, at the far end from the
> > > > USB-C connector): its pin 2 is +5 V, and a jumper on the wrong pair
> > > > there shorts the supply to ground. Never connect anything at 5 V to
> > > > the ESP header — its pins are 3.3 V.
> >
> > > #### 5.2B.3 Fit the jumper
> > >
> > > Put the jumper wire across pin 4 and pin 6.
> >
> > > #### 5.2B.4 Plug the board in
> > >
> > > With the jumper in place, plug the board in.
> > >
> > > > [!IMPORTANT]
> > > > The jumper must be on **before** the board gets power. Putting it
> > > > on a board that is already plugged in does nothing.
> >
> > > #### 5.2B.5 Remove the jumper
> > >
> > > Remove the jumper. The board stays in download mode until it
> > > is next unplugged. Go to **5.3**.
>
> > ### 5.3 Verify download mode and capture the port
> >
> > **Board:** plugged in · **Jumper:** none
> >
> > Run all three lines. The second one finds the board's port in
> > download mode and stores it in `PORT`; the third shows it:
> >
> > ```
> > lsusb -d 303a:1001
> > PORT=$(readlink -f $(find /dev/serial/by-id/ -iname '*espressif*' 2>/dev/null) 2>/dev/null)
> > echo "PORT is $PORT"
> > ```
> >
> > - ✅ `lsusb` prints exactly one line, and the last line is
> >   `PORT is /dev/ttyACM` followed by a number (for example
> >   `PORT is /dev/ttyACM0`).
> > - ❌ `lsusb` prints nothing, or the last line is just `PORT is`
> >   with nothing after it → **Fix 5.3** below.
> > - ❌ `PORT is` is followed by **two** ports, on two lines → another
> >   ESP32 board is plugged in. Unplug it, then repeat 5.3.
> >
> > > #### Fix 5.3 — the board is not in download mode
> > >
> > > Re-run the 5.0 command (`lsusb | grep -Ei ' (2341|303a):'`):
> > >
> > > - `303a:1001` → the board *is* in download mode, but its serial
> > >   port had not appeared yet when `PORT` was set. Wait five seconds
> > >   and repeat 5.3 once. If `PORT is` is still empty, **stop**.
> > > - `2341:006d` → go back to 5.0 and do its Fix 5.0-A.
> > > - `2341:1002` → the board is running its firmware normally. If you
> > >   came from 5.2, wait five seconds and repeat 5.3 once; if it still
> > >   fails, use 5.2B.
> > > - Nothing → go back to 5.0 and do its Fix 5.0-B.
> > >
> > > If you came from 5.2B: the jumper was probably not on the right
> > > pins, or not on before the board got power. Repeat 5.2B from
> > > 5.2B.1.
> > >
> > > **Then:** repeat 5.3.
>
> > ### 5.4 Back up the board
> >
> > **Board:** plugged in — do not touch · **Jumper:** none
> >
> > > [!IMPORTANT]
> > > Do not skip this. It is the only way back to exactly what is on the
> > > board now. It contains your saved WiFi credentials — it is stored
> > > outside the repository so it can never end up in git.
> >
> > > #### 5.4.1 Identify the chip
> > >
> > > Run:
> > >
> > > ```
> > > python3 hardware/esp32-patched/esp32/tools/esptool/esptool.py --chip esp32s3 --port "${PORT:?is not set}" --after no_reset flash_id
> > > ```
> > >
> > > - ✅ Among the output: `Chip is ESP32-S3 (revision v…)`, then
> > >   `Detected flash size: 8MB`, and finally `Staying in bootloader.`
> > > - ❌ `Detected flash size: 4MB` → use `0x400000` instead of
> > >   `0x800000` in 5.4.2 and `4194304` instead of `8388608` in 5.4.3.
> > > - ❌ `bash: PORT: is not set`, `Failed to connect` or any `Error` →
> > >   **Fix 5.4.1** below.
> > >
> > > > ##### Fix 5.4.1 — esptool could not talk to the board
> > > >
> > > > Nothing has been written; this only reads.
> > > >
> > > > - `bash: PORT: is not set` → this terminal window never ran 5.3.
> > > >   The board is still in download mode: repeat 5.3 in this window
> > > >   (no need to unplug), then repeat 5.4.1.
> > > > - `Permission denied` → P.7 was not passed. Do P.5 to P.7, then
> > > >   come back.
> > > > - `Failed to connect` or `port is busy` → something else is
> > > >   using the port. Close the Arduino IDE and any serial monitor.
> > > >   Ubuntu's ModemManager can also grab new serial ports; stop it
> > > >   for this session with `sudo systemctl stop ModemManager` (it
> > > >   comes back on the next boot).
> > > >
> > > > Then unplug the board, re-enter download mode (5.2 or 5.2B), and
> > > > confirm with 5.3.
> > > >
> > > > **Then:** repeat 5.4.1.
> >
> > > #### 5.4.2 Read the chip into a backup file
> > >
> > > Run:
> > >
> > > ```
> > > mkdir -p ~/unor4wifi-backups
> > > python3 hardware/esp32-patched/esp32/tools/esptool/esptool.py --chip esp32s3 --port "${PORT:?is not set}" --after no_reset read_flash 0 0x800000 ~/unor4wifi-backups/esp32s3-$(date +%Y%m%d-%H%M%S).bin
> > > ```
> > >
> > > It takes about **15 minutes** (measured on 2026-10-22: started
> > > 01:48, finished 02:03). It shows a percentage while it runs.
> > >
> > > > [!IMPORTANT]
> > > > Do not touch the board or cable while it runs, and stop the
> > > > computer from going to sleep for the duration.
> > >
> > > - ✅ Ends with `Read 8388608 bytes at 0x00000000 in … seconds …`
> > >   and then `Staying in bootloader.`
> > > - ❌ Anything else → **Fix 5.4.2** below.
> > >
> > > > ##### Fix 5.4.2 — the read failed
> > > >
> > > > Nothing has been written; this only reads.
> > > >
> > > > 1. Delete the incomplete backup file. List the folder, then
> > > >    `rm` the file whose size is not `8388608`:
> > > >
> > > >    ```
> > > >    ls -l ~/unor4wifi-backups/
> > > >    ```
> > > >
> > > > 2. Do the checks in Fix 5.4.1 just above (permissions, port in
> > > >    use, ModemManager).
> > > > 3. Unplug the board, re-enter download mode (5.2 or 5.2B), and
> > > >    confirm with 5.3.
> > > >
> > > > **Then:** repeat from 5.4.1.
> >
> > > #### 5.4.3 Check the backup file
> > >
> > > Run:
> > >
> > > ```
> > > ls -l ~/unor4wifi-backups/
> > > od -An -tx1 -N1 ~/unor4wifi-backups/esp32s3-*.bin
> > > ```
> > >
> > > - ✅ The file's size is exactly `8388608`, and `od` prints ` e9`.
> > > - ❌ Anything else → **Fix 5.4.3** below. Do not go to 5.5.
> > >
> > > > ##### Fix 5.4.3 — the backup file is wrong
> > > >
> > > > The read did not produce a usable backup. Delete the bad file
> > > > (`rm` it, using the name `ls` showed), unplug the board,
> > > > re-enter download mode (5.2 or 5.2B), and confirm with 5.3.
> > > >
> > > > **Then:** repeat from 5.4.1.
>
> > ### 5.5 Flash the firmware
> >
> > **Board:** plugged in — do not touch · **Jumper:** none
> >
> > ```
> > espflash write-bin -b 115200 --port "${PORT:?is not set}" 0x0 UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-BOOT-APP.bin
> > ```
> >
> > If it asks `Remember this serial port for future use? (y/n)`, press
> > `n`.
> >
> > It takes about 10 seconds. It first prints the chip details —
> > `Using flash stub`, `Chip type: esp32s3`, `Flash size: 8MB`, the
> > board's MAC address — then a progress bar.
> >
> > - ✅ The progress bar fills to the end and the command returns to
> >   the prompt with no error. Measured:
> >   `[00:00:10] [========================================]     803/803     0x0`.
> > - ❌ It cannot connect, or stops with an error → **Fix 5.5** below.
> >
> > **Expected noise:** the first line,
> > `🚀 A new version of espflash is available: v4.6.0` (or newer).
> >
> > > [!WARNING]
> > > Do not upgrade espflash. In espflash 4.x, `-b` no longer means
> > > baud rate, so this exact command would fail. P.4 pins 2.0.0, the
> > > version Arduino's own updater ships.
> >
> > > #### Fix 5.5 — the flash could not connect or stopped with an error
> > >
> > > - **`bash: PORT: is not set`** (nothing was run): this terminal
> > >   window never ran 5.3. The board is still in download mode:
> > >   repeat 5.3 in this window (no need to unplug), then repeat 5.5.
> > > - **It could not connect** (nothing was written): close the
> > >   Arduino IDE and any serial monitor, and stop ModemManager for
> > >   this session (`sudo systemctl stop ModemManager`). Then unplug,
> > >   re-enter download mode with 5.2B, confirm with 5.3, and repeat
> > >   5.5.
> > > - **It stopped part-way with `Bad data checksum`:** something was
> > >   scanning the serial port. Close the Arduino IDE, confirm
> > >   `pgrep -a serial-discovery` prints nothing, then do the next
> > >   bullet.
> > > - **It stopped part-way:** the board will not boot until it is
> > >   flashed completely, but nothing is damaged. Unplug, re-enter
> > >   download mode with **5.2B** (method A cannot work now), confirm
> > >   with 5.3, and repeat 5.5.
> > >
> > > **Then:** repeat 5.5.
>
> > ### 5.6 Check the new firmware
> >
> > **Board:** unplug → plug in · **Jumper:** **off**
> >
> > > #### 5.6.1 Power-cycle the board
> > >
> > > Unplug the board, make sure no jumper is on the ESP header,
> > > wait five seconds, plug it back in, wait five more seconds.
> >
> > > #### 5.6.2 Check the firmware
> > >
> > > Run:
> > >
> > > ```
> > > lsusb -v -d 2341:1002 2>/dev/null | grep -E 'bcdDevice|iProduct'
> > > ```
> > >
> > > - ✅ An `iProduct` line containing `UNO WiFi R4 CMSIS-DAP` and
> > >   `bcdDevice 0.06`.
> > > - ❌ It prints nothing → **Fix 5.6.2** below.
> > >
> > > > ##### Fix 5.6.2 — the new firmware does not show up
> > > >
> > > > Run `lsusb | grep -Ei ' (2341|303a):'`:
> > > >
> > > > - `303a:1001` → the jumper is still on, or the board was not
> > > >   unplugged. Unplug, remove the jumper, plug in, repeat 5.6.2.
> > > > - `2341:006d` → the RA4M1 sketch has taken the connector (see
> > > >   5.0's Fix 5.0-A); the ESP32 flash is probably fine.
> > > > - Nothing → the flash did not complete. Re-enter download mode
> > > >   with **5.2B**, confirm with 5.3, and repeat 5.5. If you would
> > > >   rather go back to exactly what was there before, use
> > > >   **Recovery** at the end of this document instead.
> > > >
> > > > **Then:** repeat 5.6.2.


> ## Done
>
> The board is running stock bridge firmware v0.6.0, and your backup of
> what was there before is in `~/unor4wifi-backups/`.

## Recovery — restore your backup

> [!WARNING]
> This is not part of the normal procedure. Use it only if you want
> the board back exactly as it was before step 5.

> ### R.1 Restore the step-5.4 backup
>
> > #### R.1.1 Enter download mode
> >
> > Use 5.2 or 5.2B, and confirm with 5.3.
>
> > #### R.1.2 Find the backup's exact file name
> >
> > ```
> > ls ~/unor4wifi-backups/
> > ```
>
> > #### R.1.3 Write it back
> >
> > In the same terminal window as R.1.1's 5.3. Replace `<file-name>`
> > with the name from R.1.2:
> >
> > ```
> > python3 hardware/esp32-patched/esp32/tools/esptool/esptool.py --chip esp32s3 --port "${PORT:?is not set}" write_flash 0x0 ~/unor4wifi-backups/<file-name>
> > ```
> >
> > ✅ It prints `Hash of data verified.` and then `Leaving...`.
>
> > #### R.1.4 Check
> >
> > Do 5.6. Its ✅ will show the firmware version you wrote down in
> > 5.1 instead of `0.06`.
