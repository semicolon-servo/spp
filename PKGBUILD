# Maintainer: fox <pixilreal@gmail.com>
pkgname=servo++
pkgver=1.0.0
pkgrel=1
pkgdesc="C++ implementation of the servo templating language"
arch=('x86_64')
url="https://github.com/yourusername/servo"
license=('custom')
depends=('gcc-libs')
makedepends=('gcc' 'make')
# For local builds from current directory, use: makepkg --noextract
source=("${pkgname}-${pkgver}.tar.gz")
sha256sums=('SKIP')

build() {
    cd "${srcdir}"
    # Find servo/spp directory (works with extracted tarball or --noextract)
    if [ -d "servo/spp" ]; then
        cd servo/spp
    elif [ -d "${pkgname}-${pkgver}/servo/spp" ]; then
        cd "${pkgname}-${pkgver}/servo/spp"
    fi
    make clean || true
    make
}

package() {
    cd "${srcdir}"
    # Find servo/spp directory (works with extracted tarball or --noextract)
    if [ -d "servo/spp" ]; then
        cd servo/spp
    elif [ -d "${pkgname}-${pkgver}/servo/spp" ]; then
        cd "${pkgname}-${pkgver}/servo/spp"
    fi
    install -Dm755 servo_cpp "${pkgdir}/usr/bin/servo_cpp"
}

