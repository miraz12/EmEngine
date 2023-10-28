{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let pkgs = import nixpkgs { inherit system; };
      in {
        devShell = with pkgs;
          mkShell.override { stdenv = llvmPackages.stdenv; } {
            buildInputs = [
              cmake
              clang-tools
              ccache
              libGL
              xorg.libX11
              xorg.libXrandr
              xorg.libXinerama
              xorg.libXcursor
              xorg.libXi
            ];
          };
      });
}
