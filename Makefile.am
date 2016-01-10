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
 src/iroha/common.cpp src/iroha/common.h \
 src/iroha/iroha.cpp src/iroha/iroha.h \
 src/iroha/i_design.cpp src/iroha/i_design.h \
 src/iroha/logging.cpp src/iroha/logging.h \
 src/iroha/resource_class.cpp src/iroha/resource_class.h \
 src/iroha/resource_params.cpp src/iroha/resource_params.h \
 src/iroha/stl_util.h \
 src/iroha/writer_api.h \
 src/builder/exp_builder.cpp src/builder/exp_builder.h \
 src/builder/fsm_builder.cpp src/builder/fsm_builder.h \
 src/builder/reader.cpp src/builder/reader.h \
 src/design/design_tool.cpp src/design/design_tool.h \
 src/design/util.cpp src/design/util.h \
 src/design/validator.cpp src/design/validator.h \
 src/opt/array_to_mem.cpp src/opt/array_to_mem.h \
 src/opt/bb_collector.cpp src/opt/bb_collector.h \
 src/opt/bb_set.cpp src/opt/bb_set.h \
 src/opt/bb_shrink.cpp src/opt/bb_shrink.h \
 src/opt/optimizer.cpp src/opt/optimizer.h \
 src/opt/opt_util.cpp src/opt/opt_util.h \
 src/opt/phase.h src/opt/phase.cpp \
 src/writer/connection.cpp src/writer/connection.h \
 src/writer/exp_writer.cpp src/writer/exp_writer.h \
 src/writer/module_template.cpp src/writer/module_template.h \
 src/writer/verilog/embed.cpp src/writer/verilog/embed.h \
 src/writer/verilog/insn_writer.cpp src/writer/verilog/insn_writer.h \
 src/writer/verilog/module.cpp src/writer/verilog/module.h \
 src/writer/verilog/ports.cpp src/writer/verilog/ports.h \
 src/writer/verilog/state.cpp src/writer/verilog/state.h \
 src/writer/verilog/table.cpp src/writer/verilog/table.h \
 src/writer/verilog/verilog_writer.cpp src/writer/verilog/verilog_writer.h \
 src/writer/writer.cpp src/writer/writer.h
