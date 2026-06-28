# Changelog

All notable changes to hallamp will be documented here.

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Versioning: [Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### Added
- `flake.nix` — Nix dev shell with Qt 6, CMake, Clang, and full audio codec stack (mpg123, libFLAC, libvorbis, libopenmpt, libsndfile, lame)
- `.envrc` — direnv `use flake` for automatic shell activation
- `CLAUDE.md` — contributor context: branch rules, PR policy, migration goals, project layout, Claude Code tools
- `TODO.md` — phased migration roadmap (Qt5→Qt6, Windows→Linux-native, 5 phases)
- `.mcp.json` — shared MCP server config: context7, GitHub, fetch, git, nixmcp, sequential-thinking
- `.claude/settings.json` — automation hooks:
  - PostToolUse: `clang-format` on C/C++ edits, `nixpkgs-fmt` on `.nix` edits, `nix flake check` after `flake.nix` changes, warnings for `#include <windows.h>` and unsafe C string functions
  - PreToolUse: block edits to `flake.lock` and `.vcxproj`/`.sln` files
- `.claude/agents/winapi-auditor.md` — agent for Win32→Linux porting audits
- `.claude/agents/codec-security-reviewer.md` — security review agent for codec/parser code
- `.claude/skills/qt-port/SKILL.md` — `/qt-port` skill for Qt5→Qt6 component migration
- `flake.lock` — pinned nixpkgs revision for reproducible builds
- `CHANGELOG.md` — this file
- Updated `README.md` — reframed for the Linux port, legacy Windows build instructions collapsed
- `.clang-format` (LLVM style) and a GitHub Actions `nix flake check` workflow
- **CMake build system** — root + per-target `CMakeLists.txt`; the following now build on
  Linux under `nix develop` (GCC 15, Qt 6.11, OpenSSL 3.x):
  - `libwac_network.so` — first Linux artifact; 23 TUs, zero Windows DLLs, exports
    `GetWinamp5SystemComponent`
  - `libpfc.a` — portable file components (all 4 TUs)
  - `libnu.a` — Nullsoft utility lib, portable subset (8 TUs: buffers, sort, regexp,
    `ThreadQueue`, `ServiceWatcher`)
- `wac_network_smoketest` — live network harness (also CTest `wac_network_smoke`) that
  resolves DNS, opens TCP, and completes real HTTP **and** HTTPS GETs end to end
- `nu` now also builds `strsafe.c` and `trace.cpp`; the wide `StringCch*W`/`StringCb*W`
  `printf` family is ported to Linux (delegates to native-`wchar_t` `vswprintf`)
- `nu_strsafe_test` — CTest `nu_strsafe` asserting the strsafe bounded-string contracts
  (truncation reported + destination null-terminated + correct `HRESULT`) for the ANSI
  and wide paths, and driving `nu/trace.cpp` `DebugPrintfA/W` end to end
- `OutputDebugStringA`/`OutputDebugStringW` in the Wasabi Linux shim
  (`bfc/platform/linux.h`, `linux/linux.cpp`)
- `cliff.toml` — git-cliff config; conventional-commit types map to the curated
  Added/Changed/Fixed headings. `git-cliff` added to the Nix devshell
- `Src/replicant/foundation/linux-amd64/types.h` — was missing entirely
- Linux platform shim (`Src/Wasabi/bfc/platform/linux.h`) gained `WCHAR`, `__fastcall`,
  a recursive `CRITICAL_SECTION`, and `HRESULT` as `long`

### Changed
- Default branch for active development: `hallamp` (forked from `community`)
- `pfc` is now consumed via the `Src/` include root (include as `"pfc/..."`); its header is
  named `string.h` and shadowed the standard `<string.h>` when `Src/pfc` was on the path
- Ported files adopt LLVM `clang-format` formatting wholesale (auto-format hook)

### Fixed
- `strsafe.h`'s wide `printf` workers were `#ifdef _WIN32` only (a literal
  `// TODO: benski> port to BSD` calling MSVC's `_vsnwprintf`), so any non-Windows
  caller of `StringCchPrintfW` & friends failed to link; they now compile via
  `vswprintf`. Also made `strsafe.h` self-contained on Linux (`__declspec` was only
  neutralized by the Wasabi shim, breaking the standalone `strsafe.c`)
- TLS handshakes now send **SNI** (`SSL_set_tlsext_host_name`); the 2007-era SSL path
  omitted it, so modern HTTPS servers aborted the connection with a fatal alert
- `wac_network` now compiles `RingBuffer.cpp` — it was an undefined symbol the shared
  library silently tolerated (an executable does not)
- `pfc::critical_section` uses a recursive mutex on POSIX, matching the re-entrant Win32
  `CRITICAL_SECTION` (prevents self-deadlock)
- `pfc/ptr_list.h::insert_item` passed its arguments in the wrong order (caught by GCC 15
  `-Wtemplate-body`)
- `wac_network` async-DNS worker no longer joins a stale `pthread_t` after thread exit or
  failed creation

---

## [Pre-fork baseline] — 2024

Original Winamp source release by Nullsoft/Winamp under the Winamp Collaborative License (WCL) v1.0. Windows-only, Visual Studio 2019, Qt 5.12, DirectX 9.

See upstream history at [alexfreud/winamp](https://github.com/alexfreud/winamp).
