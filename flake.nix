{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        dotnetPkg = (
          with pkgs.dotnetCorePackages;
          combinePackages [
            sdk_8_0
            sdk_9_0
          ]
        );
        deps = with pkgs; [
          zlib
          zlib.dev
          openssl
          icu
          dotnetPkg
          xorg.libX11
          xorg.libXrandr
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi
          xorg.libXext
          wayland
          wayland-protocols
          libxkbcommon
          libGL
          libxml2
          emscripten
        ];
      in
      {
        devShells.default =
          with pkgs;
          mkShell.override { stdenv = llvmPackages_17.stdenv; } {
            nativeBuildInputs = [
              cmake
              cmake-language-server
              clang-tools
              ccache
              python3
              ninja
              gdb
              omnisharp-roslyn
              nodejs
              lld
              tree-sitter
              tree-sitter-grammars.tree-sitter-cpp
              tree-sitter-grammars.tree-sitter-c-sharp
              linuxPackages.perf
              hotspot
            ] ++ deps;

            NIX_LD_LIBRARY_PATH = lib.makeLibraryPath ([ stdenv.cc.cc ] ++ deps);
            NIX_LD = "${pkgs.stdenv.cc.libc_bin}/bin/ld.so";
            EMSDK = "/home/shaggy/Git/emsdk";
            EM_CACHE = "/home/shaggy/.emscripten_cache";

            shellHook = ''
              DOTNET_ROOT="${dotnetPkg}"
              DRI_PRIME=1;
            '';
          };
      }
    );
}
