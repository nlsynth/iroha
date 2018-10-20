include config.mk

prefix ?= /usr/local

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
	install -D iroha $(prefix)/bin/iroha

install-data:
	install -D lib/generic-platform.iroha $(prefix)/share/iroha/generic-platform.iroha

.PHONY: build clean install install-data
