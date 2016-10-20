# Maintainer: Afri 5chdn <aur@cach.co>
# Available from AUR: https://aur.archlinux.org/packages/solidity/

pkgname=solidity
pkgver=0.4.2
pkgrel=1
_chash='4fc6fc2ca59579fae2472df319c2d8d31fe5bde5'
pkgdesc="The Solidity Contract-Oriented Programming Language (Including solc and lllc)"
arch=('i686' 'x86_64')
depends=(
  'boost'
  'boost-libs'
  'glibc'
)
makedepends=(
  'autoconf'
  'automake'
  'cmake'
  'gcc'
  'gcc-libs'
  'libtool'
  'git'
)
groups=('ethereum')
url="https://github.com/ethereum/solidity"
license=('GPL')
source=("https://github.com/ethereum/${pkgname}/archive/v${pkgver}.tar.gz")
sha256sums=('0dd176919cab5492aecb27d1d8de39182b5e880979d5082fca6d7a01ebce28f1')
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

build() {

  msg 'Updating...'
  cd ${srcdir}/${pkgname}-${pkgver}
  echo "${_chash}" > ${srcdir}/${pkgname}-${pkgver}/commit_hash.txt

  msg 'Building...'
  mkdir -p build && pushd build
  cmake .. \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release
  make
  popd
}

package() {
  cd ${srcdir}/${pkgname}-${pkgver}

  msg 'Installing...'
  make DESTDIR="$pkgdir" install -C build
}
