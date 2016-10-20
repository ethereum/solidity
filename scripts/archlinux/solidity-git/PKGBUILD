# Maintainer: Afri 5chdn <aur@cach.co>
# Available from AUR: https://aur.archlinux.org/packages/solidity-git/

pkgname=solidity-git
pkgver=0.4.3.git.20161020.9d30450
pkgrel=1
pkgdesc="The Solidity Contract-Oriented Programming Language (Including solc, lllc; from latest unstable git version)"
arch=('i686' 'x86_64')
depends=(
  'boost'
  'boost-libs'
  'crypto++'
  'jsoncpp'
  'glibc'
)
makedepends=(
  'autoconf'
  'automake'
  'cmake'
  'gcc'
  'gcc-libs'
  'libtool'
  'yasm'
  'git'
)
groups=('ethereum')
url="https://github.com/ethereum/solidity"
license=('GPL')
source=("${pkgname%-git}::git+https://github.com/ethereum/solidity#branch=develop")
sha256sums=('SKIP')
provides=(
  'lll'
  'lllc'
  'liblll'
  'solidity'
  'solc'
  'libsoldevcore'
  'libsolevmasm'
  'libsolidity'
)

pkgver() {
  cd ${pkgname%-git}
  echo "`grep -m1 "PROJECT\_VERSION" CMakeLists.txt | tr -cd '[[:digit:]].'`.git.`date +%Y%m%d`.`git log --pretty=format:%h -n 1`"
}

build() {
  msg 'Updating...'
  cd ${pkgname%-git}
  git submodule update --init --recursive

  msg 'Building...'
  mkdir -p build && pushd build
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr
  make
  popd
}

package() {
  cd ${pkgname%-git}

  msg 'Installing...'
  make DESTDIR="$pkgdir" install -C build

  msg 'Cleaning up pkgdir...'
  find "$pkgdir" -type d -name .git -exec rm -r '{}' +
  find "$pkgdir" -type f -name .gitignore -exec rm -r '{}' +
}
