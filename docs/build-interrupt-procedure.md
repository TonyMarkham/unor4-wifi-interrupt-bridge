# UNO R4 WiFi interrupt bridge 0.6.101 — build procedure

This procedure builds the **interrupt fork** of the ESP32-S3 bridge
firmware: Arduino's 0.6.0 plus UDP push mode (`AT+UDPPUSH`, `+UDPRX:`
frames). The result is one image file, saved outside the repository,
ready for [flash-interrupt-procedure.md](flash-interrupt-procedure.md).
It contains only what you do and what you should see.

This procedure does not touch the board.

**Run on 2026-10-22.** Followed as written, starting from an empty
`~/unor4wifi-images`; the image matched an earlier build byte for byte
(the SHA256 quoted in 2.3).

> [!IMPORTANT]
> This procedure assumes you have already followed
> [build-original-procedure.md](build-original-procedure.md) on this
> computer, at least steps P–4. It reuses that procedure's packages,
> submodules and toolchain, and does not repeat them. Step P below
> checks they are still there.

## How to use this procedure

- Do the actions in order. Each one has a number (`1.1`, `1.2`, …).
- Each step starts with a **status line**: whether the board is
  plugged in, whether the jumper is on, and where to run commands.
- Each action shows what to run, then:
  - ✅ — what you must see. If you see it, go to the next action.
  - ❌ — what to do if you see something else. Usually "→ **Fix 1.1**
    below": the fix is in its own box directly underneath, inside the
    same action. It ends with **Then:**, which tells you what to do
    next. If you got a ✅, skip the fix boxes.
  - **Expected noise** — warnings you will see that are normal.
- If a result matches neither the ✅ nor any ❌: **stop.** Do not
  improvise.

> ## Step P — Check the starting point
>
> **Board:** not needed · **Jumper:** none · **Run from:** anywhere
>
> > ### P.1 Go to the repository and the fork branch
> >
> > The fork is built from the `feature/interrupt` branch: the stock
> > branch `feature/docs` plus the push-mode change.
> >
> > ```
> > cd ~/git/unor4-wifi-interrupt-bridge
> > git switch feature/interrupt
> > git status
> > ```
> >
> > - ✅ `git status` prints `On branch feature/interrupt` → go to P.2.
> > - ❌ `git switch` says `invalid reference: feature/interrupt` →
> >   **Fix P.1-A** below.
> > - ❌ `git switch` says
> >   `Your local changes to the following files would be overwritten`
> >   → **Fix P.1-B** below.
> >
> > **Every command from here on runs from this folder** unless an
> > action says otherwise.
> >
> > > #### Fix P.1-A — the `feature/interrupt` branch does not exist
> > >
> > > Your copy of the repository does not have the branch yet. Fetch
> > > it:
> > >
> > > ```
> > > git fetch origin
> > > ```
> > >
> > > If P.1 still fails after this, the branch has not been published
> > > yet. **Stop** — there is nothing to build.
> > >
> > > **Then:** repeat P.1.
> >
> > > #### Fix P.1-B — local changes block the switch
> > >
> > > Files in the repository were edited by hand. `git switch` changed
> > > nothing. **Stop**, and decide what to do with those edits before
> > > building — this procedure must build the branch exactly as
> > > committed.
> > >
> > > **Then:** repeat P.1.
>
> > ### P.2 Check this is the fork's source
> >
> > ```
> > grep -E 'define (ESP_FW_VERSION|_UDPPUSH)' UNOR4USBBridge/at_handler.h UNOR4USBBridge/commands.h
> > ```
> >
> > - ✅ Exactly two lines: one ending in `"0.6.101"`, one ending in
> >   `"+UDPPUSH"`.
> > - ❌ The `ESP_FW_VERSION` line ends in `"0.6.0"`, or there is no
> >   `_UDPPUSH` line → you are on the stock source. Repeat P.1 and
> >   check its `On branch` line.
>
> > ### P.3 Check the submodules
> >
> > ```
> > git submodule status
> > ```
> >
> > - ✅ Exactly seven lines, and none of them starts with `-` or `+`.
> > - ❌ Any line starts with `-` or `+` → **Fix P.3** below.
> >
> > > #### Fix P.3 — a submodule is missing or on the wrong commit
> > >
> > > Fetch them again; this is build-original-procedure.md's step 1.1.
> > > It takes about 2 minutes and can go quiet for long stretches:
> > >
> > > ```
> > > git submodule update --init --depth 1 --no-single-branch
> > > ```
> > >
> > > **Then:** repeat P.3.
>
> > ### P.4 Check the compiler
> >
> > ```
> > hardware/esp32-patched/esp32/tools/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gcc --version | head -1
> > ```
> >
> > - ✅ Prints `xtensa-esp32s3-elf-gcc (crosstool-NG esp-2021r2-patch5) 8.4.0`.
> > - ❌ Prints nothing, or `No such file or directory` → the toolchain
> >   is missing. Do build-original-procedure.md's step 2 (2.1 to 2.3).
> >   **Then:** repeat P.4.

> ## Step 1 — Build the firmware
>
> **Board:** not needed · **Jumper:** none · **Run from:** repository root
>
> > ### 1.1 Build
> >
> > Run this as one line — the `echo` reports whether the build
> > succeeded:
> >
> > ```
> > ./compile.sh; echo "EXIT STATUS: $?"
> > ```
> >
> > It prints about 450 lines and takes about 30 seconds.
> >
> > - ✅ The last line is `EXIT STATUS: 0`, and about 20 lines above it
> >   you can see `Sketch uses … bytes (70%) of program storage space`
> >   (`Sketch uses 1180257 bytes` when this procedure was tested).
> > - ❌ `EXIT STATUS:` followed by anything other than `0` → **Fix 1.1**
> >   below.
> >
> > **Expected noise:** the esptool `SyntaxWarning`, twice. Ignore the
> > `70%` figure — it is measured against the wrong limit; 1.3 does the
> > real check.
> >
> > > #### Fix 1.1 — the build failed
> > >
> > > Scroll up to the **first** line containing `error:` — later errors
> > > are usually knock-on effects.
> > >
> > > - `Error resolving FQBN` or "platform not found" → you are not in
> > >   the repository root (P.1), or the submodules are incomplete
> > >   (P.3).
> > > - `xtensa-esp32s3-elf … does not exist` → the toolchain is missing
> > >   (P.4).
> > > - The error names a file in `UNOR4USBBridge/` → the fork's source
> > >   does not compile. **Stop** — this is a problem in the branch, not
> > >   on your computer.
> > > - Anything else → stop and look up the first `error:` line.
> > >
> > > **Then:** repeat 1.1.
>
> > ### 1.2 Check the build output
> >
> > ```
> > ls -l UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.{bin,bootloader.bin,partitions.bin}
> > ```
> >
> > - ✅ Exactly three rows, and no `No such file or directory`.
> > - ❌ Any `No such file or directory` → the build did not complete.
> >   Repeat 1.1 and check its `EXIT STATUS:` line. **Then:** repeat
> >   1.2.
>
> > ### 1.3 Check the firmware fits
> >
> > ```
> > stat -c %s UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bin
> > ```
> >
> > - ✅ A number **no larger than `1638400`**. It is about `1181000`
> >   (`1180624` when this procedure was tested — about 800 bytes more
> >   than the stock firmware).
> > - ❌ Larger than `1638400` → **Fix 1.3** below. Do not continue.
> >
> > > #### Fix 1.3 — the firmware is larger than 1638400 bytes
> > >
> > > A firmware this large builds without complaint but will not boot.
> > > **Do not flash it.** The push-mode change adds less than 1 KB, so
> > > this means the source holds other changes. **Stop.**
> > >
> > > **Then:** nothing — do not continue this procedure.
>
> > ### 1.4 Check the firmware is the fork
> >
> > ```
> > grep -a -o -E 'UNO WiFi R4 CMSIS-DAP \(interrupt\)|0\.6\.101|\+UDPPUSH' UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bin | sort -u
> > ```
> >
> > - ✅ Exactly these three lines:
> >
> >   ```
> >   +UDPPUSH
> >   0.6.101
> >   UNO WiFi R4 CMSIS-DAP (interrupt)
> >   ```
> >
> > - ❌ Fewer lines, or nothing → the build is not the fork. Repeat
> >   P.1 and P.2, then 1.1. **Then:** repeat 1.4.

> ## Step 2 — Make and save the flashable image
>
> **Board:** not needed · **Jumper:** none · **Run from:** repository root
>
> > ### 2.1 Export
> >
> > ```
> > ./export.sh
> > ```
> >
> > It deletes the step-1 build, rebuilds from scratch (the same ~450
> > lines), then combines everything into one image.
> >
> > - ✅ Near the end you see
> >   `gen_crt_bundle.py: Successfully added 151 certificates in total`,
> >   then `Combine binaries`, then `Done`.
> > - ❌ `python: command not found`, `Traceback`, or any `Error` →
> >   **Fix 2.1** below.
> >
> > **Expected noise:** the esptool `SyntaxWarning`, and one
> > `CryptographyDeprecationWarning: Parsed a serial number which wasn't positive …`.
> > Both are harmless.
> >
> > > #### Fix 2.1 — the export failed
> > >
> > > Scroll up through `export.sh`'s output for the first error:
> > >
> > > - `python: command not found` →
> > >   `sudo apt install python-is-python3`.
> > > - `The cryptography package is not installed` →
> > >   `sudo apt install python3-cryptography`.
> > > - Anything else → stop and look up the first error line.
> > >
> > > **Then:** repeat 2.1 (it always starts from scratch, so re-running
> > > it is safe).
> >
> > > [!WARNING]
> > > `Done` on its own proves nothing — `export.sh` prints it even when
> > > it failed. 2.2 is the real check.
>
> > ### 2.2 Check the images
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
> > ❌ Any file missing, or any size different → the export did not
> > complete, even though it printed `Done`. Repeat 2.1 and read its
> > output for the first error. **Then:** repeat 2.2.
> >
> > > [!WARNING]
> > > The only image that is ever flashed is `S3-BOOT-APP.bin`. Never
> > > flash `S3-ALL.bin` (it erases the board's saved settings) or
> > > `S3-APP.bin` (it stops the board booting).
>
> > ### 2.3 Save the image outside the repository
> >
> > The build folder is overwritten by every build, stock or fork, so
> > the image is copied to its own folder under a name that says what it
> > is. Run all six lines; the last one brings you back to the
> > repository root:
> >
> > ```
> > mkdir -p ~/unor4wifi-images
> > cp UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/S3-BOOT-APP.bin ~/unor4wifi-images/UNOR4-WIFI-S3-0.6.0-interrupt.1.bin
> > cd ~/unor4wifi-images
> > sha256sum UNOR4-WIFI-S3-0.6.0-interrupt.1.bin > UNOR4-WIFI-S3-0.6.0-interrupt.1.bin.sha256
> > cat UNOR4-WIFI-S3-0.6.0-interrupt.1.bin.sha256
> > cd ~/git/unor4-wifi-interrupt-bridge
> > ```
> >
> > - ✅ `cat` prints one line: 64 letters and digits, two spaces, then
> >   `UNOR4-WIFI-S3-0.6.0-interrupt.1.bin`. When this procedure was
> >   tested it was
> >   `603a3b8477ba41a628190cb2ff9e8d1089eef67bbde7d607a0bb8309dcbf00cb`;
> >   a different value is normal if your repository is in a different
> >   folder, because build paths are compiled into the firmware.
> > - ❌ `cp` says `No such file or directory` → 2.2 did not pass.
> >   Repeat from 2.1.
> >
> > The `.sha256` file lets the flash procedure prove the image was not
> > changed or swapped after this check.

> ## Done
>
> The fork's image is saved as
> `~/unor4wifi-images/UNOR4-WIFI-S3-0.6.0-interrupt.1.bin`, with its
> checksum next to it. To put it on the board, follow
> [flash-interrupt-procedure.md](flash-interrupt-procedure.md).
