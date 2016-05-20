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
 src/builder/tree_builder.cpp src/builder/tree_builder.h \
 src/design/design_tool.cpp src/design/design_tool.h \
 src/design/design_util.cpp src/design/design_util.h \
 src/design/validator.cpp src/design/validator.h \
 src/opt/array_to_mem.cpp src/opt/array_to_mem.h \
 src/opt/bb_collector.cpp src/opt/bb_collector.h \
 src/opt/bb_set.cpp src/opt/bb_set.h \
 src/opt/clean/empty_state.cpp src/opt/empty_state.h \
 src/opt/clean/unreachable_state.cpp src/opt/unreachable_state.h \
 src/opt/clean/unused_resource.cpp src/opt/unused_resource.h \
 src/opt/common.h \
 src/opt/data_flow.cpp src/opt/data_flow.h \
 src/opt/data_flow_collector.cpp src/opt/data_flow_collector.h \
 src/opt/debug_annotation.cpp src/opt/debug_annotation.h \
 src/opt/dominator_tree.cpp src/opt/dominator_tree.h \
 src/opt/dominator_tree_builder.cpp src/opt/dominator_tree_builder.h \
 src/opt/optimizer.cpp src/opt/optimizer.h \
 src/opt/opt_util.cpp src/opt/opt_util.h \
 src/opt/phase.cpp src/opt/phase.h \
 src/opt/ssa/phi_builder.cpp src/opt/ssa/phi_builder.h \
 src/opt/ssa/phi_cleaner.cpp src/opt/ssa/phi_cleaner.h \
 src/opt/ssa/phi_injector.cpp src/opt/ssa/phi_injector.h \
 src/opt/ssa/ssa.cpp src/opt/ssa/ssa.h \
 src/opt/ssa/ssa_converter.cpp src/opt/ssa/ssa_converter.h \
 src/opt/wire/wire_insn.cpp src/opt/wire/wire_insn.h \
 src/writer/connection.cpp src/writer/connection.h \
 src/writer/exp_writer.cpp src/writer/exp_writer.h \
 src/writer/html_writer.cpp src/writer/html_writer.h \
 src/writer/module_template.cpp src/writer/module_template.h \
 src/writer/verilog/channel.cpp src/writer/verilog/channel.h \
 src/writer/verilog/common.h \
 src/writer/verilog/embed.cpp src/writer/verilog/embed.h \
 src/writer/verilog/insn_writer.cpp src/writer/verilog/insn_writer.h \
 src/writer/verilog/internal_sram.cpp src/writer/verilog/internal_sram.h \
 src/writer/verilog/module.cpp src/writer/verilog/module.h \
 src/writer/verilog/operator.cpp src/writer/verilog/operator.h \
 src/writer/verilog/ports.cpp src/writer/verilog/ports.h \
 src/writer/verilog/resource.cpp src/writer/verilog/resource.h \
 src/writer/verilog/shared_reg.cpp src/writer/verilog/shared_reg.h \
 src/writer/verilog/state.cpp src/writer/verilog/state.h \
 src/writer/verilog/table.cpp src/writer/verilog/table.h \
 src/writer/verilog/task.cpp src/writer/verilog/task.h \
 src/writer/verilog/verilog_writer.cpp src/writer/verilog/verilog_writer.h \
 src/writer/writer.cpp src/writer/writer.h
