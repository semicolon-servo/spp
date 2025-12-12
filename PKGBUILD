# Maintainer: fox <pixilreal@gmail.com>
pkgname=servo++
pkgver=1.0
pkgrel=1
pkgdesc="C++ implementation of the servo programming language"
arch=('x86_64')
url="https://github.com/semicolon-servo/spp"
license=('MIT License')
depends=('gcc-libs')
makedepends=('gcc' 'make')
source=("https://github.com/semicolon-servo/spp/archive/refs/tags/v1.0.tar.gz")
sha256sums=('SKIP')

build() {
    cd "${srcdir}"
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
    if [ -d "servo/spp" ]; then
        cd servo/spp
    elif [ -d "${pkgname}-${pkgver}/servo/spp" ]; then
        cd "${pkgname}-${pkgver}/servo/spp"
    fi
    install -Dm755 servo_cpp "${pkgdir}/usr/bin/servo_cpp"
}

sha256sums=('76e5676b614cc2194113972e486d90c9bd0651123a46992a33b7e4d1f83c6590')
