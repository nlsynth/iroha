#include "iroha/iroha_main.h"

#include "iroha/iroha.h"

#include <iostream>

namespace iroha {

void printVersion() {
  std::cout << " Iroha frontend: " << PACKAGE << "-" << VERSION << "\n"
	    << " [OPTION]... [FILE]...\n"
	    << "  Read standard input when FILE is -\n\n"
	    << "  -s Generate shell module\n"
	    << "  -S Generate self contained (with clock and reset) shell module\n"
	    << "  -vcd Output vcd (-s or -S should be specified)\n"
	    << "  -c Output C++\n"
	    << "  -v Output Verilog\n"
	    << "  -I Set import paths (comma separated)\n"
	    << "  -h Output HTML\n"
	    << "  -o [fn] output to the file name\n"
	    << "  -d Debug dump\n"
	    << "  -j Don't process module-import\n"
	    << "  -k Don't validate ids and names\n"
	    << "  --output_marker=[marker]\n"
	    << "  --root=[root dir]\n"
	    << "  -opt [optimizers]\n";
}

string getFlagValue(int argc, char **argv, int *idx) {
  (*idx)++;
  if (!(*idx < argc)) {
    return "";
  }
  return argv[*idx];
}

// Internal main function to embed the functionality in different binaries.
int main(int argc, char **argv) {
  vector<string> files;
  bool verilog = false;
  bool cxx = false;
  bool html = false;
  bool shell = false;
  bool selfShell = false;
  bool vcd = false;
  bool skipImport = false;
  bool skipValidation = false;

  string output;
  string debug_dump;
  string output_marker;
  string root_dir;
  vector<string> opts;
  vector<string> paths;

  for (int i = 1; i < argc; ++i) {
    const string arg(argv[i]);
    if (arg == "--version") {
      printVersion();
      return 0;
    }
    if (arg == "--iroha") {
      // Ignored. This can be used to tell real frontend to run as Iroha.
      continue;
    }
    if (arg == "-s") {
      shell = true;
      continue;
    }
    if (arg == "-S") {
      selfShell = true;
      continue;
    }
    if (arg == "-v") {
      verilog = true;
      continue;
    }
    if (arg == "-c") {
      cxx = true;
      continue;
    }
    if (arg == "-h") {
      html = true;
      continue;
    }
    if (arg == "-j") {
      skipImport = true;
      continue;
    }
    if (arg == "-k") {
      skipValidation = true;
      continue;
    }
    if (arg == "-vcd") {
      vcd = true;
      continue;
    }
    if (arg == "-o") {
      output = getFlagValue(argc, argv, &i);
      continue;
    }
    if (arg == "-d") {
      debug_dump = getFlagValue(argc, argv, &i);
      continue;
    }
    if (arg == "-I") {
      string inc = getFlagValue(argc, argv, &i);
      iroha::Util::SplitStringUsing(inc, ",", &paths);
      continue;
    }
    if (arg == "-opt") {
      string o = getFlagValue(argc, argv, &i);
      iroha::Util::SplitStringUsing(o, ",", &opts);
      continue;
    }
    vector<string> tokens;
    iroha::Util::SplitStringUsing(arg, "=", &tokens);
    if (tokens[0] == "--output_marker") {
      if (tokens.size() == 1) {
	output_marker = getFlagValue(argc, argv, &i);
      } else {
	output_marker = tokens[1];
      }
      continue;
    }
    if (tokens[0] == "--root") {
      if (tokens.size() == 1) {
	root_dir = getFlagValue(argc, argv, &i);
      } else {
	root_dir = tokens[1];
      }
      continue;
    }
    // The name can be "-".
    files.push_back(arg);
  }

  if (files.empty()) {
    printVersion();
    return 0;
  }

  Iroha::Init();

  for (string &fn : files) {
    IDesign *design = Iroha::ReadDesignFromFile(fn);
    if (design == nullptr) {
      cerr << "Failed to read design from: " << fn << "\n";
      continue;
    }
    std::unique_ptr<IDesign> deleter(design);
    OptAPI *optimizer = Iroha::CreateOptimizer(design);
    if (html || !debug_dump.empty()) {
      optimizer->EnableDebugAnnotation();
    }
    bool has_opt_err = false;
    for (string &phase : opts) {
      if (!optimizer->ApplyPhase(phase)) {
	has_opt_err = true;
      }
    }
    if (has_opt_err) {
      cerr << "Failed to optimize the design: " << fn << "\n";
    }
    WriterAPI *writer = Iroha::CreateWriter(design);
    if (!output_marker.empty() || !root_dir.empty()) {
      writer->SetOutputConfig(root_dir, output_marker);
    }
    if (shell || selfShell) {
      if (!output.empty()) {
	writer->OutputShellModule(true, selfShell, vcd);
      }
    }
    if (paths.size() > 0) {
      Iroha::SetImportPaths(paths);
    }
    if (verilog) {
      writer->SetLanguage("verilog");
    }
    if (cxx) {
      writer->SetLanguage("cxx");
    }
    if (html) {
      writer->SetLanguage("html");
    }
    if (!skipImport) {
      DesignTool::ResolveImport(design);
    }
    if (!skipValidation) {
      DesignTool::Validate(design);
    }
    writer->Write(output);
    if (!debug_dump.empty()) {
      optimizer->DumpIntermediate(debug_dump);
    }
  }
  return 0;
}

}  // namespace iroha
