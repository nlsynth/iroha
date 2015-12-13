AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -I src -Wall -Wno-sign-compare -g

SUBDIRS =
EXTRA_DIST =

bin_PROGRAMS = iroha

iroha_SOURCES = src/main.cpp

iroha_LDADD = libiroha.la

noinst_LTLIBRARIES = libiroha.la
libiroha_la_SOURCES = \
 src/iroha/common.h \
 src/iroha/iroha.h src/iroha/iroha.cpp \
 src/iroha/writer_api.h \
 src/builder/exp_builder.cpp src/builder/exp_builder.h \
 src/builder/reader.cpp src/builder/reader.h \
 src/writer/writer.cpp src/writer/writer.h
