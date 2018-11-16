# $ gyp src/iroha.gyp --depth=. -f make
# maybe with --generator-output=src
{
  'make_global_settings': [
  ],
  'target_defaults': {
    'cflags': [ '-std=c++11', ],
    'defines': ['PACKAGE="iroha"', 'VERSION="0.0.1"'],
    'xcode_settings': {
      'OTHER_CFLAGS': [
          '-std=c++11',
      ],
    },
  },
  'targets': [
    {
      'target_name': 'iroha',
      'product_name': 'iroha',
      'type': 'executable',
      'include_dirs': [
        './',
      ],
      'sources': [
        'main.cpp',
      ],
      'dependencies': [
        ':libiroha'
      ],
    },
    {
      'target_name': 'numeric_test',
      'product_name': 'numeric_test',
      'type': 'executable',
      'include_dirs': [
        './',
      ],
      'sources': [
        'numeric/numeric_test.cpp',
      ],
      'dependencies': [
        ':numeric'
      ],
    },
    {
      'target_name': 'libiroha',
      'product_name': 'iroha',
      'type': 'static_library',
      'include_dirs': [
        './',
      ],
      'sources': [
        'builder/design_builder.cpp',
        'builder/design_builder.h',
        'builder/fsm_builder.cpp',
        'builder/fsm_builder.h',
        'builder/reader.cpp',
        'builder/reader.h',
        'builder/platform_builder.cpp',
        'builder/platform_builder.h',
        'builder/tree_builder.cpp',
        'builder/tree_builder.h',
        'design/design_tool.cpp',
        'design/design_tool.h',
        'design/design_util.cpp',
        'design/design_util.h',
        'design/importer.cpp',
        'design/importer.h',
        'design/module_copier.cpp',
        'design/module_copier.h',
        'design/resource_attr.cpp',
        'design/resource_attr.h',
        'design/resource_copier.cpp',
        'design/resource_copier.h',
        'design/table_copier.cpp',
        'design/table_copier.h',
        'design/validator.cpp',
        'design/validator.h',
        'iroha/common.cpp',
        'iroha/common.h',
        'iroha/i_design.cpp',
        'iroha/i_design.h',
        'iroha/i_platform.cpp',
        'iroha/i_platform.h',
        'iroha/insn_operands.h',
        'iroha/iroha.cpp',
        'iroha/iroha.h',
        'iroha/iroha_main.cpp',
        'iroha/iroha_main.h',
        'iroha/logging.cpp',
        'iroha/logging.h',
        'iroha/module_import.cpp',
        'iroha/module_import.h',
        'iroha/object_pool.h',
        'iroha/opt_api.h',
        'iroha/resource_class.cpp',
        'iroha/resource_class.h',
        'iroha/resource_params.cpp',
        'iroha/resource_params.h',
        'iroha/stl_util.h',
        'iroha/writer_api.h',
        'opt/array_to_mem.cpp',
        'opt/array_to_mem.h',
        'opt/bb_collector.cpp',
        'opt/bb_collector.h',
        'opt/bb_set.cpp',
        'opt/bb_set.h',
        'opt/clean/empty_state.cpp',
        'opt/clean/empty_state.h',
        'opt/clean/empty_table.cpp',
        'opt/clean/empty_table.h',
        'opt/clean/pseudo_resource.cpp',
        'opt/clean/pseudo_resource.h',
        'opt/clean/unreachable_state.cpp',
        'opt/clean/unreachable_state.h',
        'opt/clean/unused_resource.cpp',
        'opt/clean/unused_resource.h',
        'opt/common.h',
        'opt/compound.cpp',
        'opt/compound.h',
        'opt/data_flow_collector.cpp',
        'opt/data_flow_collector.h',
        'opt/data_flow.cpp',
        'opt/data_flow.h',
        'opt/debug_annotation.cpp',
        'opt/debug_annotation.h',
	'opt/delay_info.cpp',
	'opt/delay_info.h',
        'opt/dominator_tree_builder.cpp',
        'opt/dominator_tree_builder.h',
        'opt/dominator_tree.cpp',
        'opt/dominator_tree.h',
        'opt/optimizer.cpp',
        'opt/optimizer.h',
        'opt/opt_util.cpp',
        'opt/opt_util.h',
        'opt/phase.cpp',
        'opt/phase.h',
        'opt/ssa/phi_builder.cpp',
        'opt/ssa/phi_builder.h',
        'opt/ssa/phi_cleaner.cpp',
        'opt/ssa/phi_cleaner.h',
        'opt/ssa/phi_injector.cpp',
        'opt/ssa/phi_injector.h',
        'opt/ssa/ssa_converter.cpp',
        'opt/ssa/ssa_converter.h',
        'opt/ssa/ssa.cpp',
        'opt/ssa/ssa.h',
	'opt/wire/common.h',
	'opt/wire/data_path.cpp',
	'opt/wire/data_path.h',
	'opt/wire/path_node.cpp',
	'opt/wire/path_node.h',
	'opt/wire/resource_entry.cpp',
	'opt/wire/resource_entry.h',
	'opt/wire/resource_share.cpp',
	'opt/wire/resource_share.h',
	'opt/wire/relocator.cpp',
	'opt/wire/relocator.h',
	'opt/wire/scheduler.cpp',
	'opt/wire/scheduler.h',
        'opt/wire/virtual_resource.cpp',
        'opt/wire/virtual_resource.h',
        'opt/wire/wire.cpp',
        'opt/wire/wire.h',
        'opt/wire/wire_phase.cpp',
        'opt/wire/wire_phase.h',
        'opt/wire/wire_plan.cpp',
        'opt/wire/wire_plan.h',
        'platform/platform.cpp',
        'platform/platform.h',
        'platform/platform_db.cpp',
        'platform/platform_db.h',
        'writer/connection.cpp',
        'writer/connection.h',
        'writer/cxx/class_writer.cpp',
        'writer/cxx/class_writer.h',
        'writer/cxx/common.h',
        'writer/cxx/cxx_writer.cpp',
        'writer/cxx/cxx_writer.h',
        'writer/cxx/module.cpp',
        'writer/cxx/module.h',
        'writer/cxx/resource.cpp',
        'writer/cxx/resource.h',
        'writer/cxx/state.cpp',
        'writer/cxx/state.h',
        'writer/cxx/table.cpp',
        'writer/cxx/table.h',
        'writer/exp_writer.cpp',
        'writer/exp_writer.h',
        'writer/html_writer.cpp',
        'writer/html_writer.h',
        'writer/module_template.cpp',
        'writer/module_template.h',
        'writer/names.cpp',
        'writer/names.h',
        'writer/writer.cpp',
        'writer/writer.h',
      ],
      'dependencies': [
        ':numeric',
        ':verilog_writer'
      ],
    },
    {
      'target_name': 'numeric',
      'product_name': 'numeric',
      'type': 'static_library',
      'include_dirs': [
        './',
      ],
      'sources': [
        'numeric/numeric_op.cpp',
        'numeric/numeric_op.h',
        'numeric/numeric_type.cpp',
        'numeric/numeric_type.h',
        'numeric/wide_op.cpp',
        'numeric/wide_op.h',
      ],
    },
    {
      'target_name': 'verilog_writer',
      'product_name': 'verilog_writer',
      'type': 'static_library',
      'include_dirs': [
        './',
      ],
      'sources': [
        'writer/verilog/axi/axi_controller.cpp',
        'writer/verilog/axi/axi_controller.h',
        'writer/verilog/axi/axi_port.cpp',
        'writer/verilog/axi/axi_port.h',
        'writer/verilog/axi/master_controller.cpp',
        'writer/verilog/axi/master_controller.h',
        'writer/verilog/axi/master_port.cpp',
        'writer/verilog/axi/master_port.h',
        'writer/verilog/axi/slave_controller.cpp',
        'writer/verilog/axi/slave_controller.h',
        'writer/verilog/axi/slave_port.cpp',
        'writer/verilog/axi/slave_port.h',
        'writer/verilog/common.h',
        'writer/verilog/dataflow_in.cpp',
        'writer/verilog/dataflow_in.h',
        'writer/verilog/dataflow_state.cpp',
        'writer/verilog/dataflow_state.h',
        'writer/verilog/dataflow_table.cpp',
        'writer/verilog/dataflow_table.h',
        'writer/verilog/embed.cpp',
        'writer/verilog/embed.h',
        'writer/verilog/ext_combinational.cpp',
        'writer/verilog/ext_combinational.h',
        'writer/verilog/ext_io.cpp',
        'writer/verilog/ext_io.h',
        'writer/verilog/ext_task_call.cpp',
        'writer/verilog/ext_task_call.h',
        'writer/verilog/ext_task.cpp',
        'writer/verilog/ext_task.h',
        'writer/verilog/fifo.cpp',
        'writer/verilog/fifo.h',
        'writer/verilog/fifo_accessor.cpp',
        'writer/verilog/fifo_accessor.h',
        'writer/verilog/insn_writer.cpp',
        'writer/verilog/insn_writer.h',
        'writer/verilog/internal_sram.cpp',
        'writer/verilog/internal_sram.h',
        'writer/verilog/inter_module_wire.cpp',
        'writer/verilog/inter_module_wire.h',
        'writer/verilog/mapped.cpp',
        'writer/verilog/mapped.h',
        'writer/verilog/module.cpp',
        'writer/verilog/module.h',
        'writer/verilog/operator.cpp',
        'writer/verilog/operator.h',
        'writer/verilog/ports.cpp',
        'writer/verilog/ports.h',
        'writer/verilog/resource.cpp',
        'writer/verilog/resource.h',
        'writer/verilog/shared_memory.cpp',
        'writer/verilog/shared_memory.h',
        'writer/verilog/shared_reg_accessor.cpp',
        'writer/verilog/shared_reg_accessor.h',
        'writer/verilog/shared_reg.cpp',
        'writer/verilog/shared_reg.h',
        'writer/verilog/state.cpp',
        'writer/verilog/state.h',
        'writer/verilog/table.cpp',
        'writer/verilog/table.h',
        'writer/verilog/task.cpp',
        'writer/verilog/task.h',
        'writer/verilog/ticker.cpp',
        'writer/verilog/ticker.h',
        'writer/verilog/verilog_writer.cpp',
        'writer/verilog/verilog_writer.h',
      ],
      'dependencies': [
      ],
    },
  ]
}
