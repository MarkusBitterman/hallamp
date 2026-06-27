# hallamp — Migration Roadmap

Qt5 (Windows-only, MSVC) → Qt6 (Linux-native, Nix/CMake)

---

## Phase 0: Foundation

- [x] Fork from community branch (alexfreud/winamp)
- [x] Create `hallamp` branch
- [x] `flake.nix` + `.envrc` (direnv)
- [x] `CLAUDE.md`
- [x] Set default GH repo to `MarkusBitterman/hallamp`
- [ ] Commit and lock `flake.lock`
- [ ] GitHub Actions: `nix flake check` on PRs
- [ ] Add `.clang-format` config
- [ ] Add `git-cliff` config for changelog generation

---

## Phase 1: Build system migration (Windows → CMake)

Goal: get _something_ building on Linux under Nix.

- [ ] Root `CMakeLists.txt` — skeleton, no sources yet
- [ ] Port `/Src/nu/` (Nullsoft Utility lib)
  - Minimal Win32 surface, mostly templates and data structures
  - Good smoke test for the CMake setup
- [ ] Port `/Src/pfc/` (Portable File Components)
  - Mostly cross-platform already
- [ ] Audit and replace MSVC-specific attributes (`__declspec`, `#pragma comment(lib, ...)`)
- [ ] Audit and replace Windows-only headers (`<windows.h>`, `<winsock2.h>`, etc.)
- [ ] Replace `HANDLE`/`HWND` abstractions with platform-agnostic equivalents where feasible

---

## Phase 2: Qt5 → Qt6 (Components layer)

The Qt components in `/Src/Components/` are already somewhat isolated — start here.

- [ ] `wac_network`
  - `QNetworkAccessManager` API largely unchanged Qt5→Qt6
  - Lowest friction entry point
- [ ] `wac_downloadManager`
  - Depends on wac_network
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
