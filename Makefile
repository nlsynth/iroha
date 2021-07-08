include config.mk
export V

prefix ?= /usr/local
pkgdir ?= /tmp

iroha	: src/out/Default/iroha build
	rm -f iroha
	ln -s src/out/Default/iroha ./iroha

src/out/Default/iroha: src/Makefile config.mk
	make -C src

build:
	make -C src

src/Makefile: src/iroha.gyp
	gyp src/iroha.gyp --depth=. -f make --generator-output=src

clean:
	rm -rf src/out/
	rm -f iroha

install: install-data
	mkdir -p $(prefix)/bin
	install iroha $(prefix)/bin/iroha

install-data:
	mkdir -p $(prefix)/share/iroha
	install lib/generic-platform.iroha $(prefix)/share/iroha/generic-platform.iroha

pkg-data:
	cp lib/generic-platform.iroha $(pkgdir)/

.PHONY: build clean install install-data
