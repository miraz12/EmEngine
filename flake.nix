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
          glfw
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
              emscripten
              python3
              ninja
              gdb
              omnisharp-roslyn
              nodejs
              lld
              tree-sitter
              tree-sitter-grammars.tree-sitter-cpp
              tree-sitter-grammars.tree-sitter-c-sharp
            ] ++ deps;

            NIX_LD_LIBRARY_PATH = lib.makeLibraryPath ([ stdenv.cc.cc ] ++ deps);
            NIX_LD = "${pkgs.stdenv.cc.libc_bin}/bin/ld.so";

            shellHook = ''
              DOTNET_ROOT="${dotnetPkg}";
              EMSCRIPTEN_ROOT=${pkgs.emscripten}/share/emscripten
              DRI_PRIME=1
            '';
          };
      }
    );
}
