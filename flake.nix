{
  description = "hallamp — Winamp community fork, targeting Qt6 / Linux-native";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            # Qt6 — migration target from bundled Qt 5.12
            qt6.qtbase
            qt6.qtwebengine
            qt6.qtmultimedia
            qt6.qtnetworkauth
            qt6.wrapQtAppsHook

            # Build system — replacing Visual Studio 2019 / MSBuild
            cmake
            ninja
            pkg-config
            clang
            clang-tools

            # Audio codecs — mirrors current vcpkg dependency list
            mpg123
            flac
            libvorbis
            libogg
            libopenmpt
            libsndfile
            lame

            # Image / font
            libpng
            libjpeg
            freetype

            # Networking / crypto
            openssl
            curl
            zlib

            # Dev tooling
            python3
            git
            gh
            git-cliff # changelog generation from conventional commits
            nixpkgs-fmt

            # LSPs — clangd ships with clang-tools above
            cmake-language-server # CMakeLists.txt
            nixd # Nix files
          ];

          shellHook = ''
            echo "hallamp dev — Qt $(qmake6 --version 2>/dev/null | grep -oP '(?<=Qt version )\S+' || echo '6.x') / Nix"
            export QT_SELECT=qt6
            # Symlink compile_commands.json for clangd if a build directory exists
            if [ -f build/compile_commands.json ] && [ ! -L compile_commands.json ]; then
              ln -sf build/compile_commands.json compile_commands.json
            fi
          '';
        };
      });
}
