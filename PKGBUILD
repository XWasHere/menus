pkgname="menus"
pkgver=1.2.2
pkgrel=6
arch=('x86_64')
optdepends=('bash: for writing scripts using menus')
pkgdesc="crappy thing for getting user input in shell scripts"
licence=('GPL')
website="https://github.com/xwashere/menus"

package() {
	mkdir --parent $pkgdir/usr/bin
	mkdir --parent $pkgdir/usr/share/man/man1
	cp $startdir/build/menus $pkgdir/usr/bin/menus
	cp $startdir/docs/menus.1 $pkgdir/usr/share/man/man1/menus.1
}

build() {
	pushd $startdir
	make
	popd
}
