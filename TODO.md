# hallamp — Migration Roadmap

Qt5 (Windows-only, MSVC) → Qt6 (Linux-native, Nix/CMake)

---

## Status snapshot — 2026-06-28

**Milestone 2 ✅ — `wac_network` makes live HTTP + HTTPS requests on Linux.**
A standalone smoke-test harness (`wac_network_smoketest`, also a CTest target
`wac_network_smoke`) drives the JNL `WAC_Network_HTTPGet` object directly through
a real DNS resolve → TCP connect → HTTP/1.0 GET, for both `http://` and
`https://example.com/`. Both return `200` with body bytes. Exit 0.

Building/running the harness flushed out **two latent bugs that "it compiles"
could never catch**:
- `Src/nu/RingBuffer.cpp` was never compiled. A shared library *tolerates*
  undefined symbols (resolved at load), so the Milestone 1 `.so` linked while
  secretly incomplete; an executable forbids them, so the smoke test exposed it.
  Now compiled into the object set.
- The 2007-era SSL path never set **SNI**, so modern TLS servers aborted the
  handshake with a fatal alert. Fixed with `SSL_set_tlsext_host_name()` in
  `wac_network_ssl_connection.cpp` (verified against `openssl s_client`).

The CMake now builds the 23 TUs once into an `OBJECT` library that feeds both the
hidden-visibility `.so` and the test executable (static linking ignores ELF
visibility, so internal JNL classes resolve in the test without being exported).

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

**Still unverified (deferred, not blocking) ⚠️** — the platform-shim review only fully
confirmed the concurrency findings (recursive mutex, DNS thread) before it was cut short.
The HTTPS smoke test now exercises OpenSSL RNG seeding live (passes), but these remain
unaudited: integer widths (`DWORD`/`HRESULT` on LP64), SIGPIPE on socket writes (the test
never hit a broken-pipe path), and the custom global `operator new`/`delete` coexisting with
Qt. Revisit when a component stresses them, rather than spending a review pass now.

**Next 🎯 — Milestone 3: `wac_downloadManager` builds and links against `wac_network`.**
First test that the pattern repeats and that components *compose*. Scoped: ~20 files, one
`<windows.h>` include (`DownloadCallbackT.h`), no backslash paths. Reuses the OBJECT-library
+ smoke-test shape proven here.

**Decision recorded:** ported files adopt LLVM `clang-format` style wholesale (the
auto-format hook reformats touched files); blame churn is accepted as part of modernization.

Milestone ladder:

| # | Milestone | Proves | State |
|---|---|---|---|
| 1 | `wac_network` builds → `.so` | CMake + Nix + shim *compile* | ✅ |
| 2 | wac_network *runs* (live fetch) | shim is runtime-correct | ✅ |
| 3 | `wac_downloadManager` builds + links wac_network | pattern repeats; components compose | 🎯 next |
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
- [~] Port `/Src/nu/` (Nullsoft Utility lib) → `libnu.a` (portable subset)
  - Builds 8 cross-platform TUs: `bitbuffer`, `RingBuffer`, `GaplessRingBuffer`,
    `SpillBuffer`, `sort`, `regexp`, `ThreadQueue`, `ServiceWatcher`
  - Foundational fix: `linux.h` was missing `#define __fastcall` (had `__cdecl`);
    `SpillBuffer.h` made self-contained for `size_t`
  - Deferred (each its own bite, documented in `Src/nu/CMakeLists.txt`):
    `RedBlackTree` (needs `bfc::PtrList` stack); `trace`+`strsafe.c` (need a
    `strsafe.h` port — still `__declspec`); `DialogSkinner` + all GUI files
    (header pulls `<windows.h>`) → Phase 4; MSVC CRT stubs (`no*.c`) and the
    global-allocator override `nonewthrow.c` deliberately excluded
- [x] Port `/Src/pfc/` (Portable File Components) → `libpfc.a`
  - All four TUs build: `grow_buf.cpp`, `cfg_var.cpp`, `string.cpp`, `string_unicode.cpp`
  - Win32 INI/registry persistence + GUI text accessors gated behind `#ifdef _WIN32`
    with Linux no-op stubs (config store is redesigned later); `set_string_a/_w` got
    real UTF-8↔UTF-16 Linux bodies (`WCHAR` is 16-bit, not 32-bit `wchar_t`)
  - Foundational fixes flushed out while compiling pfc for the first time:
    - `linux.h` was missing a `WCHAR` typedef (only Apple/Win32 had one) → added (16-bit)
    - `ptr_list.h::insert_item` forwarded arguments in the wrong order (swapped
      `(idx, ptr)` vs base `(ptr, idx)`) — latent bug caught by GCC 15 `-Wtemplate-body`
    - pfc's header is literally named `string.h`; putting `Src/pfc` on the include
      path shadowed the C/C++ standard `<string.h>`/`<cstring>`. Fixed by exposing pfc
      via `Src/` (consumers use `"pfc/..."`) and never `-I Src/pfc`
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
  - [x] Runtime-proven (Milestone 2: live DNS + HTTP/HTTPS GET smoke test; SNI + RingBuffer fixes)
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
