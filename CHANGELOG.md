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

### Changed
- Default branch for active development: `hallamp` (forked from `community`)

---

## [Pre-fork baseline] — 2024

Original Winamp source release by Nullsoft/Winamp under the Winamp Collaborative License (WCL) v1.0. Windows-only, Visual Studio 2019, Qt 5.12, DirectX 9.

See upstream history at [alexfreud/winamp](https://github.com/alexfreud/winamp).
