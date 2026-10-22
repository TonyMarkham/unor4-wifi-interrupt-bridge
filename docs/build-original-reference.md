# UNO R4 WiFi bridge firmware v0.6.0 — reference

Background for [build-original-procedure.md](build-original-procedure.md):
why each step is the way it is, where every number comes from, and
what was verified. **There is nothing to run in this document.** To
build or flash, follow the procedure.

## Contents

1. What is built
2. How this document set was verified
3. Repository layout — why no board-manager install
4. The toolchain download
5. Build options and the real size limit
6. The flashable images
7. Can this brick the board?
8. USB identities and the shared connector
9. Download mode
10. The backup
11. Flashing: why option A, and option B for development
12. Release packaging and switching versions
13. Known defects in the upstream repository
14. Sources

> ## 1. What is built
>
> The **stock bridge firmware v0.6.0** (commit `ac27b7b`, "Publish
> 0.6.0") for the ESP32-S3 module on the Arduino UNO R4 WiFi — the chip
> that handles USB for the board. This fork
> (<https://github.com/TonyMarkham/unor4-wifi-interrupt-bridge>, forked
> from <https://github.com/arduino/uno-r4-wifi-usb-bridge>) applies no
> source modifications; the procedure compiles the tree exactly as
> checked out.
>
> - Hardware platform: <https://github.com/arduino/arduino-esp32>
>   branch `unor4wifi-2.0.9` (framework 2.0.9 per its `package.json`).
> - Toolchain: `xtensa-esp32s3-elf` GCC 8.4.0 (`esp-2021r2-patch5`),
>   downloaded by the platform's own `tools/get.py`.
> - Module: ESP32-S3-MINI-1-N8 (8 MB flash), per Arduino's full pinout.
> - License: the sketch and platform are GPL-2.0 / LGPL-2.1 works of
>   the Arduino and Espressif projects; keep license notices intact
>   when redistributing binaries.

> ## 2. How this document set was verified
>
> - **2026-10-21, review.** Every claim and command was checked against
>   the repository scripts, the platform's `boards.txt` and
>   `platform.txt`, an actual build, and Arduino's restore article.
>   Defects in the original draft that were corrected: esptool flash
>   mode `qio` (the build is `dio`), flashing `S3-ALL.bin` (erases NVS)
>   instead of `S3-BOOT-APP.bin`, the size limit (1671168 is not the
>   real one), `pip install esptool` (refused on Ubuntu 23.04+), port
>   checks via `arduino-cli board list` (it does not show VID/PID), and
>   no backup step.
> - **2026-10-22, rehearsal.** Steps 1–4 were run exactly as written,
>   in order, in a fresh clone (Ubuntu 26.04, Python 3.14, arduino-cli
>   1.5.1), and every ✅ check passed. The resulting `S3-BOOT-APP.bin`
>   was checked piece by piece: each part matches its build output
>   byte-for-byte at its partition offset, the embedded partition table
>   matches `unor4wifi.csv`, and both image headers are DIO. The
>   step-5 commands that do not need the board were run too (the
>   Fix 5.0-A core install and compile, the reboot helper). esptool's
>   expected output lines were taken from the esptool 4.5.1 source.
> - **Not verified:** every command that talks to the board (5.0
>   onward, and the upload in Fix 5.0-A). That is why step 5 starts
>   with a backup and stops at every failed ✅.

> ## 3. Repository layout — why no board-manager install
>
> `compile.sh` generates `arduino-cli.yaml` from
> `arduino-cli.yaml.orig`, with `directories.user` set to the current
> directory (`$PWD`). That makes the repository root the arduino-cli
> *sketchbook*, so three things resolve without installing anything
> into `~/.arduino15`:
>
> - `hardware/esp32-patched/esp32` — the platform (vendor
>   `esp32-patched`, architecture `esp32`), providing the FQBN
>   `esp32-patched:esp32:arduino_unor4wifi_usb_bridge`. It is a fork of
>   arduino-esp32 maintained by Arduino that adds this board and its
>   `unor4wifi` partition scheme to the 2.0.9 framework.
> - `libraries/ArduinoBLE`, `libraries/ArduinoHttpClient`,
>   `libraries/Arduino_DebugUtils`, `libraries/Arduino_ESP32_OTA`,
>   `libraries/BOSSA` — the sketch's libraries, as submodules.
> - `UNOR4USBBridge/` — the sketch itself.
>
> Hence "run from the repository root": `compile.sh` bakes `$PWD` into
> the yaml, and anywhere else breaks platform and library discovery.
> The seventh submodule, `unor4wifi-reboot/hid`, belongs to the reboot
> helper's Go source.
>
> arduino-cli 1.x does **not** read an `arduino-cli.yaml` from the
> current directory (checked with 1.5.1); `compile.sh` passes it with
> `--config-file`. Every other `arduino-cli` command in the procedure
> therefore uses the normal `~/.arduino15` setup — which is why
> Fix 5.0-A's `arduino:renesas_uno` install goes there.
>
> `--no-single-branch` in step 1 is the upstream README's canonical
> invocation; it keeps all remote branches reachable in each shallow
> checkout.

> ## 4. The toolchain download
>
> `get.py` reads `../package/package_esp32_index.template.json`,
> selects the entries for the host (`x86_64-pc-linux-gnu`), and
> downloads **every** tool listed, not just the one this firmware
> needs: four toolchains (`xtensa-esp32-elf`, `xtensa-esp32s2-elf`,
> `xtensa-esp32s3-elf`, `riscv32-esp-elf`), two `gdb` builds,
> `openocd-esp32`, `esptool`, `mklittlefs` and `mkspiffs` — 457 MB of
> archives into `tools/dist/`, 2.3 GB once extracted into `tools/`
> (measured). There is no supported selective download.
>
> - It disables TLS certificate verification (`CERT_NONE`) for its
>   downloads.
> - It does not checksum an archive that already exists in
>   `tools/dist/`; it skips the download (`Tool … already downloaded`)
>   and unpacks it again. A truncated archive therefore has to be
>   deleted by hand (procedure Fix 2.1).
>
> The esptool it unpacks is v4.5.1 — the version the platform's own
> upload recipe runs. It needs `pyserial` (Ubuntu package
> `python3-serial`). On Python 3.14 it prints
> `SyntaxWarning: 'return' in a 'finally' block` (`esptool/cmds.py`
> line 152) every time it starts; the code still runs correctly.

> ## 5. Build options and the real size limit
>
> `compile.sh` compiles with `-e` (export binaries) and this FQBN:
>
> | Option | Value | What it means |
> |---|---|---|
> | `PartitionScheme` | `unor4wifi` | The board's own layout (`tools/partitions/unor4wifi.csv`) with the two-slot (app0/app1) OTA table the firmware uses. |
> | `FlashSize` | `4M` | The partition layout spans 4 MB. The physical chip has 8 MB; the firmware uses the first 4 MB. |
> | `FlashMode` | `qio` | Despite the name, this menu entry sets `build.flash_mode=dio` and `build.boot=qio` (`boards.txt` lines 1146–1147): the image headers are **DIO**; only the second-stage bootloader is the `bootloader_qio_80m.elf` build, which switches the flash to QIO itself. Confirmed on the built images: both headers say DIO. |
> | `CPUFreq` | `240` | 240 MHz CPU clock. |
> | `LoopCore` / `EventsCore` | `1` / `1` | Both the Arduino loop task and the events task on core 1. |
> | `PSRAM` | `disabled` | No PSRAM. |
> | `USBMode` | `default` | **Not** the board's base value: the menu entry named `default` is "USB-OTG (TinyUSB)" (`build.usb_mode=0`), whereas the board's base is hardware CDC (`usb_mode=1`). TinyUSB is required for the firmware's composite CDC + HID (CMSIS-DAP) device. |
> | `CDCOnBoot` / `MSCOnBoot` / `DFUOnBoot` / `UploadMode` / `JTAGAdapter` / `DebugLevel` / `EraseFlash` | `default` ×5 / `none` / `none` | Disabled / UART0 upload / no JTAG / no core debug output / no erase. |
>
> **The real size limit is 1638400 bytes**, the size of the app0
> partition (`0x190000`). `boards.txt` declares
> `upload.maximum_size=1671168` (`0x198000`), 32 KB larger, and that is
> the figure arduino-cli's `Sketch uses … (70%)` line measures against.
> An app between 1638401 and 1671168 bytes builds without complaint,
> but `combine.py` then writes it past the end of app0 and
> `S3-BOOT-APP.bin` cuts it off at `0x1E0000` — a truncated image the
> bootloader refuses to boot. Hence the `stat` check in procedure 3.3.
>
> The stock app is about 1.18 MB. The exact size varies by a few
> hundred bytes with the checkout path, because build paths are
> compiled into the firmware (1179856 in one checkout, 1180480 in
> another).
>
> arduino-cli compiles in its own cache (`~/.cache/arduino/sketches/<hash>/`)
> and only *exports* to `UNOR4USBBridge/build`. Deleting the export
> folder does not force a rebuild; deleting the cache does.

> ## 6. The flashable images
>
> `export.sh` deletes `UNOR4USBBridge/build`, re-runs `compile.sh`,
> builds the TLS certificate bundle
> (`python certificates/gen_crt_bundle.py -i certificates/cacrt_all.pem`,
> 151 certificates, 68088 bytes), and runs `python combine.py`, which
> assembles a 4 MB image at fixed offsets, filling gaps with `0xff`:
>
> | Offset | Size | Contents |
> |---|---|---|
> | `0x000000` | `0x8000` | bootloader |
> | `0x008000` | `0x1000` | partition table |
> | `0x009000` | `0x2000` | `boot/boot_app0.bin` (otadata) |
> | `0x00B000` | `0x45000` | certificate bundle |
> | `0x050000` | `0x190000` | app0 — the firmware |
> | `0x1E0000` | `0x190000` | app1 — OTA slot, left empty |
> | `0x370000` | `0x80000` | `spiffs/spiffs.bin` (prebuilt, committed) |
> | `0x3F0000` | `0x10000` | nvs — left erased |
>
> `combine.py` does no size checking (see section 5).
> `boot/boot_app0.bin` is byte-identical to the platform's
> `tools/partitions/boot_app0.bin`.
>
> It writes three files:
>
> - `S3-BOOT-APP.bin` (`0x0`–`0x1E0000`, 1966080 bytes): bootloader,
>   partition table, otadata, certificate bundle, app. **The file to
>   flash.** It is exactly what Arduino's updater zips ship as
>   `firmware/UNOR4-WIFI-S3-<version>.bin` (`unor4wifi-updater/package.sh`)
>   and what Arduino's restore article flashes at `0x0`.
> - `S3-ALL.bin` (4194304 bytes): the whole table. Flashing it also
>   overwrites app1 and SPIFFS and **erases NVS** — the firmware's
>   saved preferences, including WiFi credentials (`cmds_preferences.h`).
>   Arduino's flow never writes it.
> - `S3-APP.bin` (`0xE000`–`0x1E0000`, 1908736 bytes). `0xE000` is the
>   *default* scheme's otadata offset left over in `combine.py`; in the
>   `unor4wifi` layout it lands mid-certificate partition. Nothing in
>   the repository flashes it. Written at `0x0` it overwrites the
>   bootloader and the ESP32 stops booting until reflashed from
>   download mode.
>
> `export.sh` has no error handling: it prints `Done` even when the
> certificate or combine step failed. The file sizes are the real
> check. It also prints one harmless
> `CryptographyDeprecationWarning` (a certificate in Arduino's bundle
> has a non-positive serial number; it is still added).

> ## 7. Can this brick the board?
>
> **Verified facts:**
>
> - The ESP32-S3's first boot stage is in **mask ROM** — burned into
>   the silicon, not stored in flash (ESP32-S3 Technical Reference
>   Manual, boot chapter). It runs regardless of flash contents, and no
>   flashing command in the procedure can overwrite it.
> - Entering download mode by shorting **GND**↔**Download** at
>   power-up is **Arduino's own published recovery procedure**. It does
>   not depend on flash contents.
> - Every flashing command in the procedure addresses the ESP32-S3 in
>   download mode only. The Renesas RA4M1 is a separate chip with
>   separate flash, and only Fix 5.0-A writes to it (an empty sketch,
>   with Arduino's own tools).
> - No command burns eFuses. esptool is used only for `version`,
>   `flash_id`, `read_flash` and `write_flash`; espflash only for
>   `write-bin`. Nothing in the flow is irreversible by design.
> - **The USB-C connector is shared between the two chips.** By
>   default it is wired to the ESP32-S3; analog switches (U2/U6) hand
>   it to the RA4M1's native USB when the RA4M1 drives pin P408 (D40)
>   high, and bridging solder jumper SJ1 hands it over permanently
>   (datasheet ABX00087 §6.2). While the RA4M1 holds it, the computer
>   cannot see the ESP32-S3 at all.
> - The ESP header carries 3.3 V signals only; the ESP32-S3 is not 5 V
>   tolerant (Arduino full pinout, page 1). The ICSP header's pin 2 is
>   +5 V.
>
> **Engineering reasoning, not a tested guarantee:**
>
> - Every bad-flash outcome in this flow reduces to "the ESP32 firmware
>   doesn't boot", recovered by re-entering download mode with the
>   jumper and flashing again or restoring the backup. This follows
>   from the boot architecture above but has not been reproduced.
> - The jumper controls the ESP32's boot strap, not the USB switch. If
>   an RA4M1 sketch takes the connector at power-up, the ESP32 sits in
>   download mode where the computer cannot reach it. Arduino's restore
>   article does not mention this; procedure 5.0 checks for it first.
>
> The real risk is not unrecoverable damage but flashing something
> that *builds fine and boots wrong*. The defenses: the full backup
> (5.4); flashing the same file with the same command Arduino's
> updater uses (5.5); and the app-size check (3.3), which catches the
> one way `combine.py` can silently produce a truncated image.
>
> If the board ends up in a state the procedure does not anticipate,
> follow
> [Arduino's official recovery article](https://support.arduino.cc/hc/en-us/articles/16379769332892-Restore-the-USB-connectivity-firmware-on-UNO-R4-WiFi-with-espflash).

> ## 8. USB identities and the shared connector
>
> | `lsusb` ID | What is on the connector |
> |---|---|
> | `2341:1002` | ESP32-S3 running the bridge firmware: composite CDC + HID device, product `UNO WiFi R4 CMSIS-DAP` (`USB.VID`/`USB.PID`/`USB.productName` in `UNOR4USBBridge.ino`). |
> | `303a:1001` | ESP32-S3 in ROM download mode: Espressif's USB JTAG/serial unit. Also the VID/PID the platform's `boards.txt` declares for this board. |
> | `2341:006d` | The RA4M1's own native USB (CDC + DFU runtime + HID), present when a sketch has taken the connector. The `arduino:renesas_uno` 1.6.0 core lists it as an upload port for the UNO R4 WiFi (`unor4wifi.upload_port.1.pid=0x006D`). |
>
> Only the VID:PID tells these apart, which is why the procedure uses
> `lsusb`. `arduino-cli board list` does not print VID/PID, and
> `/dev/ttyACM0` looks the same in every mode.
>
> `bcdDevice` encodes the bridge firmware version from 0.6.0 onward
> (`USB.firmwareVersion`, so `0.06` = 0.6.0). Earlier releases did not
> set it and show TinyUSB's default `1.00`.

> ## 9. Download mode
>
> **Method A — by command.** The running bridge firmware listens on its
> HID interface for a feature report whose first byte is `0xAA`; when
> it arrives, it reboots the ESP32-S3 into download mode
> (`_onSetFeature` → `usb_persist_restart(RESTART_BOOTLOADER)`,
> `UNOR4USBBridge/DAP.h` line 72). `unor4wifi-reboot` sends that report
> to `2341:1002` (hard-coded in `unor4wifi-reboot/main.go`). The
> prebuilt binary is the one Arduino's updater zips download
> (`bcmi-labs/uno-r4-wifi-usb-bridge` release 0.1.0); it is a static
> binary and runs on Ubuntu 26.04. It needs `sudo` because
> `/dev/hidraw*` is `root:root 0600` on stock Ubuntu. If it can see the
> device but not open it, it crashes with a nil-pointer panic (it
> ignores the open error, `main.go` line 53). Arduino's
> `update_linux.sh` waits one second after it before flashing. Method
> A cannot work if the ESP32 has no working firmware, or while the
> RA4M1 holds the connector.
>
> **Method B — by jumper.** The ESP header (J6) pins, per Arduino's
> full pinout and datasheet §6.1: 1 GPIO42, 2 GPIO41, 3 TXD0,
> 4 DOWNLOAD (GPIO0), 5 RXD0, 6 GND — no power pin. DOWNLOAD is the
> ESP32-S3's boot-mode strap: held low at power-up, the ROM bootloader
> runs instead of the flash firmware. The strap is only read at
> power-up, so the jumper must be on before USB is connected; Arduino's
> article allows releasing it once the board is connected.
>
> ![UNO R4 WiFi: jumper on ESP header pins 6 (GND) and 4 (DOWNLOAD); ICSP header marked as wrong](img/unor4wifi-esp-download-jumper.png)
>
> The image is Arduino's UNO R4 WiFi Full Pinout (ABX00087), page 1,
> © Arduino, CC BY-SA 4.0 — cropped and annotated; the annotated image
> is shared under the same license. Pin positions are Arduino's own
> numbering.

> ## 10. The backup
>
> The backup reads the whole 8 MB chip, so it includes the NVS
> partition (saved preferences and WiFi credentials) and whatever
> firmware was installed. Arduino's restore zip gives you *a* working
> board; the backup gives you back *your* board.
>
> - `--after no_reset` keeps the chip in download mode between
>   commands. esptool 4.5.1 also returns the chip from its RAM stub to
>   the ROM bootloader on exit (`soft_reset(True)` in
>   `esptool/__init__.py`), so the next tool connects to the ROM as
>   usual.
> - The first byte of a valid image is `0xE9`, the ESP image magic of
>   the bootloader at offset 0.
> - Restoring with `write_flash 0x0 <backup>` writes it byte-for-byte:
>   esptool 4.5.1's `write_flash` defaults to `--flash_mode keep` and
>   `--flash_size keep`, and verifies the written data by hash
>   (`Hash of data verified.`).

> ## 11. Flashing: why option A, and option B for development
>
> **Option A (the procedure's 5.5)** is Arduino's own restore command,
> `espflash write-bin -b 115200 0x0 …`, with the port and file filled
> in: the same file (`S3-BOOT-APP.bin`), the same tool and version
> (espflash 2.0.0, pinned by `unor4wifi-updater/package.sh`), the same
> span (`0x0`–`0x1E0000`). SPIFFS and NVS are untouched, so saved
> preferences survive. The README's `S3.bin` is an older name for this
> image.
>
> **Option B — separate binaries via esptool.** Not part of the
> procedure. It is for quick rebuild iterations on a board that already
> carries a matching certificate bundle, because it does **not** write
> the certificate bundle or SPIFFS. It is the platform's own upload
> recipe (`platform.txt`, `tools.esptool_py.upload.pattern_args`) with
> this board's values; run from the repository root, in download mode:
>
> > [!WARNING]
> > Do not change `--flash_mode dio` to `qio` — esptool writes this
> > value into the bootloader header at `0x0`. Do not substitute the
> > README's offsets (`0xe000` / `0x10000`): they are the *default*
> > partition scheme's, and the app would land where the bootloader is
> > not looking.
>
> ```
> python3 hardware/esp32-patched/esp32/tools/esptool/esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bootloader.bin 0x8000 UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.partitions.bin 0x9000 boot/boot_app0.bin 0x50000 UNOR4USBBridge/build/esp32-patched.esp32.arduino_unor4wifi_usb_bridge/UNOR4USBBridge.ino.bin
> ```
>
> The offsets come from three sources that agree:
> `tools/partitions/unor4wifi.csv`, the board's `boot_app0_addr=0x9000`
> and `app0_addr=0x50000` in `boards.txt`, and `combine.py`'s table.
> `--after hard_reset` reboots the ESP32 out of download mode.
>
> **Serial-port scanners.** Arduino's `serial-discovery` 1.5.0 (run in
> the background by the Arduino IDE, and briefly by `arduino-cli`
> commands that list or upload to boards) probes serial devices. While
> it does, `espflash write-bin` to this board can fail with
> `Bad data checksum` and leave the ESP32 soft-bricked
> ([bridge issue #78](https://github.com/arduino/uno-r4-wifi-usb-bridge/issues/78),
> [serial-discovery issue #128](https://github.com/arduino/serial-discovery/issues/128),
> both closed July 2026). serial-discovery 1.5.1 limits probing to
> Arduino's own USB vendor ID, which fixed the reported fault; the
> machine this was written on has 1.5.2 in `~/.arduino15`. The
> procedure still requires the IDE closed and checks
> `pgrep -a serial-discovery`, because the IDE runs a scanner
> continuously and which version it runs was not checked here.

> OTA updates (app1) and the `arduino-cli` upload path (`download.sh`)
> are not covered.

> ## 12. Release packaging and switching versions
>
> `package.sh` builds what a release ships: a release zip, a debug
> zip, and the three `unor4wifi-update-*` updater zips (which download
> espflash 2.0.0 and the prebuilt reboot helpers with `wget`, and wrap
> `S3-BOOT-APP.bin` as the firmware payload). The version is
> hard-coded on its first line (`VERSION="0.6.0"`); it takes no
> arguments. The inner `unor4wifi-updater/package.sh` does take the
> version, and the top-level script passes it.
>
> > [!WARNING]
> > For the debug build, `package.sh` edits `compile.sh`
> > (`DebugLevel=verbose`) and `UNOR4USBBridge/UNOR4USBBridge.ino`
> > (`DBG_ERROR` → `DBG_VERBOSE`) in place and does not put them back.
> > Until they are restored with
> > `git checkout -- compile.sh UNOR4USBBridge/UNOR4USBBridge.ino`,
> > every build is a verbose debug build.
>
> To build another version: check out its tag, run
> `git submodule update --init --depth 1 --no-single-branch` (submodule
> pins travel with the tag), re-run `get.py` if the platform branch in
> `.gitmodules` changed, delete `UNOR4USBBridge/build`, and repeat
> procedure steps 3–4. `package.sh`'s `VERSION` line only names the
> release folder and zips; the firmware version comes from the source.

> ## 13. Known defects in the upstream repository
>
> Found during the review; all are upstream Arduino code. The procedure
> works around each one.
>
> | Defect | Effect | Workaround in the procedure |
> |---|---|---|
> | README's esptool command uses offsets `0xe000`/`0x10000` (default scheme) | Firmware lands where the bootloader does not look | Never used; option A only |
> | `boards.txt` `upload.maximum_size` (1671168) > app0 partition (1638400) | Oversized app passes arduino-cli's check | `stat` check, 3.3 |
> | `combine.py` does no size checking | Oversized app silently truncated in `S3-BOOT-APP.bin` | `stat` check, 3.3 |
> | `export.sh` has no error handling | Prints `Done` after failures | File-size check, 4.2 |
> | `combine.py` writes `S3-APP.bin` from stale offset `0xE000` | Useless and dangerous file | Never flashed; warned in 4.2 |
> | `unor4wifi-reboot` ignores the HID open error | Nil-pointer panic instead of a message | Run with `sudo`; Fix 5.2.2 |
> | `package.sh` edits tracked files and does not restore them | Later builds are debug builds | Not used; warned in section 12 |

> ## 14. Sources
>
> - Arduino UNO R4 WiFi datasheet, ABX00087 — §6.1 ESP header, §6.2 USB
>   bridge: <https://docs.arduino.cc/resources/datasheets/ABX00087-datasheet.pdf>
> - Arduino UNO R4 WiFi full pinout, ABX00087 (CC BY-SA 4.0):
>   <https://docs.arduino.cc/resources/pinouts/ABX00087-full-pinout.pdf>
> - Arduino Help Center, "Restore the USB connectivity firmware on UNO
>   R4 WiFi with espflash":
>   <https://support.arduino.cc/hc/en-us/articles/16379769332892-Restore-the-USB-connectivity-firmware-on-UNO-R4-WiFi-with-espflash>
> - esptool v4.5.1 source: <https://github.com/espressif/esptool/tree/v4.5.1>
> - This repository: `compile.sh`, `export.sh`, `combine.py`,
>   `package.sh`, `unor4wifi-updater/`, `unor4wifi-reboot/main.go`,
>   `UNOR4USBBridge/`, and the platform's `boards.txt`, `platform.txt`
>   and `tools/partitions/unor4wifi.csv`.
