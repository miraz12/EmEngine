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
        dotnetPkg = (with pkgs.dotnetCorePackages; combinePackages [ sdk_9_0 ]);
        deps = with pkgs; [
          zlib
          zlib.dev
          openssl
          dotnetPkg
        ];
      in
      {
        devShells.default =
          with pkgs;
          mkShell.override { stdenv = llvmPackages.stdenv; } {
            nativeBuildInputs = [
              cmake
              cmake-language-server
              clang-tools
              ccache
              emscripten
              python3
              ninja
              gdb
              libGL
              glfw
              omnisharp-roslyn
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
