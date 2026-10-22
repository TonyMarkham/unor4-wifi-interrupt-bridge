# Roadmap — UNO R4 WiFi interrupt bridge

The plan for this fork, from a verified stock build to a released
interrupt-capable bridge. Each phase ends in something checkable, and
nothing is published until its phase's "done when" list is all ticked.

**Last updated:** 2026-10-22

**Current version:** this file is kept up to date on the
`feature/interrupt` branch
([view it there](https://github.com/TonyMarkham/unor4-wifi-interrupt-bridge/blob/feature/interrupt/docs/ROADMAP.md)).
The copy on `feature/docs` only changes together with the stock docs,
so it can show later phases as not started.

## The goal

The UNO R4 WiFi's RA4M1 should sleep until a UDP datagram arrives, then
act on it. The stock ESP32-S3 bridge cannot do that — it only answers
AT commands, so the RA4M1 has to poll. This fork adds an opt-in **push
mode** to the bridge, while every stock behavior stays unchanged.

To get there safely, the stock firmware is built, flashed and released
first. That release is the known-good baseline: proof the toolchain and
flash path work, and the image to go back to if the fork misbehaves.

## At a glance

| Phase | Deliverable | Status |
|---|---|---|
| 1 | Deterministic build-and-flash procedure for the stock firmware | ✅ done 2026-10-22 |
| 2 | GitHub release of the stock firmware | ✅ done 2026-10-22 |
| 3 | Interrupt (push-mode) support in the bridge | ✅ done 2026-10-22 |
| 4 | Companion procedures for building and flashing the fork | ✅ done 2026-10-22 |
| 5 | GitHub release of the fork | ✅ done 2026-10-22 |

> ## Phase 1 — Stock build-and-flash procedure
>
> **Status:** ✅ done 2026-10-22
>
> **Deliverables**
>
> - [build-original-procedure.md](build-original-procedure.md) — what to
>   do, step by step.
> - [build-original-reference.md](build-original-reference.md) — why,
>   with sources.
>
> **Where it stands**
>
> - Steps P–4 were run exactly as written in a fresh clone and every ✅
>   passed; the resulting `S3-BOOT-APP.bin` was checked byte-for-byte.
> - 2026-10-22: the whole procedure was run on a new UNO R4 WiFi. It
>   showed `2341:006d`, so Fix 5.0-A ran (and worked); the board shipped
>   with bridge firmware older than 0.6.0 (`bcdDevice 1.00`) and now
>   runs this build of 0.6.0 (`bcdDevice 0.06`). Every mismatch found
>   on the way was fixed in the doc.
>
> **Done when**
>
> - [x] The procedure has been followed on the board from P.6 to 5.6.
> - [x] Every ✅ line matches what was actually seen; any mismatch has
>       been fixed in the doc.
> - [x] The step-5.4 backup exists in `~/unor4wifi-backups/`
>       (`esp32s3-20261022-014810.bin`, 8388608 bytes, first byte
>       `e9`).
> - [x] 5.6 shows `UNO WiFi R4 CMSIS-DAP` with `bcdDevice 0.06`.
> - [x] The superseded `build-original.md` is removed from the
>       repository.
> - [x] Branch `feature/docs` created from `main`, and `docs/` and
>       `.gitignore` committed on it.
> - [x] `git diff main feature/docs --stat` lists only `docs/` and
>       `.gitignore`.

> ## Phase 2 — Stock firmware release
>
> **Status:** ✅ done 2026-10-22
>
> **Deliverable:** a GitHub release on this fork containing the stock
> firmware exactly as built and flashed in Phase 1 —
> [`v0.6.0-stock`](https://github.com/TonyMarkham/unor4-wifi-interrupt-bridge/releases/tag/v0.6.0-stock).
>
> **Plan**
>
> - Tag: `v0.6.0-stock`, on `feature/docs`, at `5c498ec` (the commit
>   that contains the committed procedure).
> - Assets:
>   - `UNOR4-WIFI-S3-0.6.0-stock.bin` — the `S3-BOOT-APP.bin` that was
>     flashed and verified in Phase 1, renamed after Arduino's updater
>     naming (`UNOR4-WIFI-S3-<version>.bin`);
>   - a `SHA256SUMS` file for it.
>   - The three separate build `.bin`s are **not** shipped: nothing in
>     the procedure flashes them, and they invite the README's
>     wrong-offset esptool command.
> - Release notes say:
>   - built from this tag with the procedure, and the toolchain versions
>     used (platform 2.0.9, GCC 8.4.0, arduino-cli 1.5.1, esptool
>     4.5.1, espflash 2.0.0);
>   - this is a self-built image of Arduino's 0.6.0 source, **not**
>     Arduino's own binary — it differs by a few hundred bytes because
>     build paths are compiled into the firmware;
>   - how to flash it: link to procedure step 5;
>   - the license note (see "Decisions" below).
>
> **Done when**
>
> - [x] The release text has been reviewed before publishing.
> - [x] The release is published, and the asset's SHA256 matches the
>       file flashed in Phase 1 (`73f33387…e423`; the downloaded asset
>       is byte-for-byte the flashed `S3-BOOT-APP.bin`).

> ## Phase 3 — Interrupt support in the bridge
>
> **Status:** ✅ done 2026-10-22
>
> **Deliverable:** the bridge with push mode (`AT+UDPPUSH`, `+UDPRX:`
> frames).
>
> **Where it stands**
>
> - On `feature/interrupt`: the upstream fix for a dangling socket
>   pointer when `AT+UDPBEGIN` fails, push mode, the fork marker, and
>   the RA4M1 example `examples/UDPPushSleep` (it replaced the first,
>   always-awake test sketch `tests/UDPPushTest`).
> - Fork marker, chosen: version `0.6.101` (patch = 100 + fork
>   revision; must stay below 128 because WiFiS3 reads the version
>   bytes as signed chars, and the version text within 11 characters)
>   **and** the USB product name `UNO WiFi R4 CMSIS-DAP (interrupt)`
>   (OpenOCD finds the debug adapter by `CMSIS-DAP` in that name, so it
>   stays).
> - WiFiS3 check: the library only reports the bridge version; nothing
>   in it changes behavior on it. Its examples compare the version
>   text against `"0.6.0"`, and `"0.6.101"` does not trigger their
>   upgrade message.
> - App size 1180624 bytes; the image builds byte for byte the same
>   from the same checkout folder (SHA256 `603a3b84…00cb`).
>
> **Plan**
>
> - Implement the change on a branch.
> - Mark the firmware as the fork **without leaving Arduino's version
>   number**: major.minor stay equal to the Arduino release it is based
>   on (see "Maintenance model"). The version (`FIRMWARE_MAJOR` /
>   `FIRMWARE_MINOR` / `FIRMWARE_PATCH`, `UNOR4USBBridge/at_handler.h`
>   lines 19–21) is reported three ways:
>   - USB `bcdDevice` — major.minor only, visible to `lsusb`;
>   - the HID get-feature report — all three numbers (`DAP.h`);
>   - `AT+FWVERSION` / `AT+FWVERSION_U32` — readable by the RA4M1
>     sketch.
> - Candidates for the fork marker, chosen in this phase: a patch
>   number Arduino will not reach (e.g. `0.6.101` for interrupt revision
>   1 — visible via HID and `AT+FWVERSION`, not via `lsusb`), and/or a
>   suffix on the USB product name (visible with `lsusb -v`; first check
>   that no Arduino tool matches on that name).
> - Every build passes the procedure's size check (3.3: no larger than
>   1638400 bytes).
>
> **Done when**
>
> - [x] It builds, and passes the size check.
> - [x] Flashed, it passes every stock check (5.6, a sketch upload, WiFi
>       join) — flash procedure 4.2, 5.3, 5.4.
> - [x] The chosen fork marker is visible on the board, and stock
>       firmware does not show it — `(interrupt)` in 4.2.2,
>       `+FWVERSION: 0.6.101` in 5.4.2.
> - [x] The feature check passes: a datagram reaches the RA4M1 with no
>       poll from its side — first `+UDPRX: 0,5|hello\r\n` with the test
>       sketch, then with `examples/UDPPushSleep` (flash procedure 5.5):
>       six datagrams, each woke the sleeping RA4M1, `0` idle wake-ups
>       in between.

> ## Phase 4 — Companion procedure for the fork
>
> **Status:** ✅ done 2026-10-22
>
> **Deliverable:** two procedures in the same format as the stock
> procedure (numbered actions, ✅/❌ outcomes as list items, each fix in
> its own box directly under its ❌ ending with "Then:", status line per
> step, nested blocks), both assuming the stock procedure was done:
>
> - [build-interrupt-procedure.md](build-interrupt-procedure.md) —
>   build the fork's image and save it to `~/unor4wifi-images/`.
> - [flash-interrupt-procedure.md](flash-interrupt-procedure.md) —
>   back up (optional when the board runs stock), flash, and test push
>   mode with `examples/UDPPushSleep`; going back to stock from the
>   `v0.6.0-stock` release.
>
> **Plan**
>
> - Reuse the stock procedure's verified steps; document only what
>   differs (branch or tag to check out, the expected version, the
>   feature check).
> - The backup (5.4) and the Recovery section stay. Restoring the
>   Phase 2 release image is the documented way back to stock.
>
> **Done when**
>
> - [x] The companion procedures have been followed end to end on the
>       board, and every ✅ matches what was seen (2026-10-22; one
>       mismatch found and fixed: 5.5's example IP address was
>       runnable, so it went to the wrong address).

> ## Phase 5 — Fork firmware release
>
> **Status:** ✅ done 2026-10-22
>
> **Deliverable:** a **separate** GitHub release for the fork's binary —
> [`v0.6.0-interrupt.1`](https://github.com/TonyMarkham/unor4-wifi-interrupt-bridge/releases/tag/v0.6.0-interrupt.1).
>
> **Plan**
>
> - Tag: `v0.6.0-interrupt.1`, on `feature/interrupt`, at `7e5cd3d`
>   (the commit with the procedures and `examples/UDPPushSleep`) —
>   Arduino's base version plus the fork's revision; see "Maintenance
>   model".
> - Assets: `UNOR4-WIFI-S3-0.6.0-interrupt.1.bin` (the image flashed and
>   tested in Phase 4) and `SHA256SUMS`, as decided for Phase 2.
> - Release notes: Phase 2's shape, plus what push mode adds, how to
>   tell the fork from stock, the example sketch, and links to both
>   procedures. Kept in
>   [releases/v0.6.0-interrupt.1.md](releases/v0.6.0-interrupt.1.md).
>
> **Done when**
>
> - [x] The release text has been reviewed before publishing.
> - [x] The release is published, and the asset's SHA256 matches the
>       file verified in Phase 4 (`603a3b84…00cb`; the downloaded asset
>       is byte-for-byte the flashed image).

## Fork revisions

Each revision goes through the same steps as Phases 3–5: change,
build-interrupt-procedure.md and flash-interrupt-procedure.md run on
the board, reviewed release notes, release.

| Revision | Version | Change | Status |
|---|---|---|---|
| `v0.6.0-interrupt.1` | 0.6.101 | Push mode, upstream pointer fix, fork marker | Released 2026-10-22 |
| `v0.6.0-interrupt.2` | 0.6.102 | Security: TLS root certificates updated after the eMudhra CA key compromise (Mozilla 2026-10-20 + GlobalSign Root CA + Arduino's two); `examples/TLSCheck` and flash step 6 | Released 2026-10-22 ([notes](releases/v0.6.0-interrupt.2.md); tag at `2f5f632`; the downloaded asset, SHA256 `0f20d1ad…2412`, is byte-for-byte the image tested on the board) |

## Maintenance model

Each Arduino bridge release becomes **two releases of this fork**:

| | Arduino 0.6.0 | Arduino 0.7.0 (future) |
|---|---|---|
| Stock (Arduino's code, built and verified here) | `v0.6.0-stock` | `v0.7.0-stock` |
| Fork (stock + interrupt support) | `v0.6.0-interrupt.1`, `.2`, … | `v0.7.0-interrupt.1`, … |

Branches:

```
Arduino ──▶ main ──▶ feature/docs ──▶ feature/interrupt
            (pure     (+ docs/,         (+ push mode,
             mirror)   .gitignore)       companion doc)
```

| Branch | Contains | Releases tagged here |
|---|---|---|
| `main` | Arduino's code only — a plain mirror, updated with GitHub's "Sync fork" | none |
| `feature/docs` | `main` + `docs/` + `.gitignore`; no firmware changes | stock (`vX.Y.Z-stock`) |
| `feature/interrupt` | `feature/docs` + the push-mode change + the companion doc | fork (`vX.Y.Z-interrupt.N`) |

- Changes always flow left to right; nothing is ever merged back into
  `main`. Both `feature/` branches are permanent.
- `git diff main feature/docs` must only ever show `docs/` and
  `.gitignore` — that is the proof that a stock binary carries no
  changes of this fork.
- The stock procedure is always present when building stock, because
  it lives on the branch being built.
- An upstream PR to Arduino (e.g. the README's wrong esptool offsets)
  branches from `main`.

**Per Arduino release, the work is:** sync `main`; merge `main` into
`feature/docs`; re-run the stock procedure on the board and update
every ✅ that changed (sizes, versions, toolchain); release stock;
merge `feature/docs` into `feature/interrupt`; re-run the companion
procedure on the board; release the fork. The docs are only
trustworthy for the version they were last run against, so each
procedure records that version.

## Decisions

| Decision | Why | Decided |
|---|---|---|
| Two releases (stock and fork), each tagged at the commit that built its binary — not one release with a binary added later | Each binary sits next to its exact source; satisfies the license's corresponding-source expectation (below) | 2026-10-22 |
| The stock release is labelled a self-built image of Arduino's 0.6.0, not Arduino's binary | Build paths are compiled in, so the bytes differ from Arduino's (app 1,179,856 bytes in the released build vs 1,180,064 in Arduino's) | 2026-10-22 |
| Every released binary ships with a SHA256 | Identifies exactly which file was verified | 2026-10-22 |
| Release assets: only the flash image, named `UNOR4-WIFI-S3-<version>-<variant>.bin`, plus `SHA256SUMS` | Follows Arduino's updater name for the same image; the separate build `.bin`s are never flashed in this flow | 2026-10-22 |
| Fork marker: patch number `100 + revision` (`0.6.101` for interrupt.1) **and** the USB product-name suffix `(interrupt)` | The patch is readable by a sketch (`AT+FWVERSION`) and over HID; the name shows in `lsusb -v` with no sketch at all. Limits: patch below 128 (WiFiS3 reads signed chars), version text within 11 characters, `CMSIS-DAP` kept in the name (OpenOCD matches it) | 2026-10-22 |
| Fix upstream's dangling `udps[]` pointer (failed `AT+UDPBEGIN`) in the fork, as its own commit | The new `AT+UDPPUSH` would otherwise be one more way to use freed memory; stock releases stay unchanged | 2026-10-22 |
| The fork's TLS root bundle = Mozilla's current list (curl `cacert.pem`, SHA256 verified) + GlobalSign Root CA + Arduino's two certificates | Removes both paths to eMudhra's compromised intermediates (emSign ECC Root CA - G3, AAA Certificate Services). ESP-IDF 4.4 only checks the issuer of the topmost certificate a server sends, so GlobalSign Root CA (retired by Mozilla for age) must stay or Google, Cloudflare and arduino.cc fail. Verified on the board with `examples/TLSCheck`. Stock keeps Arduino's bundle (branch rule) | 2026-10-22 |
| The fork is identifiable on the board, but keeps Arduino's major.minor | A separate number (e.g. 0.7) would collide with Arduino's own next release; the fork must always read as "based on Arduino X.Y" | 2026-10-22 (revised the same day) |
| One stock + one fork release per Arduino release, tagged `vX.Y.Z-stock` and `vX.Y.Z-interrupt.N` | Keeps each pair tied to the Arduino version it is built from | 2026-10-22 (tag names proposed) |
| Branches: `main` (pure Arduino mirror) → `feature/docs` (+ docs) → `feature/interrupt` (+ push mode); stock releases on `feature/docs`, fork releases on `feature/interrupt` | `main` stays syncable and PR-able; docs are always present when building; `git diff main feature/docs` proves stock is unmodified | 2026-10-22 |
| Flashing uses `S3-BOOT-APP.bin` with espflash 2.0.0 — Arduino's own flow | Proven path; leaves NVS (saved settings) intact | 2026-10-22 |
| Procedures follow the format in `build-original-procedure.md` | Confirmed readable; see that file's "How to use this procedure" | 2026-10-22 |

**Licensing.** The bridge's AT server (`chAT.hpp`, `parser.cpp`,
`server.cpp`, Copyright Reimu NotMoe / SudoMaker) carries the GNU Affero
General Public License v3-or-later header, and the ESP32 core is
LGPL-2.1. Arduino's own bridge files (`UNOR4USBBridge.ino`,
`at_handler.*`, `cmds_*.h`) carry no license header, and the repository
has no top-level license file. A modified bridge is a derivative of the
AGPL code. Distributing a binary carries an expectation to provide its
corresponding source, which a tag per release does. This is a reading
of the license headers, not legal advice.

## Open questions

| Question | Settle by | Phase |
|---|---|---|
| Byte-for-byte reproducible builds on any machine (strip build paths with a compiler flag) — wanted? Today the image is identical only when built from the same checkout folder. | It is a source change, so it belongs to the fork, not the stock release | next fork revision |

## Ground rules

- Nothing is published to GitHub without a reviewed draft first.
- Every procedure step that touches the board is run on hardware before
  its doc is called done.
- The stock release image and the Phase 1 backup are the two ways back;
  both stay available throughout Phases 3–5.
