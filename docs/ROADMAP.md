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
| 3 | Interrupt (push-mode) support in the bridge | ⚪ not started |
| 4 | Companion procedure for building and flashing the fork | ⚪ not started |
| 5 | GitHub release of the fork | ⚪ not started |

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
> **Status:** ⚪ not started · **Needs:** Phase 2 done ✅
>
> **Deliverable:** the bridge with push mode (`AT+UDPPUSH`, `+UDPRX:`
> frames).
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
> - [ ] It builds, and passes the size check.
> - [ ] Flashed, it passes every stock check (5.6, a sketch upload, WiFi
>       join).
> - [ ] The chosen fork marker is visible on the board, and stock
>       firmware does not show it.
> - [ ] The feature check passes: a datagram reaches the RA4M1 with no
>       poll from its side. This needs RA4M1-side firmware.

> ## Phase 4 — Companion procedure for the fork
>
> **Status:** ⚪ not started · **Needs:** Phase 3 done
>
> **Deliverable:** `build-interrupt-procedure.md`, plus a reference if
> needed, in the same format as the stock procedure: numbered actions,
> ✅/❌ outcomes as list items, each fix in its own box directly under
> its ❌ (ending with "Then:"), status line per step, nested blocks.
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
> - [ ] The companion has been followed end to end on the board, and
>       every ✅ matches what was seen.

> ## Phase 5 — Fork firmware release
>
> **Status:** ⚪ not started · **Needs:** Phase 4 done
>
> **Deliverable:** a **separate** GitHub release for the fork's binary.
>
> **Plan**
>
> - Tag: `v0.6.0-interrupt.1` (proposed) — Arduino's base version plus
>   the fork's revision; see "Maintenance model".
> - Same asset set and release-note shape as Phase 2, plus what push
>   mode adds and a link to the companion procedure.
>
> **Done when**
>
> - [ ] The release text has been reviewed before publishing.
> - [ ] The release is published, and the asset's SHA256 matches the
>       file verified in Phase 4.

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
| Fork marker: high patch number, USB product-name suffix, or both | Check Arduino tools do not match on the product name; then decide | 3 |
| Byte-for-byte reproducible builds (strip build paths with a compiler flag) — wanted? | It is a source change, so it belongs to the fork, not the stock release | 3 |
| Does `WiFiS3` on the RA4M1 behave differently when the bridge reports a version newer than 0.6.0? | Read the `WiFiS3` library's version check before choosing the number | 3 |

## Ground rules

- Nothing is published to GitHub without a reviewed draft first.
- Every procedure step that touches the board is run on hardware before
  its doc is called done.
- The stock release image and the Phase 1 backup are the two ways back;
  both stay available throughout Phases 3–5.
