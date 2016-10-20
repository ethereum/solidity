# PKGBUILD for Arch Linux

Create packages for Arch Linux from this repository.

## Build the packages

### Build soldity package (release)

Stable/Release packages are downloaded from
[github release page](https://github.com/ethereum/solidity/releases), checksum
verified and built from source.

    cd scripts/archlinux/solidity
    makepkg

### Build solidity-git package (develop)

Unstable/Develop packages are cloned from Github and built from latest
`develop` commit source.

    cd scripts/archlinux/solidity-git
    makepkg

## Install the packages

E.g., using `pacman`:

    sudo pacman -U solidity-0.4.2-1-x86_64.pkg.tar.xz

Or, respectively:

    sudo pacman -U solidity-git-0.4.3.git.9d30450-1-x86_64.pkg.tar.xz

## Arch user repository

Both packages are available from AUR:

- https://aur.archlinux.org/packages/solidity/
- https://aur.archlinux.org/packages/solidity-git/

Install them directly via
[AUR Helpers](https://wiki.archlinux.org/index.php/AUR_helpers), e.g.,

    yaourt -S solidity

Or, respectively:

    yaourt -S solidity-git

## Contributors

Maintained by Afri @5chdn
