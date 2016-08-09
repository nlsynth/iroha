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
	    << "  -c Output C++\n"
	    << "  -v Output Verilog\n"
	    << "  -I Set import paths (comma separated)\n"
	    << "  -h Output HTML\n"
	    << "  -o [fn] output to the file name\n"
	    << "  -d Debug dump\n"
	    << "  -opt [optimizers]\n";
}

int main(int argc, char **argv) {
  vector<string> files;
  bool verilog = false;
  bool cxx = false;
  bool html = false;
  bool shell = false;
  bool selfShell = false;

  string output;
  string debug_dump;
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
    if (arg == "-o" && i + 1 < argc) {
      output = string(argv[i + 1]);
      ++i;
      continue;
    }
    if (arg == "-d" && i + 1 < argc) {
      debug_dump = string(argv[i + 1]);
      ++i;
      continue;
    }
    if (arg == "-I" && i + 1 < argc) {
      iroha::Util::SplitStringUsing(argv[i + 1], ",", &paths);
      ++i;
      continue;
    }
    if (arg == "-opt" && i + 1 < argc) {
      iroha::Util::SplitStringUsing(argv[i + 1], ",", &opts);
      ++i;
      continue;
    }
    // The name can be "-".
    files.push_back(arg);
  }

  if (files.empty()) {
    printVersion();
    return 0;
  }

  iroha::Iroha::Init();

  for (string &fn : files) {
    iroha::IDesign *design = iroha::Iroha::ReadDesignFromFile(fn);
    if (design == nullptr) {
      cerr << "Failed to read design from: " << fn << "\n";
      continue;
    }
    iroha::OptAPI *optimizer = iroha::Iroha::CreateOptimizer(design);
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
    iroha::WriterAPI *writer = iroha::Iroha::CreateWriter(design);
    if (shell || selfShell) {
      if (!output.empty()) {
	writer->OutputShellModule(true, selfShell);
      }
    }
    if (paths.size() > 0) {
      iroha::Iroha::SetImportPaths(paths);
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
    writer->Write(output);
    if (!debug_dump.empty()) {
      optimizer->DumpIntermediate(debug_dump);
    }
  }
  return 0;
}

}  // namespace iroha
