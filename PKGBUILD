# Maintainer: Your Name <you@example.com>

pkgname=omasuite
pkgver=0.1.0
pkgrel=1
pkgdesc="All-in-one mail, calendar, contacts and tasks suite for Omarchy"
arch=('x86_64')
url="https://github.com/omarchy/omasuite"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'qt6-declarative' 'qt6-networkauth' 'openssl' 'libsecret')
makedepends=('qt6-base' 'qt6-declarative' 'qt6-networkauth')
source=()

build() {
  cd "$startdir"
  mkdir -p build-pkg
  cd build-pkg
  qmake6 "$startdir/OmaSuite.pro"
  make -j"$(nproc)"
}

package() {
  cd "$startdir/build-pkg"

  install -Dm755 omasuite \
    "$pkgdir/usr/bin/omasuite"

  install -Dm644 "$startdir/omasuite.desktop" \
    "$pkgdir/usr/share/applications/omasuite.desktop"

  install -Dm644 "$startdir/icons/omasuite.svg" \
    "$pkgdir/usr/share/icons/hicolor/scalable/apps/omasuite.svg"
}
