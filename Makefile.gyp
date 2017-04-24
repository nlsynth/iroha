include config.mk

prefix ?= /usr/local

iroha	: src/out/Default/iroha
	rm -f iroha
	ln -s src/out/Default/iroha ./iroha

src/out/Default/iroha: src/Makefile config.mk
	make -C src

src/Makefile: src/iroha.gyp
	gyp src/iroha.gyp --depth=. -f make --generator-output=src

clean:
	rm -rf src/out/
	rm -f iroha
