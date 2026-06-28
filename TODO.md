# hallamp — Migration Roadmap

Qt5 (Windows-only, MSVC) → Qt6 (Linux-native, Nix/CMake)

---

## Status snapshot — 2026-06-27

**Milestone 1 ✅ — `wac_network` is the first Linux build artifact.**
`libwac_network.so` compiles (23/23 TUs) and links under `nix develop` against Qt6Core,
Qt6Network, OpenSSL 3.x, and zlib — **zero Windows DLLs**. It exports the real Winamp host
entry point `GetWinamp5SystemComponent`, with `-fvisibility=hidden` keeping everything else
internal. This proves the CMake skeleton, the Nix dev shell, and the platform compat layer.

What it took (the reusable groundwork, not just wac_network-local fixes):
- Root + per-component `CMakeLists.txt`; `add_compile_definitions(LINUX)` (Wasabi keys off
  `LINUX`, not `__linux__`).
- New `Src/replicant/foundation/linux-amd64/types.h` (was missing entirely).
- Platform shim `Src/Wasabi/bfc/platform/linux.h`: `HRESULT` `void*`→`long`; `min`/`max`
  macros replaced with `using std::min/max` under C++ (GCC 15 + STL templates); removed
  `#define None` (collided with Qt's `qcoreevent.h`).
- `Src/pfc/critsec.h` POSIX `pthread_mutex_t` path; `nonewthrow.c` `noexcept` on delete;
  win32 includes/pragmas guarded across `wac_network` sources.

**In flight 🔄** — adversarial multi-agent review of the shared platform shim for *latent*
(compiles-but-wrong) bugs: recursive-mutex deadlock (`CRITICAL_SECTION` is re-entrant, a
default `pthread_mutex` is not), integer widths (`DWORD`/`HRESULT` on LP64), OpenSSL RNG
seeding after dropping `CryptGenRandom`, SIGPIPE on socket writes, and the custom global
`operator new`/`delete` coexisting with Qt. Confirmed findings fold into the Milestone 1
checkpoint before commit.

**Next 🎯 — Milestone 2: "first bytes over the wire."** Prove `wac_network` *runs*, not just
builds — a CMake test target that drives the JNL classes directly to resolve DNS, open a TCP
socket, and complete a live HTTP + HTTPS GET. This exercises every runtime unknown above and
turns the review's findings into a concrete pass/fail gate.

**Decision recorded:** ported files adopt LLVM `clang-format` style wholesale (the
auto-format hook reformats touched files); blame churn is accepted as part of modernization.

Milestone ladder:

| # | Milestone | Proves | State |
|---|---|---|---|
| 1 | `wac_network` builds → `.so` | CMake + Nix + shim *compile* | ✅ |
| 2 | wac_network *runs* (live fetch) | shim is runtime-correct | 🎯 next |
| 3 | `wac_downloadManager` builds + links wac_network | pattern repeats; components compose | scoped |
| 4 | `wac_playlists`, then `wac_browser` (Qt6 WebEngine) | the hard Qt5→Qt6 surface | later |

---

## Phase 0: Foundation

Legend: `[x]` done · `[~]` partial (only the subset the current milestone needed) · `[ ]` not started

- [x] Fork from community branch (alexfreud/winamp)
- [x] Create `hallamp` branch
- [x] `flake.nix` + `.envrc` (direnv)
- [x] `CLAUDE.md`
- [x] Set default GH repo to `MarkusBitterman/hallamp`
- [ ] Commit and lock `flake.lock`
- [x] GitHub Actions: `nix flake check` on PRs — `.github/workflows/nix-flake-check.yml`
- [x] Add `.clang-format` config — LLVM style; activates the auto-format hook
- [ ] Add `git-cliff` config for changelog generation

---

## Phase 1: Build system migration (Windows → CMake)

Goal: get _something_ building on Linux under Nix. ✅ **achieved (Milestone 1)**

- [x] Root `CMakeLists.txt` — drives `Src/pfc` + `Src/Components/wac_network`; sets `-DLINUX`
- [~] Port `/Src/nu/` (Nullsoft Utility lib)
  - Touched only what wac_network pulls in: `nonewthrow.c` (`noexcept`), `threadpool/api_threadpool.h`
    (dropped `<windows.h>`), `strsafe.h` (already Unix-ported upstream)
  - TODO: a standalone `nu` CMake target / full build
- [~] Port `/Src/pfc/` (Portable File Components)
  - Builds: `grow_buf.cpp`; `critsec.h` POSIX path; `pfc.h` win32/NOVTABLE guard
  - Excluded for now: `cfg_var.cpp`, `string*.cpp` — direct `<windows.h>` use, needs porting
- [~] Audit and replace MSVC-specific attributes (`__declspec`, `#pragma intrinsic`, `#pragma comment`)
  - Done across wac_network surface; tree-wide sweep still pending
- [~] Audit and replace Windows-only headers (`<windows.h>`, `<winsock2.h>`, `<wincrypt.h>`, `WAT.h`)
  - Guarded across wac_network; `pfc`/`nu`/Wasabi still have unguarded uses
- [~] Replace `HANDLE`/`HWND` abstractions with platform-agnostic equivalents where feasible
  - `linux.h` provides POSIX-backed `HANDLE`/`CRITICAL_SECTION`/`HWND` stubs (see review caveats in snapshot)

---

## Phase 2: Qt5 → Qt6 (Components layer)

The Qt components in `/Src/Components/` are already somewhat isolated — start here.

- [~] `wac_network`
  - [x] Compiles + links → `libwac_network.so` (Milestone 1)
  - [ ] Runtime-proven (Milestone 2: live DNS + HTTP/HTTPS GET test harness)
  - Note: links OpenSSL/Qt6Network; actual Qt `QNetworkAccessManager` usage is minimal —
    the real work is JNL (the bundled Nullsoft socket lib) + the Wasabi BFC dispatch layer
- [ ] `wac_downloadManager`
  - Depends on wac_network (now buildable). Scoped: ~20 files, one `<windows.h>` include
    (`DownloadCallbackT.h`), no backslash paths. First test of inter-component linking.
- [ ] `wac_playlists`
  - Depends on wac_network
- [ ] `wac_browser`
  - `QWebEngineView` had significant API changes Qt5→Qt6
  - `QWebEnginePage` script injection model changed
  - Tackle last; highest complexity

Qt5→Qt6 migration reference: https://doc.qt.io/qt-6/sourcebreaks.html

---

## Phase 3: Audio subsystem (Linux output)

The codecs (mpg123, FLAC, Vorbis, etc.) are already cross-platform — the output layer is the problem.

- [ ] New plugin: `out_pipewire` (PipeWire output, replaces WASAPI/DirectSound)
  - PipeWire has ALSA/PulseAudio compatibility layers as fallback
- [ ] Verify codec plugins build on Linux:
  - [ ] `in_mp3` (mpg123) — should be straightforward
  - [ ] `in_flac` — libFLAC is cross-platform
  - [ ] `in_vorbis` — libogg/libvorbis are cross-platform
  - [ ] `in_mod` (openmpt) — cross-platform, has Linux CI already
- [ ] Remove or stub WASAPI plugin (`out_wasapi`) on non-Windows

---

## Phase 4: GUI / rendering layer

This is the hardest phase. Wasabi is a custom Win32 UI framework with ~32 subdirectories.

- [ ] Audit Wasabi (`/Src/Wasabi/`) for Win32 surface area
  - Map which APIs are Win32-only vs. abstractable
- [ ] Decide: wrap Wasabi in Qt6 widgets, or replace with Qt Quick (QML)
  - Qt Quick is the modern path; Qt Widgets is lower risk
- [ ] Replace `CreateWindow`/`RegisterClass` Win32 windowing with Qt6
- [ ] Replace DirectX 9 rendering (`D3D9`, `DDraw`, `GDI+`) with Qt RHI (OpenGL/Vulkan backend)
- [ ] Replace Win32 file dialogs, tray icon, hotkeys with Qt equivalents
- [ ] Port visualizers (`vis_avs`, `vis_milk2`) — heavily DirectX/Win32, major effort

---

## Phase 5: Packaging

- [ ] `packages.default` in `flake.nix` — buildable Nix package
- [ ] AppImage target (broad Linux compatibility, no Nix required)
- [ ] Flatpak target (for distribution via Flathub)
- [ ] GitHub Actions: build and attach AppImage to releases

---

## Tracking

Use GitHub Issues with labels:
- `phase/0-foundation`, `phase/1-build`, `phase/2-qt6`, `phase/3-audio`, `phase/4-gui`, `phase/5-packaging`
- `good-first-issue` for self-contained Win32 → cross-platform substitutions
