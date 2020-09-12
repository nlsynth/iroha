#include "iroha/iroha_main.h"

#include <iostream>

#include "iroha/iroha.h"

namespace iroha {

void printUsage() {
  std::cout
      << " Iroha frontend: " << PACKAGE << "-" << VERSION << "\n"
      << " [OPTION]... [FILE]...\n"
      << "  Read standard input when FILE is -\n\n"
      << "  -s Generate shell module\n"
      << "  -S Generate self contained (with clock and reset) shell module\n"
      << "  -vcd Output vcd (-s or -S should be specified)\n"
      << "  -v Output Verilog\n"
      << "  -I Set import paths (comma separated. can have multiple -I "
         "options)\n"
      << "  -h Output HTML\n"
      << "  -dot Output Dot (graphviz)\n"
      << "  -o [fn] output to the file name\n"
      << "  -d Debug dump\n"
      << "  -k Don't validate ids and names\n"
      << "  --output_marker=[marker]\n"
      << "  --root=[root dir]\n"
      << "  --flavor=[flavor]\n"
      << "  -opt [optimizer names (comma separated)]\n";
  std::cout << "    available optimizers: ";
  vector<string> phases = Iroha::GetOptimizerPhaseNames();
  for (size_t i = 0; i < phases.size(); ++i) {
    if (i > 0) {
      std::cout << ",";
    }
    std::cout << phases[i];
  }
  std::cout << "\n";
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
  bool dot = false;
  bool html = false;
  bool shell = false;
  bool selfShell = false;
  bool vcd = false;
  bool skipValidation = false;
  bool debugWriter = false;
  bool showVersion = false;

  string output;
  string debug_dump;
  string output_marker;
  string root_dir;
  string flavor;
  vector<string> opts;
  vector<string> inc_paths;

  for (int i = 1; i < argc; ++i) {
    const string arg(argv[i]);
    if (arg == "--version" || arg == "--help") {
      showVersion = true;
      break;
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
    if (arg == "-h") {
      html = true;
      continue;
    }
    if (arg == "-k") {
      skipValidation = true;
      continue;
    }
    if (arg == "-dot") {
      dot = true;
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
    if (arg == "-dw") {
      debugWriter = true;
      continue;
    }
    if (arg == "-I") {
      string inc = getFlagValue(argc, argv, &i);
      vector<string> paths;
      iroha::Util::SplitStringUsing(inc, ",", &paths);
      for (auto &p : paths) {
        inc_paths.push_back(p);
      }
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
    if (tokens[0] == "--flavor") {
      if (tokens.size() == 1) {
        flavor = getFlagValue(argc, argv, &i);
      } else {
        flavor = tokens[1];
      }
      continue;
    }
    // The name can be "-".
    files.push_back(arg);
  }

  Iroha::Init();

  if (files.empty() || showVersion) {
    printUsage();
    return 0;
  }

  if (inc_paths.size() > 0) {
    Iroha::SetImportPaths(inc_paths);
  }

  for (string &fn : files) {
    IDesign *design = Iroha::ReadDesignFromFile(fn);
    if (design == nullptr) {
      cerr << "Failed to read design from: " << fn << "\n";
      continue;
    }
    std::unique_ptr<IDesign> deleter(design);
    OptAPI *optimizer = Iroha::CreateOptimizer(design);
    if (!debug_dump.empty()) {
      optimizer->EnableDebugAnnotation();
    }
    bool has_opt_err = false;
    for (string &pass : opts) {
      if (!optimizer->ApplyPass(pass)) {
        has_opt_err = true;
      }
    }
    if (has_opt_err) {
      cerr << "Failed to optimize the design: " << fn << "\n";
    }
    WriterAPI *writer = Iroha::CreateWriter(design);
    if (!output_marker.empty() || !root_dir.empty() || !flavor.empty() ||
        debugWriter) {
      writer->SetOutputConfig(root_dir, flavor, output_marker, debugWriter);
    }
    if (shell || selfShell) {
      if (!output.empty()) {
        writer->OutputShellModule(true, selfShell, vcd);
      }
    }
    if (verilog) {
      writer->SetLanguage("verilog");
    }
    if (html) {
      writer->SetLanguage("html");
    }
    if (dot) {
      writer->SetLanguage("dot");
    }
    if (!skipValidation) {
      DesignTool::Validate(design);
    }
    if (!writer->Write(output)) {
      return 1;
    }
    if (!debug_dump.empty()) {
      optimizer->DumpIntermediateToFiles(debug_dump);
    }
  }
  return 0;
}

}  // namespace iroha
