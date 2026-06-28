# hallamp — Migration Roadmap

Qt5 (Windows-only, MSVC) → Qt6 (Linux-native, Nix/CMake)

---

## Status snapshot — 2026-06-28

Five green checkpoints on `hallamp` (all committed + pushed). Foundation libs and
the first real component build on Linux under `nix develop`:

| commit | artifact | proof |
|---|---|---|
| `615b88ea` | `libwac_network.so` | compiles 23/23 TUs, zero Windows DLLs, exports `GetWinamp5SystemComponent` |
| `28d3ea74` | `wac_network_smoketest` | **runtime-proven**: live DNS + HTTP + HTTPS GET, exit 0 |
| `4af19b79` | `libpfc.a` | all 4 TUs (cfg_var, string, string_unicode, grow_buf) |
| `be0392ca` | `libnu.a` | 8 portable TUs (buffers, sort, regexp, ThreadQueue, ServiceWatcher) |
| `nu_strsafe` | `libnu.a` (+`strsafe.c` +`trace.cpp`) + `nu_strsafe_test` | **runtime-proven**: `nu_strsafe` asserts copy/printf/vprintfEx truncation + null-term + HRESULT contracts and drives trace.cpp end to end, exit 0 |

### What we've learned (this drives the refactor below)

- **"Builds" ≠ "correct."** A shared lib tolerates undefined symbols (resolved at
  load), so the M1 `.so` linked while missing `RingBuffer`; only the smoke-test
  *executable* exposed it. Plus the 2007 SSL path had no **SNI** → modern TLS
  refused it. Neither was visible at compile time. **Every component needs a
  runtime proof, not just a build.**
- **The platform shim `linux.h` is the real leverage point.** Each bite hardens
  it and pays forward to every later component: `HRESULT long`, `min/max`,
  drop `None`, **`WCHAR`**, **`__fastcall`**, recursive `CRITICAL_SECTION`.
  Treat shim gaps as foundational wins, not detours.
- **GCC 15 `-Wtemplate-body` is a free bug-finder** — it caught `ptr_list`'s
  swapped insert args without instantiation.
- **Survey-by-static-marker is a good *first* cut, not a verdict.** For nu, ~1/3
  of "category A" needed deeper work once compiled (header self-containment,
  transitive `<windows.h>`, dependency stacks). Budget for it.
- **`pfc/string.h` shadows the standard `<string.h>`** — never put `Src/pfc` on
  an include path; expose pfc via `Src/` and include as `"pfc/..."`.
- **Transitive `<windows.h>` hides missing includes.** `trace.cpp` got `realloc`/
  `free` via `<windows.h>` on Win32; on Linux it needed an explicit `<stdlib.h>`.
  And a header that relied on the Wasabi shim for `__declspec`/`__stdcall` broke
  when compiled standalone (`strsafe.c`) — make ported headers self-contained.
- **A wide path is not one decision.** `strsafe`'s `StringCch*W` works on the
  *platform-native* `wchar_t` (32-bit) via `vswprintf` — fine for `trace.cpp` and
  native callers, but the codebase's 16-bit `WCHAR` callers are a separate problem
  (no libc 16-bit wide printf). Port the easy width first, defer the hard one.
- **The original per-component scoping was optimistic.** `wac_downloadManager` is
  NOT "~20 files, one `<windows.h>`": it needs a real Win32→pthreads threading
  port and is entangled with QtWebEngine (corrected in Phase 2 below).

### Milestone ladder (refactored)

| # | Milestone | Proves | State |
|---|---|---|---|
| 1 | `wac_network` builds → `.so` | CMake + Nix + shim *compile* | ✅ |
| 2 | `wac_network` *runs* (live fetch) | shim is runtime-correct | ✅ |
| 3 | Foundation libs: `libpfc.a` + `libnu.a` (portable subsets) | shared utility layer compiles standalone | ✅ |
| 4 | A second component *composes* (links wac_network + nu + pfc) | the pattern repeats across components | 🎯 next |
| 5 | `wac_playlists` builds + runtime-proven | playlist/JNL surface | later |
| 6 | `wac_browser` (Qt6 WebEngine) | the hard Qt5→Qt6 surface | last |

**Milestone 4 — picking the second component.** The lighter prerequisite (the
**`strsafe.h` port**) is now done, so Milestone 4 is `wac_downloadManager` — the
first component that *composes* (links `wac_network` + `nu` + `pfc`). It carries a
Win32→pthreads threading port + WebEngine entanglement (a real bite, scoped in
Phase 2).

**Deferred bites — tracked, each well-scoped:**
- `wac_downloadManager`: Win32→pthreads port (`CreateThread`/`CreateEvent`/
  `CRITICAL_SECTION`/`WaitForSingleObject` → `std::thread`/atomic/`std::mutex`);
  WebEngine UA sync gated until Phase 4; fix backslash includes; guard `WAT.h`.
- **16-bit `WCHAR` wide-strsafe reconciliation.** The ported `StringCch*W` path
  uses native 32-bit `wchar_t` (`vswprintf`); the ~775 tree-wide `StringCchPrintfW`
  callers that pass 16-bit `WCHAR*` need a custom 16-bit formatter or a narrow
  (UTF-8) bridge. Surfaces when a `WCHAR*`-heavy component enters the build.
  (`Src/replicant/nu/strsafe.h` — a separate copy — is untouched and still
  Win-only-ish; reconcile if a consumer of it is ported.)
- `nu/RedBlackTree.cpp` → needs the `bfc::PtrList` container stack ported.
- **Shim dimensions still unverified** (no component has stressed them yet): LP64
  integer widths (`DWORD`/`HRESULT`), SIGPIPE on socket writes, global
  `operator new`/`delete` vs Qt. Flush out via a component that exercises them,
  not a speculative review.

**Decision recorded:** ported files adopt LLVM `clang-format` style wholesale (the
auto-format hook reformats touched files); blame churn is accepted as part of modernization.

---

## Phase 0: Foundation

Legend: `[x]` done · `[~]` partial (only the subset the current milestone needed) · `[ ]` not started

- [x] Fork from community branch (alexfreud/winamp)
- [x] Create `hallamp` branch
- [x] `flake.nix` + `.envrc` (direnv)
- [x] `CLAUDE.md`
- [x] Set default GH repo to `MarkusBitterman/hallamp`
- [x] Commit and lock `flake.lock` (committed in `HEAD`)
- [x] GitHub Actions: `nix flake check` on PRs — `.github/workflows/nix-flake-check.yml`
- [x] Add `.clang-format` config — LLVM style; activates the auto-format hook
- [x] Add `git-cliff` config for changelog generation — `cliff.toml`; conventional
  types map to the curated Added/Changed/Fixed headings; tool in the Nix devshell

---

## Phase 1: Build system migration (Windows → CMake)

Goal: get _something_ building on Linux under Nix. ✅ **achieved (Milestone 1)**

- [x] Root `CMakeLists.txt` — drives `Src/nu` + `Src/pfc` + `Src/Components/wac_network`;
  sets `-DLINUX`, `enable_testing()`
- [~] Port `/Src/nu/` (Nullsoft Utility lib) → `libnu.a` (portable subset)
  - Builds 10 cross-platform TUs: `bitbuffer`, `RingBuffer`, `GaplessRingBuffer`,
    `SpillBuffer`, `sort`, `regexp`, `ThreadQueue`, `ServiceWatcher`, plus
    `strsafe.c` + `trace.cpp` (added with the strsafe.h port)
  - Foundational fix: `linux.h` was missing `#define __fastcall` (had `__cdecl`);
    `SpillBuffer.h` made self-contained for `size_t`
  - **strsafe.h port**: un-gated the wide `printf` path (was `#ifdef _WIN32 //
    TODO: benski> port to BSD`) by aliasing `_vsnwprintf`→`vswprintf` (native
    32-bit `wchar_t`); made `strsafe.h` self-contained (`__declspec`/`__stdcall`
    neutralized in-header); `trace.h` no longer needs Windows `<wtypes.h>`; added
    canonical `OutputDebugStringA/W` to the Wasabi shim. Proven by `nu_strsafe`.
    Still deferred: 16-bit `WCHAR` reconciliation (see deferred bites above).
  - Deferred (each its own bite, documented in `Src/nu/CMakeLists.txt`):
    `RedBlackTree` (needs `bfc::PtrList` stack); `DialogSkinner` + all GUI files
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
  - Done across `wac_network`, `pfc`, `nu` (incl. `strsafe.h`: `__declspec`/`__stdcall`
    now neutralized in-header); tree-wide sweep pending
- [~] Audit and replace Windows-only headers (`<windows.h>`, `<winsock2.h>`, `<wincrypt.h>`, `WAT.h`)
  - Gated across `wac_network`, `pfc`, `nu` (built TUs); GUI/Wasabi + deferred files remain
- [~] Replace `HANDLE`/`HWND` abstractions with platform-agnostic equivalents where feasible
  - `linux.h` now provides: `HRESULT` (long), `WCHAR`/`__fastcall`, recursive
    `CRITICAL_SECTION`, POSIX `HANDLE`/`HWND` stubs. Each component adds what it needs.
  - Unverified on LP64: integer-width types (`DWORD`/`HRESULT`) — see deferred bites

---

## Phase 2: Qt5 → Qt6 (Components layer)

The Qt components in `/Src/Components/` are already somewhat isolated — start here.

- [~] `wac_network`
  - [x] Compiles + links → `libwac_network.so` (Milestone 1)
  - [x] Runtime-proven (Milestone 2: live DNS + HTTP/HTTPS GET smoke test; SNI + RingBuffer fixes)
  - Note: links OpenSSL/Qt6Network; actual Qt `QNetworkAccessManager` usage is minimal —
    the real work is JNL (the bundled Nullsoft socket lib) + the Wasabi BFC dispatch layer
- [ ] `wac_downloadManager` — **rescoped after inspection** (Milestone 4 candidate)
  - Links against `wac_network` + `nu` + `pfc` (all now buildable).
  - Real cost (the original "~20 files, one `<windows.h>`" note was wrong):
    - Win32→pthreads threading port: `CreateThread`/`CreateEvent`/`SetEvent`/
      `WaitForSingleObject`/`CloseHandle` + `CRITICAL_SECTION downloadsCS` drive a
      background download worker. Map to `std::atomic<bool>` killswitch +
      `std::mutex`/`pfc::critical_section` + pthread/`std::thread` join.
    - QtWebEngine entanglement: 2 lines (`wac_downloadManager.cpp:221,223`) sync the
      browser user-agent → gate behind a feature macro until Phase 4 (don't pull in
      Chromium for a UA string).
    - Backslash includes (`..\wac_network\…`, `..\WAT\WAT.h`) + Windows-only `WAT.h`;
      `%S` wide-format in `StringCchPrintfA`. Note `DownloadEx` is currently a stub.
- [ ] `wac_playlists`
  - Depends on wac_network. Survey real scope before committing (downloadManager
    taught us the per-component notes are optimistic).
- [ ] `wac_browser`
  - `QWebEngineView` had significant API changes Qt5→Qt6
  - `QWebEnginePage` script injection model changed
  - Tackle last; highest complexity. Re-enables the downloadManager UA sync above.

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
