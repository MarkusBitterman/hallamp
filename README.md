# hallamp

A community fork of Winamp, targeting a Linux-native port with Qt6 and Nix.

> **Status**: Active development on the `hallamp` branch. Migrating from Windows-only (Visual Studio 2019, Qt 5.12) to Linux-native (CMake, Qt 6, Nix). The first components now **build and run** on Linux — see below. Full roadmap in [TODO.md](TODO.md).

## What builds today

Under `nix develop` (GCC 15, Qt 6.11, OpenSSL 3.x), CMake produces:

| Target | Kind | Status |
|---|---|---|
| `wac_network` | `libwac_network.so` | builds (zero Windows DLLs) and **runtime-proven** via a live DNS + HTTP/HTTPS smoke test |
| `pfc` | `libpfc.a` | builds (portable file components) |
| `nu` | `libnu.a` | builds (portable utility subset) |

```bash
nix develop
cmake -B build -G Ninja
cmake --build build           # libwac_network.so, libpfc.a, libnu.a
ctest --test-dir build        # wac_network_smoke: live HTTP + HTTPS GET (needs network)
```

## Dev environment

Requires [Nix](https://nixos.org/) with flakes enabled:

```
experimental-features = nix-command flakes
```

```bash
git clone https://github.com/MarkusBitterman/hallamp
cd hallamp
direnv allow        # if you have direnv — activates automatically
# or:
nix develop         # enter the Qt6 + audio libs shell manually
```

The shell provides: Qt 6, CMake, Ninja, Clang, mpg123, libFLAC, libvorbis, libopenmpt, libsndfile, OpenSSL, and supporting tools.

## Project layout

| Path | Purpose |
|---|---|
| `Src/Components/` | Qt-based components — primary Qt5→Qt6 migration target |
| `Src/Winamp/` | Main application |
| `Src/Plugins/` | Input/output/DSP/visualizer/library plugins (`.w5s` format) |
| `Src/Wasabi/` | Custom UI framework (heavy Win32 — ported last) |
| `Src/nu/` | Nullsoft Utility lib — good early Linux port candidate |
| `Src/pfc/` | Portable file components |
| `Src/external_dependencies/` | CEF, openmpt, cpr, vorbis, theora |

## Contributing

- Branch: **`hallamp`** — all work goes here
- Commits: [conventional commits](https://www.conventionalcommits.org/) (`feat:`, `fix:`, `chore:`, `refactor:`, `build:`, `docs:`)
- See [CLAUDE.md](CLAUDE.md) for full contributor context and branch rules

## Migration roadmap

See [TODO.md](TODO.md) for the full phased plan:

1. **Phase 0** — Foundation (Nix, CMake skeleton, CI) — *mostly done*
2. **Phase 1** — Build system migration (CMake, port `nu`/`pfc`) ← *in progress* (`libpfc.a`, `libnu.a` building)
3. **Phase 2** — Qt5 → Qt6 (`Src/Components/`) ← *in progress* (`wac_network` builds + runs)
4. **Phase 3** — Audio subsystem (PipeWire output)
5. **Phase 4** — GUI / rendering layer
6. **Phase 5** — Packaging (Nix package, AppImage, Flatpak)

## License

[Winamp Collaborative License (WCL) v1.0](LICENSE.md)

---

<details>
<summary>Legacy Windows build instructions (upstream)</summary>

Building the Windows client requires Visual Studio 2019 and Intel IPP 6.1.1.035 (exact version).

Two options:
1. `build_winampAll_2019.cmd` — builds 4 variants (x86/x64 × Debug/Release) without the IDE
2. `winampAll_2019.sln` — open in Visual Studio for IDE debugging

### Dependencies (Windows)

**libvpx** — from [ShiftMediaProject/libvpx](https://github.com/ShiftMediaProject/libvpx), run `unpack_libvpx_v1.8.2_msvc16.cmd`

**libmpg123** — from [mpg123.de](https://www.mpg123.de/download.shtml), run `unpack_libmpg123.cmd`

**OpenSSL 1.0.1u** — build static libs with `build_vs_2019_openssl_x86.cmd` and `build_vs_2019_openssl_64.cmd`. Requires 7-Zip, NASM, and Perl.

**DirectX 9 SDK (June 2010)** — run `unpack_microsoft_directx_sdk_2010.cmd`

**Intel IPP 6.1.1.035** — run `unpack_intel_ipp_6.1.1.035.cmd`

**ATLMFC fix** — in `atltransactionmanager.h` line 427, change `::DeleteFile` to `DeleteFile`.

</details>
