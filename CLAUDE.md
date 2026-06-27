# hallamp

Community fork of Winamp, targeting a Qt6 + Linux-native port via Nix.

## Branch strategy

- **`hallamp`** — our working branch and release target. All development goes here.
- **`community`** — frozen upstream fork point. Do not commit to it, push to it, or rebase from it.
- Upstream (`alexfreud/winamp`) is unmaintained. We do not open PRs there, sync from it, or touch it.

## Pull requests

We do **not** open our own PRs on `hallamp`. We review and respond to incoming PRs from contributors.

## GitHub

Default repo: `MarkusBitterman/hallamp`
If `gh` commands target the wrong repo: `gh repo set-default MarkusBitterman/hallamp`

## Commit style

Conventional commits: `feat:`, `fix:`, `chore:`, `refactor:`, `build:`, `docs:`

## Dev environment

```bash
nix develop        # enter the Qt6 / Nix shell
direnv allow       # or let direnv activate via .envrc (use flake)
```

Requires Nix with flakes enabled (`experimental-features = nix-command flakes`).

## Migration goals

Current state: Windows-only, Visual Studio 2019, Qt 5.12 (bundled pre-compiled 7z blobs).
Target: Linux-native, CMake + Nix, Qt 6.

The Qt components in `/Src/Components/` are the primary migration seam:
- `wac_network` — lowest friction, start here
- `wac_downloadManager` — depends on wac_network
- `wac_playlists` — depends on wac_network
- `wac_browser` — QWebEngineView changed significantly Qt5→Qt6, tackle last

See `TODO.md` for the full phased roadmap.

## Project layout (key paths)

| Path | Purpose |
|---|---|
| `/Src/Components/` | Qt-based components — primary migration target |
| `/Src/Winamp/` | Main application |
| `/Src/Plugins/` | Input/output/DSP/vis/library plugins |
| `/Src/Wasabi/` | Custom UI framework (heavy Win32) |
| `/Src/nu/` | Nullsoft Utility lib — good early Linux port candidate |
| `/Src/pfc/` | Portable file components |
| `/Src/external_dependencies/` | CEF, openmpt, cpr, vorbis, theora |
| `flake.nix` | Nix dev shell (Qt6 + build tools + audio libs) |
| `.envrc` | direnv — `use flake` |

## Automation

- **CI**: GitHub Actions — build check on PRs via `nix flake check` (not yet configured)
- **Format**: `nixpkgs-fmt` (`.nix`), `clang-format` (C/C++) — both wired via `.claude/settings.json` hooks
- **Changelog**: git-cliff with conventional commits (planned)
- **Deps**: Renovate for nixpkgs pin updates (planned)

## Useful commands

```bash
nix flake check          # validate flake outputs resolve
nix flake update         # bump nixpkgs pin (updates flake.lock)
nix develop              # enter dev shell manually
gh repo set-default MarkusBitterman/hallamp   # fix gh if it targets upstream
```

## Claude Code tools

Agents (invoke by describing the task — Claude selects automatically):
- **winapi-auditor** — audits a file/dir for Win32 API usage, outputs severity-ranked Linux replacements
- **codec-security-reviewer** — security review for codec/parser code (buffer overflows, OOB, etc.)

Skills (user-invocable):
- `/qt-port <path>` — walks a component through the full Qt5→Qt6 migration checklist

## Gotchas

- **GitHub MCP**: requires `GITHUB_TOKEN` in your shell env; add to `.envrc` or shell profile
- **clang-format hook**: fires automatically on C/C++ edits but requires a `.clang-format` file in the repo root to take effect — create one with `clang-format --style=LLVM --dump-config > .clang-format`
- **`Qt/DLL_5.12_x86/`**: pre-compiled Qt 5.12 Windows DLLs as 7z archives — do not edit; irrelevant to the Linux port
- **Plugin format**: Winamp plugins compile to `.w5s` files (DLLs renamed); naming convention is `in_*` (input), `out_*` (output), `gen_*` (general), `vis_*` (visualizer), `ml_*` (media library)
- **`.vcxproj`/`.sln` edits are blocked** by the PreToolUse hook — add new targets to CMakeLists.txt instead
