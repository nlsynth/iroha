#include <iostream>

#include "iroha/iroha.h"

void printVersion() {
  std::cout << PACKAGE << "-" << VERSION << "\n";
}

int main(int argc, char **argv) {
  vector<string> files;
  bool verilog = false;
  bool html = false;

  string output;
  string debug_dump;
  vector<string> opts;

  for (int i = 1; i < argc; ++i) {
    const string arg(argv[i]);
    if (arg == "--version") {
      printVersion();
      return 0;
    }
    if (arg == "-v") {
      verilog = true;
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
    if (arg == "-opt" && i + 1 < argc) {
      iroha::Util::SplitStringUsing(argv[i + 1], ",", &opts);
      ++i;
      continue;
    }
    files.push_back(arg);
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
    if (verilog) {
      writer->SetLanguage("verilog");
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
