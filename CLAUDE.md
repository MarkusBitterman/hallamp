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

- **CI**: GitHub Actions — build check on PRs via `nix flake check`
- **Format**: `nixpkgs-fmt` for Nix files; `clang-format` for C++ (to be wired up)
- **Changelog**: git-cliff with conventional commits
- **Deps**: Renovate for nixpkgs pin updates
